// DeepSeek 翻译助手 —— 设置弹窗

document.addEventListener("DOMContentLoaded", async () => {
  const [settings, secret] = await Promise.all([
    chrome.storage.sync.get({ targetLang: "中文", model: "deepseek-chat", baseUrl: "https://api.deepseek.com" }),
    chrome.storage.local.get({ apiKey: "" })
  ]);
  document.getElementById("apiKey").value = secret.apiKey || "";
  document.getElementById("targetLang").value = settings.targetLang || "中文";
  document.getElementById("model").value = settings.model || "deepseek-chat";
  document.getElementById("baseUrl").value = settings.baseUrl || "https://api.deepseek.com";
});

document.getElementById("save").addEventListener("click", async () => {
  const apiKey = document.getElementById("apiKey").value.trim();
  const targetLang = document.getElementById("targetLang").value.trim() || "中文";
  const model = document.getElementById("model").value;
  const baseUrl = document.getElementById("baseUrl").value.trim() || "https://api.deepseek.com";

  await Promise.all([
    chrome.storage.local.set({ apiKey }),
    chrome.storage.sync.set({ targetLang, model, baseUrl })
  ]);

  const status = document.getElementById("status");
  status.textContent = "✓ 已保存";
  status.className = "ok";
  setTimeout(() => { status.textContent = ""; status.className = ""; }, 2000);
});
