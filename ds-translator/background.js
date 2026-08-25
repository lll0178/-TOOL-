// DeepSeek 翻译助手 —— Service Worker（后台）
// 负责调用 DeepSeek API。在扩展后台调用可绕过网页的 CORS 限制。

// 读取配置：API Key 存 local（不出本机），其余设置存 sync（跟随浏览器账号）
async function getConfig() {
  const [settings, secret] = await Promise.all([
    chrome.storage.sync.get({
      targetLang: "中文",
      model: "deepseek-chat",
      baseUrl: "https://api.deepseek.com"
    }),
    chrome.storage.local.get({ apiKey: "" })
  ]);
  return { ...settings, apiKey: secret.apiKey || "" };
}

// 调用 DeepSeek Chat Completions API 翻译
async function translate(text, config) {
  if (!config.apiKey) {
    throw new Error("尚未配置 API Key。请点击浏览器工具栏中的扩展图标，填入 DeepSeek API Key 后保存。");
  }
  if (!text || !text.trim()) {
    throw new Error("没有要翻译的文本。");
  }
  const base = (config.baseUrl || "https://api.deepseek.com").replace(/\/+$/, "");
  const res = await fetch(base + "/chat/completions", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      "Authorization": "Bearer " + config.apiKey
    },
    body: JSON.stringify({
      model: config.model || "deepseek-chat",
      messages: [
        {
          role: "system",
          content:
            "You are a professional translator. Translate the user's text into " +
            config.targetLang +
            ". Keep code, technical terms, proper nouns and original formatting where appropriate. " +
            "Output ONLY the translation, with no explanations, notes or extra text."
        },
        { role: "user", content: text }
      ],
      temperature: 0.3,
      stream: false
    })
  });

  if (!res.ok) {
    let detail = res.statusText;
    try {
      const err = await res.json();
      detail = (err && (err.error && err.error.message || err.message)) || JSON.stringify(err);
    } catch (_) { /* 保留默认 detail */ }
    throw new Error("API 请求失败 (HTTP " + res.status + ")：" + detail);
  }

  const data = await res.json();
  const content = data.choices && data.choices[0] && data.choices[0].message
    ? data.choices[0].message.content
    : "";
  if (!content) throw new Error("API 返回了空结果，请重试。");
  return content.trim();
}

// ---------- 右键菜单 ----------
function createContextMenu() {
  chrome.contextMenus.removeAll(() => {
    chrome.contextMenus.create({
      id: "ds-translate-selection",
      title: "翻译选中文本（DeepSeek）",
      contexts: ["selection"]
    });
  });
}

chrome.runtime.onInstalled.addListener(createContextMenu);
chrome.runtime.onStartup.addListener(createContextMenu);

chrome.contextMenus.onClicked.addListener((info, tab) => {
  if (info.menuItemId === "ds-translate-selection" && tab && tab.id != null) {
    // 通知该标签页的内容脚本读取选区并翻译
    chrome.tabs.sendMessage(tab.id, { type: "dsTranslateFromContextMenu" }).catch(() => {
      /* 页面刚打开、内容脚本未就绪时忽略 */
    });
  }
});

// ---------- 消息处理：内容脚本请求翻译 ----------
chrome.runtime.onMessage.addListener((msg, _sender, sendResponse) => {
  if (msg && msg.type === "dsTranslate") {
    (async () => {
      const config = await getConfig();
      const text = await translate(String(msg.text || ""), config);
      return { ok: true, text };
    })()
      .then(sendResponse)
      .catch((e) => sendResponse({ ok: false, error: (e && e.message) || String(e) }));
    return true; // 保持消息通道以异步返回
  }
});
