using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Microsoft.VisualStudio.ComponentModelHost;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text.Operations;

namespace CppAiFixer
{
    /// <summary>核心修复流程：读编译错误 → 问 AI → 弹窗选做法 → 预览 → 应用。</summary>
    internal static class FixFlow
    {
        public static bool IsCppFile(ITextBuffer buffer)
        {
            string path = GetDocumentPath(buffer);
            if (string.IsNullOrEmpty(path)) return false;
            string ext = Path.GetExtension(path).ToLowerInvariant();
            return ext == ".c" || ext == ".h" || ext == ".cpp" || ext == ".cc"
                || ext == ".cxx" || ext == ".hpp" || ext == ".hxx" || ext == ".inl";
        }

        public static string GetDocumentPath(ITextBuffer buffer)
        {
            if (buffer == null) return null;
            if (buffer.Properties.TryGetProperty(typeof(ITextDocument), out ITextDocument doc))
            {
                return doc.FilePath;
            }
            return null;
        }

        public static AiFixerOptionsPage GetOptions(IServiceProvider sp)
        {
            var pkg = AIFixPackage.Instance;
            if (pkg == null && sp != null)
            {
                try { pkg = sp.GetService(typeof(AIFixPackage)) as AIFixPackage; } catch { }
            }
            return pkg?.Options ?? new AiFixerOptionsPage();
        }

        public static string GetApiKey(AiFixerOptionsPage opts)
        {
            if (!string.IsNullOrWhiteSpace(opts.ApiKey)) return opts.ApiKey.Trim();
            string env = Environment.GetEnvironmentVariable("DEEPSEEK_API_KEY");
            return string.IsNullOrWhiteSpace(env) ? "" : env.Trim();
        }

        /// <summary>灯泡流程：优先用编译输出定位错误，没有就用波浪线提示兜底。</summary>
        public static async Task RunCaretFixAsync(IServiceProvider sp, ITextView view, string tagText)
        {
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            var opts = GetOptions(sp);
            string key = GetApiKey(opts);
            if (key.Length == 0)
            {
                ShowKeyMissing(sp);
                return;
            }

            string docPath = GetDocumentPath(view.TextBuffer);
            int caretLine = view.Caret.Position.BufferPosition.GetContainingLine().LineNumber;

            var errors = BuildOutputReader.ParseErrors(BuildOutputReader.ReadBuildOutput(sp))
                .Where(e => BuildOutputReader.SameFile(e.File, docPath)).ToList();
            BuildOutputError err = null;
            if (errors.Count > 0)
            {
                err = errors.OrderBy(e => Math.Abs(e.Line - caretLine)).First();
            }
            if (err == null)
            {
                string tip = CleanTagText(tagText);
                if (string.IsNullOrWhiteSpace(tip))
                {
                    ShowInfo(sp, "没有找到该文件的编译错误。请先 Ctrl+F7 编译当前文件，再把光标移到报错行。");
                    return;
                }
                var m = Regex.Match(tip, @"\b(?:C|E|LNK|MSB)\d{3,4}\b");
                err = new BuildOutputError
                {
                    File = docPath,
                    Line = caretLine,
                    Column = 0,
                    Code = m.Success ? m.Value : "E?",
                    Message = tip,
                };
            }
            await TryFixOneAsync(sp, view, err, opts, key, single: true);
        }

        /// <summary>右键命令：修复光标处错误（仅用编译输出）。</summary>
        public static async Task RunSingleErrorAsync(IServiceProvider sp, ITextView view, BuildOutputError error, bool single)
        {
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            var opts = GetOptions(sp);
            string key = GetApiKey(opts);
            if (key.Length == 0)
            {
                ShowKeyMissing(sp);
                return;
            }
            await TryFixOneAsync(sp, view, error, opts, key, single);
        }

