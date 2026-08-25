#pragma once
// ============================================================================
// algo/math/fft.hpp —— 快速傅里叶变换卷积（复数 FFT）
// ----------------------------------------------------------------------------
// 功能：
//   - fft(a, invert)：迭代 FFT（位反转蝴蝶变换），invert=true 做逆变换。
//   - convolution(a, b)：两个整数序列的卷积，结果 round 到 long long。
// 时间复杂度：O(n log n)，n 为补到 2 的幂后的长度。
// 空间复杂度：O(n)。
// 使用示例：
//   #include "math/fft.hpp"
//   std::vector<long long> a = {1, 2, 3}, b = {4, 5};
//   auto c = algo::convolution(a, b);   // {4, 13, 22, 15}
// 注意事项：
//   1. 复数 FFT 存在浮点误差：当结果值域很大（如接近 1e15）时 round 可能出错，
//      建议用于值域 < 1e9 且长度 < 1e5 的场景；更大/更精确请改用 NTT。
//   2. 输入为空时返回空 vector；结果长度为 a.size()+b.size()-1。
// ============================================================================
#include "algo/common.hpp"

#include <cmath>
#include <complex>
#include <vector>

namespace algo {

// 迭代 FFT：invert = false 做正变换，true 做逆变换（结果已除以 n）。
inline void fft(std::vector<std::complex<double>>& a, bool invert) {
    int n = (int)a.size();
    // 位反转置换。
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * std::acos(-1.0) / len * (invert ? -1.0 : 1.0);
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = a[i + j];
                std::complex<double> v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert) {
        for (int i = 0; i < n; ++i) a[i] /= (double)n;
    }
}

// 复数 FFT 卷积：结果四舍五入到 long long。
inline std::vector<ll> convolution(const std::vector<ll>& a, const std::vector<ll>& b) {
    if (a.empty() || b.empty()) return {};
    int need = (int)a.size() + (int)b.size() - 1;
    int n = 1;
    while (n < need) n <<= 1;
    // 显式把整数系数转成复数，避免隐式转换告警。
    std::vector<std::complex<double>> fa(n), fb(n);
    for (int i = 0; i < (int)a.size(); ++i) fa[i] = (double)a[i];
    for (int i = 0; i < (int)b.size(); ++i) fb[i] = (double)b[i];
    fft(fa, false);
    fft(fb, false);
    for (int i = 0; i < n; ++i) fa[i] *= fb[i];
    fft(fa, true);
    std::vector<ll> res(need);
    for (int i = 0; i < need; ++i) res[i] = (ll)std::llround(fa[i].real());
    return res;
}

}  // namespace algo
