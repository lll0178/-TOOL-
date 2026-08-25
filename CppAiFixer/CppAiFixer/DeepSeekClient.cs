using System;
using System.Globalization;
using System.IO;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Runtime.Serialization.Json;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace CppAiFixer
{
    /// <summary>调用 DeepSeek / OpenAI 兼容接口，让模型给出多种修复做法。无第三方 JSON 依赖。</summary>
    internal static class DeepSeekClient
    {
        public static async Task<FixResponse> RequestCandidatesAsync(
            AiFixerOptionsPage opts, string apiKey, string filePath, string language,
            BuildOutputError error, string context)
        {
            ServicePointManager.SecurityProtocol |= SecurityProtocolType.Tls12;

            string baseUrl = string.IsNullOrWhiteSpace(opts.BaseUrl)
                ? "https://api.deepseek.com"
                : opts.BaseUrl.Trim().TrimEnd('/');
            string url = baseUrl + "/chat/completions";
            string model = string.IsNullOrWhiteSpace(opts.Model) ? "deepseek-chat" : opts.Model.Trim();
            int maxCand = Math.Max(1, Math.Min(5, opts.MaxCandidates));

            string systemPrompt =
                "你是资深 C/C++ 代码修复专家。用户给你一条编译错误及其代码上下文，你要提出多种修改做法。" +
                "必须只输出一个 JSON 对象，格式：{\"candidates\":[{\"title\":\"做法名\",\"explanation\":\"为什么这样做\",\"edits\":[{\"old\":\"原文精确片段(可多行)\",\"new\":\"替换后的片段(可多行)\"}]}]}。" +
                "要求：1) old 必须与上下文代码逐字符一致，必要时包含前后行以保证唯一可定位；" +
                "2) 每处修改尽量小；3) 不要输出代码块标记或 JSON 以外任何文字；" +
                "4) 最多 " + maxCand + " 个候选做法，按推荐程度排序；" +
                "5) 若确实无法修复，输出 {\"candidates\":[]}。";

            var userPrompt = new StringBuilder();
            userPrompt.Append("错误文件: ").AppendLine(filePath ?? "");
            userPrompt.Append("语言: ").AppendLine(language);
            userPrompt.Append("错误码: ").AppendLine(error.Code ?? "");
            userPrompt.Append("错误信息: ").AppendLine(error.Message ?? "");
            userPrompt.Append("错误行号: ").AppendLine((error.Line + 1).ToString());
            userPrompt.AppendLine("代码上下文(行号:内容):");
            userPrompt.Append(context);

            double temperature = Math.Min(2.0, Math.Max(0.0, opts.Temperature));
            bool jsonMode = !model.Contains("reasoner"); // deepseek-reasoner 不支持 JSON 输出模式
            string jsonBody = BuildRequestJson(model, temperature, jsonMode, systemPrompt, userPrompt.ToString());

            using (var http = new HttpClient())
            {
                http.Timeout = TimeSpan.FromSeconds(Math.Max(10, opts.TimeoutSeconds));
                http.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", apiKey);
                using (var content = new StringContent(jsonBody, Encoding.UTF8, "application/json"))
                using (var resp = await http.PostAsync(url, content).ConfigureAwait(false))
                {
                    string body = await resp.Content.ReadAsStringAsync().ConfigureAwait(false);
                    if (!resp.IsSuccessStatusCode)
                    {
                        throw new InvalidOperationException("HTTP " + (int)resp.StatusCode + ": " + Truncate(body, 300));
                    }
                    return ParseFixResponse(body);
                }
            }
        }

        private static string BuildRequestJson(string model, double temperature, bool jsonMode, string systemPrompt, string userPrompt)
        {
            var sb = new StringBuilder();
            sb.Append('{');
            sb.Append("\"model\":").Append(Quote(model)).Append(',');
            sb.Append("\"temperature\":").Append(temperature.ToString("0.##", CultureInfo.InvariantCulture)).Append(',');
            if (jsonMode) sb.Append("\"response_format\":{\"type\":\"json_object\"},");
            sb.Append("\"messages\":[");
            sb.Append("{\"role\":\"system\",\"content\":").Append(Quote(systemPrompt)).Append("},");
            sb.Append("{\"role\":\"user\",\"content\":").Append(Quote(userPrompt)).Append('}');
            sb.Append("]}");
            return sb.ToString();
        }

        private static string Quote(string s)
        {
            if (string.IsNullOrEmpty(s)) return "\"\"";
            var sb = new StringBuilder("\"");
            foreach (char c in s)
            {
                switch (c)
                {
                    case '"': sb.Append("\\\""); break;
                    case '\\': sb.Append("\\\\"); break;
                    case '\n': sb.Append("\\n"); break;
                    case '\r': sb.Append("\\r"); break;
                    case '\t': sb.Append("\\t"); break;
                    case '\b': sb.Append("\\b"); break;
                    case '\f': sb.Append("\\f"); break;
                    default:
                        if (c < 0x20) sb.Append("\\u").Append(((int)c).ToString("x4"));
                        else sb.Append(c);
                        break;
                }
            }
            sb.Append('"');
            return sb.ToString();
        }

        public static FixResponse ParseFixResponse(string raw)
        {
            string text = raw ?? "";
            int s = text.IndexOf('{');
            int e = text.LastIndexOf('}');
            if (s >= 0 && e > s) text = text.Substring(s, e - s + 1);

            // 键名统一小写，避免模型大小写不一致导致反序列化失败
            text = Regex.Replace(text, "\"([A-Za-z_][A-Za-z0-9_]*)\"(?=\\s*:)",
                m => "\"" + m.Groups[1].Value.ToLowerInvariant() + "\"");

            try
            {
                var serializer = new DataContractJsonSerializer(typeof(FixResponse));
                using (var ms = new MemoryStream(Encoding.UTF8.GetBytes(text)))
                {
                    return (FixResponse)serializer.ReadObject(ms);
                }
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException("模型未按约定返回 JSON。原始响应：" + Truncate(raw, 400), ex);
            }
        }

        private static string Truncate(string s, int n)
        {
            return s != null && s.Length > n ? s.Substring(0, n) + "…" : s;
        }
    }
}
