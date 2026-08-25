# Toolbox · 个人工具集

个人开发工具与作品的合集仓库，包含算法模板库、VS 插件、浏览器扩展等。

## 目录结构

| 目录 / 文件 | 说明 |
| --- | --- |
| [algo-lib](algo-lib/) | C++ 算法与数据结构模板库（ACM/OI 板子库），header-only，即拷即用，自带文档与冒烟测试 |
| [CppAiFixer](CppAiFixer/) | VS2022 / VS2026 的 C/C++ AI 错误修复插件（DeepSeek 驱动），含发布包与安装脚本 |
| [ds-translator](ds-translator/) | DeepSeek 翻译助手浏览器扩展（Chrome / Edge），选中网页英文一键翻译 |
| [change_txt](change_txt/) | 文本处理小工具（C++，交互式命令行） |
| [牛客app](牛客app/) | C++ Win32 小程序：双击用系统浏览器打开牛客网 |
| [使用指南.txt](使用指南.txt) | 身体数据记录器 v1.0 详细使用指南 |

## 使用说明

- **algo-lib**：`#include "algo_lib.h"` 或按需拷贝单个头文件，详见 `algo-lib/README.md`
- **CppAiFixer**：编译安装见 `CppAiFixer/README.md`，发布包在 `CppAiFixer/发布包/`（含安装插件.cmd）
- **ds-translator**：浏览器扩展，`edge://extensions`（或 `chrome://extensions`）开启开发者模式后加载本目录，详见 `ds-translator/README.md`
- **牛客app**：VS2022 打开 `牛客app.sln`，Release/x64 生成后双击 exe 即可，详见 `牛客app/README.md`

## 环境

- Windows 10/11 + Visual Studio 2022（C++，x64）
- C++17 / MSVC，仅依赖 C++ 标准库（algo-lib）
- Chrome / Edge 浏览器（ds-translator）