        /// <summary>逐条修复当前文件的全部编译错误（右键/工具菜单）。</summary>
        public static async Task RunFileErrorsAsync(IServiceProvider sp, ITextView view)
        {
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
            var opts = GetOptions(sp);
            string key = GetApiKey(opts);
            if (key.Length == 0)
            {
                ShowKeyMissing(sp);
                return;
            }

            string docPath = GetDocumentPath(view.TextBuffer);
            var errors = CurrentFileErrors(sp, docPath);
            if (errors.Count == 0)
            {
                int r = VsShellUtilities.ShowMessageBox(sp,
                    "生成输出中没有发现该文件的 C/C++ 错误。是否先编译当前文件（Ctrl+F7）再修复？",
                    "AI C++ Fixer", OLEMSGICON.OLEMSGICON_QUERY, OLEMSGBUTTON.OLEMSGBUTTON_YESNO, OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
                if (r != 6) return;
                await CompileCurrentFileAsync(sp);
                errors = CurrentFileErrors(sp, docPath);
                if (errors.Count == 0)
                {
                    ShowInfo(sp, "编译完成，仍未发现该文件的 C/C++ 错误。");
                    return;
                }
            }

            int total = Math.Min(errors.Count, 10);
            var skipped = new HashSet<string>();
            int done = 0;
            while (done < total)
            {
                var current = CurrentFileErrors(sp, docPath)
                    .Where(e => !skipped.Contains(Key(e)))
                    .OrderBy(e => e.Line)
                    .ToList();
                if (current.Count == 0) break;

                var err = current[0];
                int action = await TryFixOneAsync(sp, view, err, opts, key, single: false);
                if (action == 2) break;
                if (action == 1) skipped.Add(Key(err));
                done++;
            }

            SetStatus(sp, "");
            if (done > 0) ShowInfo(sp, "已处理 " + done + " 个错误。建议重新编译确认。");
        }

        private static List<BuildOutputError> CurrentFileErrors(IServiceProvider sp, string docPath)
        {
            try
            {
                return BuildOutputReader.ParseErrors(BuildOutputReader.ReadBuildOutput(sp))
                    .Where(e => BuildOutputReader.SameFile(e.File, docPath)).ToList();
            }
            catch
            {
                return new List<BuildOutputError>();
            }
        }

        private static string Key(BuildOutputError e)
        {
            return (e.Code ?? "") + "|" + (e.Message ?? "") + "|" + e.Line;
        }

        private static async Task CompileCurrentFileAsync(IServiceProvider sp)
        {
            SetStatus(sp, "AI C++ Fixer: 正在编译当前文件…");
            try
            {
                var dte = BuildOutputReader.GetDte(sp);
                dte?.ExecuteCommand("Build.Compile");
            }
            catch { }
            string prev = "";
            for (int i = 0; i < 60; i++)
            {
                await Task.Delay(1500);
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                string cur = BuildOutputReader.ReadBuildOutput(sp) ?? "";
                if (cur.Length > 0 && cur == prev) break;
                prev = cur;
            }
        }

        /// <summary>返回 0=已应用 1=跳过 2=取消全部 -1=失败。</summary>
        private static async Task<int> TryFixOneAsync(IServiceProvider sp, ITextView view, BuildOutputError error, AiFixerOptionsPage opts, string key, bool single)
        {
            ITextBuffer buffer = view.TextBuffer;
            string docPath = GetDocumentPath(buffer);
            string language = GetLanguage(docPath);

            SetStatus(sp, "AI C++ Fixer: 正在请求 AI 生成修复做法…");
            FixResponse resp;
            string context;
            try
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                var snapshot = buffer.CurrentSnapshot;
                context = BuildContext(snapshot, error.Line, Math.Max(2, opts.ContextLines));
                resp = await DeepSeekClient.RequestCandidatesAsync(opts, key, docPath, language, error, context);
            }
            catch (Exception ex)
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                SetStatus(sp, "");
                ShowError(sp, "请求 AI 失败：" + ex.Message);
                return -1;
            }

            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();

            var candidates = (resp?.Candidates ?? new List<FixCandidate>())
                .Where(c => c != null && c.Edits != null && c.Edits.Count > 0)
                .Take(Math.Max(1, Math.Min(5, opts.MaxCandidates)))
                .ToList();
            if (candidates.Count == 0)
            {
                SetStatus(sp, "");
                ShowInfo(sp, "AI 没有给出修复做法。");
                return 1;
            }

            var win = new FixPickerWindow(error, candidates, single);
            bool? result = win.ShowDialog();
            if (result != true)
            {
                SetStatus(sp, "");
                return single ? -1 : 2;
            }
            if (win.SkipRequested)
            {
                SetStatus(sp, "");
                return 1;
            }
            if (win.PickedCandidate == null)
            {
                SetStatus(sp, "");
                return 1;
            }

            try
            {
                bool applied = ApplyCandidate(buffer, win.PickedCandidate, error.Line, sp);
                SetStatus(sp, "");
                if (!applied)
                {
                    ShowInfo(sp, "无法应用该做法：目标文本在当前文件中未找到（文件可能已变化）。");
                }
                return applied ? 0 : 1;
            }
            catch (Exception ex)
            {
                SetStatus(sp, "");
                ShowError(sp, "应用修复失败：" + ex.Message);
                return 1;
            }
        }

