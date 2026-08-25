using System.ComponentModel;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.Shell;

namespace CppAiFixer
{
    /// <summary>工具 &gt; 选项 &gt; AI C++ Fixer 设置页。</summary>
    [Guid("a5f0d2b7-8c41-4e6f-9b3d-2a7c1e5f4d80")]
    public class AiFixerOptionsPage : DialogPage
    {
        [Category("AI 接口")]
        [DisplayName("API 地址")]
        [Description("OpenAI 兼容接口地址。默认 DeepSeek 官方；本地模型可填如 http://localhost:11434/v1")]
        public string BaseUrl { get; set; } = "https://api.deepseek.com";

        [Category("AI 接口")]
        [DisplayName("API Key")]
        [Description("DeepSeek API Key。留空时读取环境变量 DEEPSEEK_API_KEY。")]
        [PasswordPropertyText(true)]
        public string ApiKey { get; set; } = "";

        [Category("AI 接口")]
        [DisplayName("模型")]
        [Description("模型名，如 deepseek-chat。注意 deepseek-reasoner 不支持 JSON 输出模式，本插件会自动为其关闭该选项。")]
        public string Model { get; set; } = "deepseek-chat";

        [Category("行为")]
        [DisplayName("上下文行数")]
        [Description("每个错误提交给模型的上下文行数（错误行上下各取这么多行）。")]
        public int ContextLines { get; set; } = 10;

        [Category("行为")]
        [DisplayName("候选做法数上限")]
        [Description("每个错误最多给出几种修复做法（1-5）。")]
        public int MaxCandidates { get; set; } = 3;

        [Category("行为")]
        [DisplayName("超时(秒)")]
        [Description("请求 AI 的超时时间。")]
        public int TimeoutSeconds { get; set; } = 120;

        [Category("行为")]
        [DisplayName("随机性 temperature")]
        [Description("0 最稳定，越大越发散（0-2）。")]
        public double Temperature { get; set; } = 0.2;
    }
}
