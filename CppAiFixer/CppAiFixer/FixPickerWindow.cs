using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

namespace CppAiFixer
{
    /// <summary>候选做法选择窗口：列出 AI 提出的多种做法 + 差异预览。</summary>
    internal sealed class FixPickerWindow : Window
    {
        public FixCandidate PickedCandidate { get; private set; }
        public bool SkipRequested { get; private set; }

        private readonly ListBox _list;
        private readonly TextBox _preview;

        public FixPickerWindow(BuildOutputError error, List<FixCandidate> candidates, bool single)
        {
            Title = "AI 修复做法 · 请选择";
            Width = 760;
            Height = 600;
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            ResizeMode = ResizeMode.CanResize;
            FontFamily = new FontFamily("Microsoft YaHei UI");
            Background = new SolidColorBrush(Color.FromRgb(0xF5, 0xF5, 0xF5));

            var grid = new Grid { Margin = new Thickness(12) };
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1.2, GridUnitType.Star) });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });

            var errText = new TextBlock
            {
                Text = string.Format("{0}  第 {1} 行  {2}  {3}",
                    Path.GetFileName(error.File ?? ""),
                    error.Line + 1,
                    string.IsNullOrEmpty(error.Code) ? "" : error.Code + ":",
                    error.Message ?? ""),
                FontWeight = FontWeights.SemiBold,
                TextWrapping = TextWrapping.Wrap,
            };
            Grid.SetRow(errText, 0);
            grid.Children.Add(errText);

            var hint = new TextBlock
            {
                Text = "以下做法由 AI 生成，选择后可预览差异；应用后可用 Ctrl+Z 撤销。",
                Margin = new Thickness(0, 6, 0, 4),
                Foreground = Brushes.Gray,
            };
            Grid.SetRow(hint, 1);
            grid.Children.Add(hint);

            _list = new ListBox { Margin = new Thickness(0, 0, 0, 4) };
            var dt = new DataTemplate(typeof(FixCandidate));
            var panel = new FrameworkElementFactory(typeof(StackPanel));
            panel.SetValue(StackPanel.OrientationProperty, Orientation.Vertical);
            var tTitle = new FrameworkElementFactory(typeof(TextBlock));
            tTitle.SetBinding(TextBlock.TextProperty, new Binding("Title"));
            tTitle.SetValue(TextBlock.FontWeightProperty, FontWeights.SemiBold);
            panel.AppendChild(tTitle);
            var tExpl = new FrameworkElementFactory(typeof(TextBlock));
            tExpl.SetBinding(TextBlock.TextProperty, new Binding("Explanation"));
            tExpl.SetValue(TextBlock.TextWrappingProperty, TextWrapping.Wrap);
            tExpl.SetValue(TextBlock.ForegroundProperty, Brushes.Gray);
            panel.AppendChild(tExpl);
            dt.VisualTree = panel;
            _list.ItemTemplate = dt;
            Grid.SetRow(_list, 2);
            grid.Children.Add(_list);

            var prevTitle = new TextBlock
            {
                Text = "预览（- 原文 / + 改为）：",
                FontWeight = FontWeights.SemiBold,
                Margin = new Thickness(0, 4, 0, 2),
            };
            Grid.SetRow(prevTitle, 3);
            grid.Children.Add(prevTitle);

            _preview = new TextBox
            {
                IsReadOnly = true,
                AcceptsReturn = true,
                VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
                HorizontalScrollBarVisibility = ScrollBarVisibility.Auto,
                FontFamily = new FontFamily("Consolas"),
                FontSize = 12,
                TextWrapping = TextWrapping.NoWrap,
                Background = Brushes.White,
            };
            Grid.SetRow(_preview, 4);
            grid.Children.Add(_preview);

            var buttons = new StackPanel
            {
                Orientation = Orientation.Horizontal,
                HorizontalAlignment = HorizontalAlignment.Right,
                Margin = new Thickness(0, 8, 0, 0),
            };
            var applyBtn = new Button
            {
                Content = single ? "应用此做法" : "应用此做法，继续下一个错误",
                Padding = new Thickness(14, 5, 14, 5),
                IsDefault = true,
                FontWeight = FontWeights.SemiBold,
            };
            applyBtn.Click += (s, e) =>
            {
                PickedCandidate = _list.SelectedItem as FixCandidate;
                DialogResult = true;
            };
            buttons.Children.Add(applyBtn);

            if (!single)
            {
                var skipBtn = new Button
                {
                    Content = "跳过此错误",
                    Padding = new Thickness(14, 5, 14, 5),
                    Margin = new Thickness(8, 0, 0, 0),
                };
                skipBtn.Click += (s, e) =>
                {
                    SkipRequested = true;
                    DialogResult = true;
                };
                buttons.Children.Add(skipBtn);
            }

            var cancelBtn = new Button
            {
                Content = "取消",
                Padding = new Thickness(14, 5, 14, 5),
                Margin = new Thickness(8, 0, 0, 0),
                IsCancel = true,
            };
            cancelBtn.Click += (s, e) => { DialogResult = false; };
            buttons.Children.Add(cancelBtn);
            Grid.SetRow(buttons, 5);
            grid.Children.Add(buttons);

            _list.SelectionChanged += (s, e) => UpdatePreview();
            _list.ItemsSource = candidates;
            if (candidates.Count > 0) _list.SelectedIndex = 0;

            Content = grid;
        }

        private void UpdatePreview()
        {
            var c = _list.SelectedItem as FixCandidate;
            if (c == null)
            {
                _preview.Text = "";
                return;
            }
            var sb = new StringBuilder();
            sb.AppendLine("做法: " + (c.Title ?? ""));
            if (!string.IsNullOrEmpty(c.Explanation)) sb.AppendLine("说明: " + c.Explanation);
            sb.AppendLine();
            foreach (var e in c.Edits ?? new List<FixEdit>())
            {
                sb.AppendLine("--- 原文:");
                sb.AppendLine(e.Old ?? "");
                sb.AppendLine("+++ 改为:");
                sb.AppendLine(e.New ?? "");
                sb.AppendLine();
            }
            _preview.Text = sb.ToString();
        }
    }
}
