using System;
using System.Collections.Generic;
using System.ComponentModel.Composition;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Microsoft.VisualStudio.Imaging.Interop;
using Microsoft.VisualStudio.Language.Intellisense;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.Text;
using Microsoft.VisualStudio.Text.Editor;
using Microsoft.VisualStudio.Text.Tagging;
using Microsoft.VisualStudio.Utilities;

namespace CppAiFixer
{
    /// <summary>灯泡建议源：光标所在行有错误波浪线（IErrorTag）时提供 AI 修复动作。</summary>
    [Export(typeof(ISuggestedActionsSourceProvider))]
    [ContentType("code")]
    [Name("Cpp AI Fixer 灯泡")]
    internal sealed class CaretFixSuggestedActionsSourceProvider : ISuggestedActionsSourceProvider
    {
        [Import]
        internal SVsServiceProvider ServiceProvider { get; set; }

        [Import]
        internal IViewTagAggregatorFactoryService TagAggregatorFactory { get; set; }

        public ISuggestedActionsSource CreateSuggestedActionsSource(ITextView textView, ITextBuffer textBuffer)
        {
            if (textView == null || textBuffer == null) return null;
            return new CaretFixSuggestedActionsSource(this, textView, textBuffer);
        }
    }

    internal sealed class CaretFixSuggestedActionsSource : ISuggestedActionsSource
    {
        private readonly CaretFixSuggestedActionsSourceProvider _provider;
        private readonly ITextView _view;
        private readonly ITextBuffer _buffer;
        private readonly ITagAggregator<IErrorTag> _aggregator;
        private string _caretErrorText;

        public CaretFixSuggestedActionsSource(CaretFixSuggestedActionsSourceProvider provider, ITextView view, ITextBuffer buffer)
        {
            _provider = provider;
            _view = view;
            _buffer = buffer;
            _aggregator = provider.TagAggregatorFactory.CreateTagAggregator<IErrorTag>(view);
        }

        public event EventHandler<EventArgs> SuggestedActionsChanged { add { } remove { } }

        public async Task<bool> HasSuggestedActionsAsync(ISuggestedActionCategorySet requestedActionCategories, SnapshotSpan range, CancellationToken cancellationToken)
        {
            _caretErrorText = null;
            try
            {
                if (!FixFlow.IsCppFile(_buffer)) return false;
                var caret = _view.Caret.Position.BufferPosition;
                var line = caret.GetContainingLine();
                var span = new SnapshotSpan(line.Start, line.End);
                bool has = false;
                foreach (var mapping in _aggregator.GetTags(span))
                {
                    var tag = mapping.Tag;
                    if (tag == null) continue;
                    string et = (tag.ErrorType ?? "").ToLowerInvariant();
                    if (et.Contains("warning") || et.Contains("suggest") || et.Contains("hint")) continue;
                    has = true;
                    string tip = tag.ToolTipContent == null ? "" : tag.ToolTipContent.ToString();
                    if (!string.IsNullOrWhiteSpace(tip))
                    {
                        _caretErrorText = tip;
                        break;
                    }
                }
                return has;
            }
            catch
            {
                return false;
            }
        }

        public IEnumerable<SuggestedActionSet> GetSuggestedActions(ISuggestedActionCategorySet requestedActionCategories, SnapshotSpan range, CancellationToken cancellationToken)
        {
            string display = BuildDisplayText(_caretErrorText);
            var action = new AiFixSuggestedAction(_provider.ServiceProvider, _view, _caretErrorText, display);
            return new[]
            {
                new SuggestedActionSet(PredefinedSuggestedActionCategoryNames.CodeFix, new ISuggestedAction[] { action }),
            };
        }

        private static string BuildDisplayText(string tip)
        {
            string s = FixFlow.CleanTagText(tip);
            if (s.Length > 60) s = s.Substring(0, 60) + "…";
            return string.IsNullOrWhiteSpace(s) ? "AI 修复此处错误" : "AI 修复: " + s;
        }

        public bool TryGetTelemetryId(out Guid telemetryId)
        {
            telemetryId = Guid.Empty;
            return false;
        }

        public void Dispose()
        {
            try { _aggregator.Dispose(); } catch { }
        }
    }

    /// <summary>灯泡里的单个建议动作。</summary>
    internal sealed class AiFixSuggestedAction : ISuggestedAction
    {
        private readonly IServiceProvider _sp;
        private readonly ITextView _view;
        private readonly string _tagText;
        private readonly string _displayText;

        public AiFixSuggestedAction(IServiceProvider sp, ITextView view, string tagText, string displayText)
        {
            _sp = sp;
            _view = view;
            _tagText = tagText;
            _displayText = displayText;
        }

        public string DisplayText => _displayText;
        public bool HasActionSets => false;
        public bool HasPreview => false;
        public string IconAutomationText => "AI 修复";
        public string InputGestureText => null;
        public ImageMoniker IconMoniker => default(ImageMoniker);

        public void Invoke(CancellationToken cancellationToken)
        {
            _ = ThreadHelper.JoinableTaskFactory.RunAsync(async () =>
            {
                await FixFlow.RunCaretFixAsync(_sp, _view, _tagText);
            });
        }

        public Task<IEnumerable<SuggestedActionSet>> GetActionSetsAsync(CancellationToken cancellationToken)
        {
            return Task.FromResult<IEnumerable<SuggestedActionSet>>(new List<SuggestedActionSet>());
        }

        public Task<object> GetPreviewAsync(CancellationToken cancellationToken)
        {
            return Task.FromResult<object>(null);
        }

        public bool TryGetTelemetryId(out Guid telemetryId)
        {
            telemetryId = Guid.Empty;
            return false;
        }

        public void Dispose() { }
    }
}
