using System.ComponentModel.Composition;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Utilities;

namespace CppAiFixer
{
    /// <summary>跟踪当前活动代码编辑器，供右键菜单命令使用。</summary>
    [Export(typeof(IWpfTextViewCreationListener))]
    [ContentType("code")]
    [TextViewRole(PredefinedTextViewRoles.Editable)]
    internal sealed class ActiveViewTracker : IWpfTextViewCreationListener
    {
        public static IWpfTextView LastActiveView { get; private set; }

        public void TextViewCreated(IWpfTextView textView)
        {
            LastActiveView = textView;
            textView.GotAggregateFocus += (s, e) => LastActiveView = textView;
            textView.Closed += (s, e) =>
            {
                if (ReferenceEquals(LastActiveView, textView)) LastActiveView = null;
            };
        }
    }
}
