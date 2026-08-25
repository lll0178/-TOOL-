# AI C++ Fixer — VS2022 C/C++ AI 错误修复插件

读取 VS 错误列表中的 C/C++ 编译错误 / IntelliSense 错误，调用大模型（默认 DeepSeek）生成多种修复做法，
以灯泡 Quick Actions、右键菜单两种方式呈现，选择后预览差异再一键应用（可用 Ctrl+Z 撤销）。

## 功能

- **灯泡 Quick Actions**：光标停在错误行，出现灯泡，点击“AI 修复 …”
- **右键菜单**：“AI 修复光标处错误” / “AI 修复当前文件错误”（逐条修复整个文件，可跳过）
- **工具菜单**：同样两条命令
- **多个候选做法**：每个错误最多给出 N 种做法（默认 3），带标题、理由与差异预览
- **AI 引擎**：DeepSeek 官方接口，或任意 OpenAI 兼容接口（可换本地 Ollama 等）
- 支持 VS2022（17.x）与 VS2026（18.x）

## 构建

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" "CppAiFixer\CppAiFixer.csproj" /restore /t:Rebuild /p:Configuration=Release /m
```

产物：`CppAiFixer\CppAiFixer\bin\Release\CppAiFixer.vsix`

## 安装

1. 关闭所有 Visual Studio 实例
2. 双击 `CppAiFixer.vsix`，勾选 VS2022 Community（如装了 VS2026 也可以勾上），安装（需要管理员权限）
3. 重启 VS2022

命令行安装（管理员终端）：
```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VSIXInstaller.exe" /quiet "CppAiFixer\CppAiFixer\bin\Release\CppAiFixer.vsix"
```

**注意**：VS2022 不会把每用户扩展目录加入程序集探测路径，手工解压到 `%LOCALAPPDATA%\...\Extensions` 会导致包加载失败（FileNotFoundException），扩展会被 VS 视为损坏。必须安装到 VS 安装目录（机器级，需管理员）。

**一键安装（推荐）**：双击仓库根目录的 `install-vs2022.cmd`（弹出 UAC 时点“是”），或直接双击 `CppAiFixer.vsix`，在向导里勾选 VS2022 Community 后安装。

## 配置（必做）

**工具 > 选项 > AI C++ Fixer**：

- API 地址：默认 `https://api.deepseek.com`（OpenAI 兼容接口均可）
- API Key：DeepSeek 平台申请；或设置环境变量 `DEEPSEEK_API_KEY`
- 模型：默认 `deepseek-chat`

> 切换到其他 API（智谱 v4、通义、Kimi、硅基流动、本地 Ollama 等）的完整说明见 **[API配置指南.md](API配置指南.md)**。

## 使用

1. 先编译让错误进入错误列表（F7 生成 / Ctrl+F7 编译当前文件；IntelliSense 的红色波浪线错误也可以直接修）
2. 把光标放到报错行，点灯泡“AI 修复 …”，或右键选“AI 修复光标处错误”
3. 在弹窗里选择一种做法（可看差异预览），点“应用”
4. 想要批量：右键“AI 修复当前文件错误”，逐个错误确认，可“跳过”

## 工作原理

1. 读取 VS 输出窗口“生成”窗格的编译输出，解析 MSVC 错误行（Cxxxx / LNKxxxx，兼容中英文 VS）；灯泡由编辑器错误波浪线（IErrorTag）触发，未编译时用 IntelliSense 提示兜底
2. 取错误行上下各 N 行代码，连同错误码、错误信息发给模型，要求返回严格 JSON
3. 每个候选做法包含若干组“原文精确片段 → 替换片段”编辑
4. 应用时在缓冲区中按原文定位（多个匹配时选离错误行最近的），以 `ITextEdit` 写入并包成单个撤销事务

## 常见问题

- **灯泡不出现**：光标所在行没有错误波浪线（IntelliSense 尚未分析，或已编译通过）。先 Ctrl+F7 编译当前文件，再把光标移到报错行。
- **提示未配置 API Key**：到 工具 > 选项 > AI C++ Fixer 填写，或设置环境变量 `DEEPSEEK_API_KEY`。
- **本地模型**：API 地址填 `http://localhost:11434/v1`，模型填如 `qwen2.5-coder:7b`（需支持 OpenAI 格式与 JSON 输出；不支持 JSON 模式的模型，插件会自动去掉 json_object 参数，但输出格式可能不稳定）。
- **deepseek-reasoner**：不支持 JSON 输出模式，插件会自动关闭该参数，但建议用 deepseek-chat 保证格式稳定。
- **想调试开发**：用 VS2022 打开 `CppAiFixer.sln`，需要“Visual Studio 扩展开发”工作负载，按 F5 启动实验实例。
