# 牛客app

一个用 C++（Win32）写的小程序：**双击即可用系统默认浏览器打开牛客网**。

## 使用方法

1. 用 **Visual Studio 2022** 打开 `牛客app.sln`。
2. 选择 **Release / x64**（或其它配置），按 `F5` 或“生成解决方案”。
3. 编译出来的程序在 `x64\Release\牛客app.exe`（或对应配置目录）。
4. 双击 `牛客app.exe`，就会自动打开牛客网。

也可以把这个 exe 发送到桌面快捷方式，方便日常点击。
（本目录里已生成好一个“牛客”桌面快捷方式时，直接双击桌面图标即可。）

## 修改打开的网址

打开 `main.cpp`，修改这一行即可：

```cpp
static const wchar_t* kNiuKeUrl = L"https://www.nowcoder.com/";
```

## 程序图标

程序图标由文件夹里的 `下载.png` 生成（`app.ico`），已嵌入 exe。
想换图标：把新的 png 替换 `下载.png`，重新生成 `app.ico` 后重新编译即可。

## 文件说明

| 文件 | 说明 |
| --- | --- |
| `main.cpp` | 程序入口：调用 ShellExecute 打开浏览器 |
| `app.rc` / `resource.h` | 资源文件：把图标嵌入 exe |
| `app.ico` | 程序图标（由 `下载.png` 生成） |
| `下载.png` | 原始图标图片 |
| `牛客app.sln` / `牛客app.vcxproj` | VS2022 解决方案 / 工程文件 |
