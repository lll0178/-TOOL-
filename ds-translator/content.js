// DeepSeek 翻译助手 —— 内容脚本
// 选中英文文本后显示浮动"译"按钮，点击后请求后台翻译并弹出翻译浮窗。

(() => {
  if (window.__dsTranslatorLoaded) return;
  window.__dsTranslatorLoaded = true;

  const MAX_LEN = 8000; // 单次最多翻译的字符数
  let lastSelection = ""; // 最近一次选中的文本（供右键菜单使用）
  let floatBtn = null; // 浮动翻译按钮
  let tooltip = null; // 翻译结果浮窗

  // ---------- 工具 ----------
  function getSelectedText() {
    // 优先处理输入框 / 文本域内的选区
    const el = document.activeElement;
    if (el && (el.tagName === "TEXTAREA" || el.tagName === "INPUT") && el.selectionStart != null) {
      return el.value.substring(el.selectionStart, el.selectionEnd);
    }
    const sel = window.getSelection();
    return sel ? sel.toString() : "";
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, (c) => ({
      "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;"
    }[c]));
  }

  function isEnglish(text) {
    return /[A-Za-z]{2,}/.test(text);
  }

  // ---------- 浮动按钮 ----------
  function showFloatBtn(x, y) {
    hideFloatBtn();
    floatBtn = document.createElement("div");
    floatBtn.id = "ds-translator-btn";
    floatBtn.textContent = "译";
    floatBtn.title = "用 DeepSeek 翻译选中文本";
    // 防止 mousedown 导致选区消失
    floatBtn.addEventListener("mousedown", (e) => e.preventDefault());
    floatBtn.addEventListener("click", (e) => {
      e.stopPropagation();
      hideFloatBtn();
      doTranslate(lastSelection);
    });
    document.documentElement.appendChild(floatBtn);
    positionFloatBtn(x, y);
  }

  function positionFloatBtn(x, y) {
    const r = floatBtn.getBoundingClientRect();
    let left = x + 8;
    let top = y - r.height - 6;
    if (left + r.width > window.innerWidth - 4) left = x - r.width - 8;
    if (top < 4) top = y + 14;
    floatBtn.style.left = left + "px";
    floatBtn.style.top = top + "px";
  }

  function hideFloatBtn() {
    if (floatBtn) { floatBtn.remove(); floatBtn = null; }
  }

  // ---------- 翻译浮窗 ----------
  function showTooltip(x, y, html, onCopy) {
    hideTooltip();
    tooltip = document.createElement("div");
    tooltip.id = "ds-translator-tooltip";
    tooltip.innerHTML = html;
    document.documentElement.appendChild(tooltip);

    const copyBtn = tooltip.querySelector(".ds-t-copy");
    if (copyBtn && onCopy) copyBtn.addEventListener("click", () => onCopy());
    const closeBtn = tooltip.querySelector(".ds-t-close");
    if (closeBtn) closeBtn.addEventListener("click", hideTooltip);

    const r = tooltip.getBoundingClientRect();
    let left = Math.min(Math.max(8, x - r.width / 2), window.innerWidth - r.width - 8);
    let top = y - r.height - 10;
    if (top < 8) top = y + 16;
    tooltip.style.left = left + "px";
    tooltip.style.top = top + "px";
  }

  function hideTooltip() {
    if (tooltip) { tooltip.remove(); tooltip = null; }
  }

  function markCopied() {
    const btn = tooltip && tooltip.querySelector(".ds-t-copy");
    if (btn) {
      btn.textContent = "已复制 ✓";
      setTimeout(() => { btn.textContent = "复制"; }, 1500);
    }
  }

  function legacyCopy(text) {
    const ta = document.createElement("textarea");
    ta.value = text;
    ta.style.position = "fixed";
    ta.style.opacity = "0";
    document.body.appendChild(ta);
    ta.select();
    let ok = false;
    try { ok = document.execCommand("copy"); } catch (_) { /* 忽略 */ }
    ta.remove();
    return ok;
  }

  function copyText(text) {
    if (navigator.clipboard && window.isSecureContext) {
      navigator.clipboard.writeText(text).then(markCopied).catch(() => { legacyCopy(text); markCopied(); });
    } else {
      legacyCopy(text);
      markCopied();
    }
  }

  // ---------- 翻译主流程 ----------
  function selectionRect() {
    const sel = window.getSelection();
    if (sel && sel.rangeCount) {
      const rect = sel.getRangeAt(0).getBoundingClientRect();
      if (rect && (rect.width || rect.height)) return rect;
    }
    return null;
  }

  async function doTranslate(rawText) {
    let text = (rawText || "").trim();
    if (!text) return;

    const rect = selectionRect();
    const x = rect ? Math.min(rect.right, window.innerWidth - 10) : window.innerWidth / 2;
    const y = rect ? rect.top : window.innerHeight / 2;

    if (text.length > MAX_LEN) {
      text = text.slice(0, MAX_LEN) + "\n…（内容过长，已截断）";
    }

    showTooltip(x, y,
      '<div class="ds-t-head">DeepSeek 翻译中…</div>' +
      '<div class="ds-t-body ds-t-loading">正在翻译，请稍候…</div>');

    try {
      const resp = await chrome.runtime.sendMessage({ type: "dsTranslate", text });
      if (resp && resp.ok) {
        const body = escapeHtml(resp.text).replace(/\n/g, "<br>");
        showTooltip(x, y,
          '<div class="ds-t-head"><span>翻译结果</span><span class="ds-t-actions">' +
          '<button class="ds-t-copy">复制</button><button class="ds-t-close">×</button></span></div>' +
          '<div class="ds-t-body">' + body + "</div>",
          () => copyText(resp.text));
      } else {
        showErrorTooltip(x, y, (resp && resp.error) || "翻译失败，请重试。");
      }
    } catch (e) {
      showErrorTooltip(x, y, (e && e.message) || "无法连接到扩展后台，请刷新页面后重试。");
    }
  }

  function showErrorTooltip(x, y, msg) {
    const body = escapeHtml(msg).replace(/\n/g, "<br>");
    showTooltip(x, y,
      '<div class="ds-t-head"><span>翻译失败</span><span class="ds-t-actions">' +
      '<button class="ds-t-close">×</button></span></div>' +
      '<div class="ds-t-body ds-t-error">' + body + "</div>");
  }

  // ---------- 事件 ----------
  document.addEventListener("mouseup", (e) => {
    if (e.target && e.target.closest && e.target.closest("#ds-translator-btn, #ds-translator-tooltip")) return;
    setTimeout(() => {
      const text = getSelectedText().trim();
      lastSelection = text;
      if (text && isEnglish(text)) {
        const rect = selectionRect();
        showFloatBtn(rect ? rect.right : e.clientX, rect ? rect.top : e.clientY);
      } else {
        hideFloatBtn();
      }
    }, 0);
  });

  document.addEventListener("mousedown", (e) => {
    if (e.target && e.target.closest && e.target.closest("#ds-translator-btn, #ds-translator-tooltip")) return;
    hideFloatBtn();
  });

  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape") { hideFloatBtn(); hideTooltip(); }
  });

  window.addEventListener("scroll", () => hideFloatBtn(), true);

  // 右键菜单触发：由后台转发的消息
  chrome.runtime.onMessage.addListener((msg) => {
    if (msg && msg.type === "dsTranslateFromContextMenu") {
      const text = (getSelectedText() || "").trim() || lastSelection;
      if (text) doTranslate(text);
    }
  });

  window.addEventListener("beforeunload", () => { hideFloatBtn(); hideTooltip(); });
})();