        public static bool ApplyCandidate(ITextBuffer buffer, FixCandidate candidate, int errorLine, IServiceProvider sp)
        {
            ThreadHelper.ThrowIfNotOnUIThread();
            ITextSnapshot snapshot = buffer.CurrentSnapshot;
            string text = snapshot.GetText();
            var edit = buffer.CreateEdit();
            bool ok = true;
            foreach (var e in candidate.Edits)
            {
                if (e.Old == null)
                {
                    ok = false;
                    break;
                }
                if (e.Old.Length == 0 && (e.New ?? "").Length == 0) continue;
                int pos = FindBestOccurrence(text, e.Old, errorLine, snapshot);
                if (pos < 0)
                {
                    ok = false;
                    break;
                }
                edit.Replace(new Span(pos, e.Old.Length), e.New ?? "");
            }
            if (!ok)
            {
                edit.Cancel();
                return false;
            }

            ApplyWithUndo(sp, buffer, edit);
            return true;
        }

        private static void ApplyWithUndo(IServiceProvider sp, ITextBuffer buffer, ITextEdit edit)
        {
            try
            {
                var cm = sp?.GetService(typeof(SComponentModel)) as IComponentModel;
                var registry = cm?.GetService<ITextUndoHistoryRegistry>();
                var history = registry?.GetHistory(buffer);
                if (history != null)
                {
                    using (var tx = history.CreateTransaction("AI C++ Fixer 修复"))
                    {
                        edit.Apply();
                        tx.Complete();
                    }
                    return;
                }
            }
            catch { }
            edit.Apply();
        }

        private static int FindBestOccurrence(string text, string old, int errorLine, ITextSnapshot snapshot)
        {
            int idx = text.IndexOf(old, StringComparison.Ordinal);
            if (idx < 0) return -1;
            if (idx == text.LastIndexOf(old, StringComparison.Ordinal)) return idx;

            int best = -1;
            int bestDist = int.MaxValue;
            int from = 0;
            while (true)
            {
                int i = text.IndexOf(old, from, StringComparison.Ordinal);
                if (i < 0) break;
                int line = snapshot.GetLineNumberFromPosition(i);
                int d = Math.Abs(line - errorLine);
                if (d < bestDist)
                {
                    bestDist = d;
                    best = i;
                }
                from = i + 1;
            }
            return best;
        }

        private static string BuildContext(ITextSnapshot snapshot, int errorLine0, int contextLines)
        {
            var sb = new StringBuilder();
            int total = snapshot.LineCount;
            int start = Math.Max(0, errorLine0 - contextLines);
            int end = Math.Min(total - 1, errorLine0 + contextLines);
            for (int i = start; i <= end; i++)
            {
                string line = snapshot.GetLineFromLineNumber(i).GetText();
                sb.Append(i + 1).Append(": ").Append(line);
                if (!line.EndsWith("\n")) sb.AppendLine();
            }
            return sb.ToString();
        }

        private static string GetLanguage(string docPath)
        {
            string ext = string.IsNullOrEmpty(docPath) ? "" : Path.GetExtension(docPath).ToLowerInvariant();
            return ext == ".c" || ext == ".h" ? "C" : "C++";
        }

        /// <summary>清洗波浪线 tooltip 文本。</summary>
        public static string CleanTagText(string t)
        {
            if (string.IsNullOrWhiteSpace(t)) return "";
            string s = t.Trim();
            s = Regex.Replace(s, @"^(?:错误|error|活動|活动|active)\s*(?:\([^)]*\))?\s*[:：]?\s*", "", RegexOptions.IgnoreCase);
            s = s.Replace("\r", " ").Replace("\n", " ");
            if (s.Length > 200) s = s.Substring(0, 200);
            return s.Trim();
        }

        private static void SetStatus(IServiceProvider sp, string text)
        {
            try
            {
                ThreadHelper.ThrowIfNotOnUIThread();
                if (sp?.GetService(typeof(SVsStatusbar)) is IVsStatusbar bar) bar.SetText(text);
            }
            catch { }
        }

        private static void ShowKeyMissing(IServiceProvider sp)
        {
            VsShellUtilities.ShowMessageBox(sp,
                "尚未配置 API Key。请到 工具 > 选项 > AI C++ Fixer 填写，或设置环境变量 DEEPSEEK_API_KEY。",
                "AI C++ Fixer", OLEMSGICON.OLEMSGICON_WARNING, OLEMSGBUTTON.OLEMSGBUTTON_OK, OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
        }

        private static void ShowInfo(IServiceProvider sp, string msg)
        {
            VsShellUtilities.ShowMessageBox(sp, msg, "AI C++ Fixer", OLEMSGICON.OLEMSGICON_INFO, OLEMSGBUTTON.OLEMSGBUTTON_OK, OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
        }

        private static void ShowError(IServiceProvider sp, string msg)
        {
            VsShellUtilities.ShowMessageBox(sp, msg, "AI C++ Fixer", OLEMSGICON.OLEMSGICON_CRITICAL, OLEMSGBUTTON.OLEMSGBUTTON_OK, OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
        }
    }
}
