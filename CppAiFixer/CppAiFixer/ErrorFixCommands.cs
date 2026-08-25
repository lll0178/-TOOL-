using System;
using System.ComponentModel.Design;
using System.Linq;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;

namespace CppAiFixer
{
    /// <summary>右键菜单 / 工具菜单命令。</summary>
    internal static class ErrorFixCommands
    {
        public static void Register(AIFixPackage package, OleMenuCommandService mcs)
        {
            var caret = new CommandID(PackageGuids.CmdSet, PackageIds.FixAtCaretCmdId);
            mcs.AddCommand(new OleMenuCommand((s, e) => ExecuteCaret(package), caret));

            var file = new CommandID(PackageGuids.CmdSet, PackageIds.FixFileCmdId);
            mcs.AddCommand(new OleMenuCommand((s, e) => ExecuteFile(package), file));

            var caretTools = new CommandID(PackageGuids.CmdSet, PackageIds.FixAtCaretToolsCmdId);
            mcs.AddCommand(new OleMenuCommand((s, e) => ExecuteCaret(package), caretTools));

            var fileTools = new CommandID(PackageGuids.CmdSet, PackageIds.FixFileToolsCmdId);
            mcs.AddCommand(new OleMenuCommand((s, e) => ExecuteFile(package), fileTools));
        }

        private static void ExecuteCaret(AIFixPackage package)
        {
            _ = ThreadHelper.JoinableTaskFactory.RunAsync(async () =>
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                var view = ActiveViewTracker.LastActiveView;
                if (view == null || view.IsClosed)
                {
                    ShowInfo(package, "没有活动的代码编辑器。请先把光标放在 C/C++ 文件里再试。");
                    return;
                }
                if (!FixFlow.IsCppFile(view.TextBuffer))
                {
                    ShowInfo(package, "当前文件不是 C/C++ 文件。");
                    return;
                }
                string docPath = FixFlow.GetDocumentPath(view.TextBuffer);
                var errors = BuildOutputReader.ParseErrors(BuildOutputReader.ReadBuildOutput(package))
                    .Where(e => BuildOutputReader.SameFile(e.File, docPath)).ToList();
                if (errors.Count == 0)
                {
                    ShowInfo(package, "生成输出中没有发现该文件的 C/C++ 错误。请先编译（F7 / Ctrl+F7）后再试。");
                    return;
                }
                int caretLine = view.Caret.Position.BufferPosition.GetContainingLine().LineNumber;
                var err = errors.OrderBy(x => Math.Abs(x.Line - caretLine)).First();
                await FixFlow.RunSingleErrorAsync(package, view, err, single: true);
            });
        }

        private static void ExecuteFile(AIFixPackage package)
        {
            _ = ThreadHelper.JoinableTaskFactory.RunAsync(async () =>
            {
                await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync();
                var view = ActiveViewTracker.LastActiveView;
                if (view == null || view.IsClosed)
                {
                    ShowInfo(package, "没有活动的代码编辑器。请先把光标放在 C/C++ 文件里再试。");
                    return;
                }
                if (!FixFlow.IsCppFile(view.TextBuffer))
                {
                    ShowInfo(package, "当前文件不是 C/C++ 文件。");
                    return;
                }
                await FixFlow.RunFileErrorsAsync(package, view);
            });
        }

        private static void ShowInfo(AIFixPackage package, string msg)
        {
            VsShellUtilities.ShowMessageBox(package, msg, "AI C++ Fixer",
                OLEMSGICON.OLEMSGICON_INFO, OLEMSGBUTTON.OLEMSGBUTTON_OK, OLEMSGDEFBUTTON.OLEMSGDEFBUTTON_FIRST);
        }
    }
}
