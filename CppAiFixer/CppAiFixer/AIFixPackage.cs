using System;
using System.ComponentModel.Design;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.VisualStudio;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Shell.Interop;
using Task = System.Threading.Tasks.Task;

namespace CppAiFixer
{
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    // 把插件所在目录注册为程序集绑定路径（VSIX 加载本包程序集的关键机制）
    [ProvideBindingPath]
    [InstalledProductRegistration("AI C++ Fixer", "AI 驱动的 C/C++ 错误修复助手（灯泡/右键提出多种修复做法）", "1.0.2")]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [ProvideOptionPage(typeof(AiFixerOptionsPage), "AI C++ Fixer", "通用", 101, 102, true)]
    // SolutionExists UI 上下文 GUID
    [ProvideAutoLoad("{f1536ef8-92ec-443c-9ed7-fdadf150da82}", PackageAutoLoadFlags.BackgroundLoad)]
    [Guid(PackageGuids.PackageGuidString)]
    public sealed class AIFixPackage : AsyncPackage
    {
        public static AIFixPackage Instance { get; private set; }

        public AiFixerOptionsPage Options
        {
            get
            {
                try { return (AiFixerOptionsPage)GetDialogPage(typeof(AiFixerOptionsPage)); }
                catch { return null; }
            }
        }

        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
        {
            await ThreadHelper.JoinableTaskFactory.SwitchToMainThreadAsync(cancellationToken);
            Instance = this;

            try
            {
                var mcs = (await GetServiceAsync(typeof(IMenuCommandService))) as OleMenuCommandService;
                if (mcs != null)
                {
                    ErrorFixCommands.Register(this, mcs);
                }
            }
            catch (Exception ex)
            {
                ActivityLog.TryLogError("AI C++ Fixer", "InitializeAsync 失败: " + ex);
            }
        }
    }

    internal static class PackageGuids
    {
        public const string PackageGuidString = "b7e4a9c2-6f3d-4c8a-9e21-5a7b3d1f9c44";
        public const string CmdSetGuidString = "c8d5b0e3-7a4e-4d9b-8f32-6b8c4e2a0d55";
        public static readonly Guid CmdSet = new Guid(CmdSetGuidString);
    }

    internal static class PackageIds
    {
        public const int FixAtCaretCmdId = 0x0100;
        public const int FixFileCmdId = 0x0101;
        public const int FixAtCaretToolsCmdId = 0x0102;
        public const int FixFileToolsCmdId = 0x0103;
    }
}
