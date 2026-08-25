using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.TextManager.Interop;

namespace CppAiFixer
{
    /// <summary>一条解析出的 MSVC 编译错误。</summary>
    public sealed class BuildOutputError
    {
        public string File { get; set; }
        public int Line { get; set; }        // 0-based
        public int Column { get; set; }      // 0-based
        public string Code { get; set; }
        public string Message { get; set; }
    }

    /// <summary>读取并解析“生成”输出窗格中的 C/C++ 编译错误。</summary>
    internal static class BuildOutputReader
    {
        /// <summary>“生成”输出窗格 GUID。</summary>
        private static readonly Guid BuildPaneGuid = new Guid("{1BD8A850-02D1-11D1-BEE7-00A0C913F1C8}");

        private static readonly Regex ErrorRegex = new Regex(
            @"(?m)^\s*(?<file>[^\r\n(]+?)\s*\((?<line>\d+)(?:,(?<col>\d+))?\)\s*:\s*" +
            @"(?:(?:fatal\s+)?error|错误|致命错误)\s+(?<code>(?:C|E|LNK|MSB)\d+)\s*:\s*(?<msg>[^\r\n]+)\s*$",
            RegexOptions.Compiled);

        /// <summary>读取“生成”输出窗格全文（逐行，避免 GetLineText 截断）。失败返回空串。</summary>
        public static string ReadBuildOutput(IServiceProvider sp)
        {
            var sb = new StringBuilder();
            try
            {
                ThreadHelper.ThrowIfNotOnUIThread();
                var outWin = sp?.GetService(typeof(SVsOutputWindow)) as IVsOutputWindow;
                if (outWin == null) return "";
                Guid paneGuid = BuildPaneGuid;
                IVsOutputWindowPane pane = null;
                if (outWin.GetPane(ref paneGuid, out pane) != 0 || pane == null)
                {
                    // 窗格尚未创建：先创建（隐藏）再获取
                    outWin.CreatePane(ref paneGuid, "生成", 0, 1);
                    outWin.GetPane(ref paneGuid, out pane);
                }
                var textView = pane as IVsTextView;
                if (textView == null) return "";
                textView.GetBuffer(out IVsTextLines lines);
                if (lines == null) return "";
                lines.GetLastLineIndex(out int lastLine, out int lastIndex);
                if (lastLine < 0) return "";
                for (int i = 0; i <= lastLine; i++)
                {
                    lines.GetLengthOfLine(i, out int len);
                    if (len <= 0)
                    {
                        sb.AppendLine();
                        continue;
                    }
                    lines.GetLineText(i, 0, i, len, out string text);
                    sb.AppendLine(text ?? "");
                }
                return sb.ToString();
            }
            catch
            {
                return sb.ToString();
            }
        }

        /// <summary>获取 DTE（仅用于 ExecuteCommand 触发编译）。须在 UI 线程调用。</summary>
        public static EnvDTE.DTE GetDte(IServiceProvider sp)
        {
            try
            {
                ThreadHelper.ThrowIfNotOnUIThread();
                return sp?.GetService(typeof(EnvDTE.DTE)) as EnvDTE.DTE;
            }
            catch
            {
                return null;
            }
        }

        /// <summary>从编译输出文本解析 MSVC 错误行（兼容中英文 VS）。</summary>
        public static List<BuildOutputError> ParseErrors(string output)
        {
            var result = new List<BuildOutputError>();
            if (string.IsNullOrEmpty(output)) return result;
            foreach (Match m in ErrorRegex.Matches(output))
            {
                string code = m.Groups["code"].Value;
                if (code.StartsWith("MSB", StringComparison.OrdinalIgnoreCase)) continue; // 跳过 MSBuild 错误
                result.Add(new BuildOutputError
                {
                    File = m.Groups["file"].Value.Trim(),
                    Line = Math.Max(0, int.Parse(m.Groups["line"].Value) - 1),
                    Column = m.Groups["col"].Success ? Math.Max(0, int.Parse(m.Groups["col"].Value) - 1) : 0,
                    Code = code,
                    Message = m.Groups["msg"].Value.Trim(),
                });
            }
            return result;
        }

        /// <summary>错误里的文件与当前文档是否同一个（全路径比较，兜底文件名比较）。</summary>
        public static bool SameFile(string errFile, string docPath)
        {
            if (string.IsNullOrEmpty(errFile) || string.IsNullOrEmpty(docPath)) return false;
            if (string.Equals(errFile, docPath, StringComparison.OrdinalIgnoreCase)) return true;
            if (string.Equals(errFile.Replace('/', '\\'), docPath.Replace('/', '\\'), StringComparison.OrdinalIgnoreCase)) return true;
            return string.Equals(Path.GetFileName(errFile), Path.GetFileName(docPath), StringComparison.OrdinalIgnoreCase);
        }
    }
}
