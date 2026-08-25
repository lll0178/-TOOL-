#pragma once
// ============================================================================
// misc/timer.hpp —— 计时与内存
// ----------------------------------------------------------------------------
// 功能：基于 std::chrono::steady_clock 的计时器（毫秒/秒），以及 Linux 下的
//       进程内存占用统计（读取 /proc/self/statm）。
// 时间复杂度：O(1)。
// 空间复杂度：O(1)。
// 使用示例：
//   #include "misc/timer.hpp"
//   algo::Timer t;
//   // ... 被测代码 ...
//   double ms = t.elapsed_ms();
//   t.reset();
//   long long kb = algo::memory_usage_kb();
// 注意事项：
//   - steady_clock 单调递增，不受系统时间调整影响，适合测时。
//   - memory_usage_kb 仅在 Linux（__linux__）实现，返回常驻内存 RSS 的 KB 数；
//     其他平台返回 -1（不依赖 windows.h，避免与用户代码宏冲突）。
// ============================================================================
#include "algo/common.hpp"

#include <chrono>
#include <cstdio>

#ifdef __linux__
#include <unistd.h>  // sysconf / _SC_PAGESIZE（仅 Linux）
#endif

namespace algo {

// 计时器：构造时开始计时，elapsed_* 返回从构造/最近一次 reset 起经过的时间。
class Timer {
public:
    Timer() : start_(std::chrono::steady_clock::now()) {}

    // 经过的毫秒数
    double elapsed_ms() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }

    // 经过的秒数
    double elapsed_sec() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - start_).count();
    }

    // 重新开始计时
    void reset() {
        start_ = std::chrono::steady_clock::now();
    }

private:
    std::chrono::steady_clock::time_point start_;
};

// 进程内存占用（KB）。
// 仅 Linux 实现：读取 /proc/self/statm 的常驻内存页数并换算为 KB；
// 其他平台返回 -1。
inline ll memory_usage_kb() {
#ifdef __linux__
    std::FILE* f = std::fopen("/proc/self/statm", "r");
    if (f == nullptr) return -1;

    long long size_pages = 0;
    long long resident_pages = 0;
    // statm 格式：size resident shared text lib data dt（单位为页）
    int got = std::fscanf(f, "%lld %lld", &size_pages, &resident_pages);
    std::fclose(f);
    if (got != 2) return -1;

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;  // 兜底：默认 4KB 页
    return (resident_pages * page_size) / 1024;
#else
    return -1;
#endif
}

}  // namespace algo
