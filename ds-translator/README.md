# 🈶 DeepSeek 翻译助手（浏览器扩展）

一个 Chrome / Edge 浏览器扩展：**选中网页上的英文，一键调用 DeepSeek API 翻译成中文**。
专门为天天刷 GitHub、Codeforces 的同学设计，拯救眼睛 👀。

## 功能

- 📝 **选中即译**：在任意网页选中英文，旁边浮出一个蓝色“译”按钮，点击立即翻译
- 🖱️ **右键菜单**：选中文字后右键 → “翻译选中文本（DeepSeek）”，同样可以翻译
- 💬 **翻译浮窗**：结果直接显示在选区旁边，支持一键复制、× 关闭、Esc 关闭
- ⚙️ **可配置**：API Key、目标语言（默认中文）、模型（deepseek-chat / deepseek-reasoner）都可改
- 🔒 **隐私**：API Key 只保存在本机浏览器里（`chrome.storage.local`），不会上传到任何地方

## 安装步骤（以 Edge 为例，Chrome 同理）

1. 打开浏览器，地址栏输入 `edge://extensions`（Chrome 是 `chrome://extensions`）
2. 打开左下角（或右上角）的 **“开发人员模式”** 开关
3. 点击 **“加载解压缩的扩展”**，选择本文件夹：`E:\Dyer\tool\ds-translator`
4. 看到“DeepSeek 翻译助手”出现在列表里，安装完成 ✅

## 配置 API Key

1. 没有 Key 的话，先去 [platform.deepseek.com](https://platform.deepseek.com) 注册
2. 左侧菜单 **“API Keys”** → **“创建 API Key”** → 复制以 `sk-` 开头的密钥
3. 点击浏览器工具栏里的扩展图标 🧩 → 把 Key 粘贴进 **API Key** 输入框 → **保存设置**
4. 顺便可以改“目标语言”（默认中文）和“模型”（默认 `deepseek-chat`，便宜又快）

## 使用方式

在 **GitHub、Codeforces** 或任何网页上：

1. 用鼠标选中一段英文（问题描述、README、issue、题解都行）
2. 点击选区旁边浮出的 **“译”** 按钮 —— 或右键选择 **“翻译选中文本（DeepSeek）”**
3. 翻译结果会显示在浮窗里，点 **复制** 即可粘贴使用

## 注意事项

- 单次最多翻译 **8000 个字符**，超出会自动截断（防止 API 费用失控）
- 翻译需要联网；失败时会显示具体原因（如 API Key 错误、余额不足等）
- 不要在 `chrome://` 内置页面、扩展商店页面上使用（浏览器禁止扩展在这些页面运行）
- 本插件免费开源，调用 DeepSeek API 产生的费用由你的 DeepSeek 账户承担（翻译一段题解通常只要几厘钱）

## 文件结构

```
ds-translator/
├── manifest.json    # 扩展清单（MV3）
├── background.js    # 后台 Service Worker：调用 DeepSeek API
├── content.js       # 内容脚本：选区检测、浮动按钮、翻译浮窗
├── content.css      # 按钮和浮窗样式
├── popup.html       # 设置弹窗
├── popup.js         # 设置弹窗逻辑
└── popup.css        # 设置弹窗样式
```

## 常见问题

**Q：点了“译”按钮没反应 / 提示“尚未配置 API Key”？**
A：点工具栏扩展图标，填入 API Key 并保存。

**Q：提示“API 请求失败 (HTTP 401)”？**
A：API Key 填错了或已失效，去 platform.deepseek.com 重新生成一个。

**Q：提示“余额不足”之类的 402 错误？**
A：去 platform.deepseek.com → 充值/查看用量。

**Q：想翻译成别的语言？**
A：扩展图标 → 把“目标语言”改成如 `英语`、`日语`、`English` 等，保存即可。
