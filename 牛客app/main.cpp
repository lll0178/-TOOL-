// 牛客app —— 一键打开牛客网
// 双击运行后，自动用系统默认浏览器打开牛客网（www.nowcoder.com）。
// 想换成其它网址，只需修改下面的 kNiuKeUrl。

#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

// ========== 要打开的网址，可自行修改 ==========
static const wchar_t* kNiuKeUrl = L"https://www.nowcoder.com/problem/tracker#/daily";
// =============================================

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // 用系统默认浏览器打开网址
    HINSTANCE hResult = ShellExecuteW(nullptr,
                                      L"open",
                                      kNiuKeUrl,
                                      nullptr,
                                      nullptr,
                                      SW_SHOWNORMAL);

    // ShellExecute 返回值小于等于 32 表示调用失败
    if (reinterpret_cast<INT_PTR>(hResult) <= 32)
    {
        MessageBoxW(nullptr,
                    L"打开浏览器失败，请检查系统默认浏览器设置。",
                    L"牛客",
                    MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}
