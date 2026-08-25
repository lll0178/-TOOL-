# AI C++ Fixer · API 配置指南

插件通过标准的 **OpenAI 兼容 Chat Completions 协议** 调用大模型：
实际请求地址 = `{API 地址}/chat/completions`。任何支持该协议的云端服务或本地服务都可以接入。

## 一、配置入口

**工具 > 选项 > AI C++ Fixer**，三个关键字段：

| 字段 | 说明 |
|---|---|
| API 地址 | 接口的 base URL（不带 /chat/completions 后缀） |
| API Key | 服务商的密钥；留空时读环境变量 `DEEPSEEK_API_KEY` |
| 模型 | 模型名，按服务商文档填写 |

## 二、常见服务配置示例

| 服务 | API 地址 | 模型示例 | 备注 |
|---|---|---|---|
| DeepSeek 官方（默认） | `https://api.deepseek.com` | `deepseek-chat` | 推荐；`deepseek-reasoner` 不支持 JSON 输出模式，插件会自动关闭该参数 |
| 智谱 GLM（v4 接口） | `https://open.bigmodel.cn/api/paas/v4` | `glm-4-flash` / `glm-4.5` / 按官网最新 | 如果你说的 V4 是指智谱开放平台的 v4 接口路径，就是这样填 |
| 通义千问 | `https://dashscope.aliyuncs.com/compatible-mode/v1` | `qwen-plus` / `qwen-max` | 用兼容模式地址 |
| Moonshot Kimi | `https://api.moonshot.cn/v1` | `kimi-k2-turbo-preview` | |
| 硅基流动 SiliconFlow | `https://api.siliconflow.cn/v1` | `deepseek-ai/DeepSeek-V3` 等 | 可跑各种开源模型 |
| Ollama 本地模型 | `http://localhost:11434/v1` | `qwen2.5-coder:7b` | 免费离线，但修复质量看模型 |
| 其他 OpenAI 兼容服务 | 服务商文档里的 base URL | 服务商文档里的模型名 | 只要能调 `/chat/completions` 即可 |

> 若你指的 V4 是别的供应商（比如某个聚合 API 的 v4 版本），原理一样：
> 把它的 base URL 填进 API 地址、模型名填进模型、密钥填进 API Key 即可。

## 三、模型选择建议

1. **优先选支持 `response_format: json_object` 的模型**：插件靠模型返回严格 JSON 来生成修复做法，
   支持 JSON 输出模式的模型结果最稳定（插件对名称不含 `reasoner` 的模型会自动开启该模式）。
2. **推理模型（如 deepseek-reasoner）**：插件会自动关闭 JSON 模式，能用但格式偶尔不稳定，
   出错时会提示“模型未按约定返回 JSON”。
3. **代码修复场景**：指令跟随强的 chat 模型通常比纯聊天模型效果好。

## 四、用环境变量配 Key（不写进选项页）

```powershell
setx DEEPSEEK_API_KEY "你的密钥"
```
设置后重启 VS 生效；选项页里 API Key 留空即可。

## 五、验证与排错

改完配置后：在一个 C++ 文件里故意写个错（如删个分号）→ Ctrl+F7 编译 → 光标放报错行 → 右键“AI 修复光标处错误”。

| 现象 | 原因 |
|---|---|
| 弹窗提示 HTTP 401 | API Key 错误或未填 |
| HTTP 404 | API 地址或模型名不对（注意服务商路径里是否要带 /v1、/v4 等） |
| 请求超时 | 地址不通 / 本地服务没启动 / 网络问题 |
| 模型未按约定返回 JSON | 模型不支持 JSON 输出模式，换个模型 |
| AI 没有给出修复做法 | 模型返回空候选，可换更强的模型或增大上下文行数 |
