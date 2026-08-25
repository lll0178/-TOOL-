#pragma once
// ============================================================================
// algo/three_point.hpp —— 三分法求单峰函数极值
// ----------------------------------------------------------------------------
// 功能：
//   在 [l, r] 上对"单峰函数"（单谷：先减后增；或单峰：先增后减）求极值点。
//   - ternary_min：实数域求极小值点（返回自变量位置）；
//   - ternary_min_int：整数域求极小值点（返回自变量位置）。
//   求极大值时，对目标函数取负（写成 lambda 返回 -f(x)）再调用求极小即可。
// 时间复杂度：
//   实数版：O(iter) 次 f 调用（iter 默认 200）；
//   整数版：每次区间缩小到约 2/3，O(log_{3/2}(r - l + 1)) ≈ O(log n) 次调用。
// 空间复杂度：O(1)。
// 使用示例：
//   // 求 f(x) = (x-3)^2 在 [-100, 100] 的极小值点（约等于 3）
//   double x = algo::ternary_min(-100.0, 100.0,
//       [](double x) { return (x - 3.0) * (x - 3.0); });
//
//   // 整数域求 f(x) = x*x 在 [-10, 10] 的极小值点（0）
//   long long x = algo::ternary_min_int(-10LL, 10LL,
//       [](long long x) { return x * x; });
//
//   // 求极大值：对 f 取负
//   double xmax = algo::ternary_min(-10.0, 10.0,
//       [](double x) { return -(x - 2.0) * (x - 2.0); });
// 注意事项：
//   - 函数必须在区间内"单峰"（严格凸/凹）；存在平台（相邻值相等）时
//     仍能得到一个极值点，但不保证唯一；
//   - 实数版取 m1 = l + (r-l)/3、m2 = r - (r-l)/3，避免直接相加溢出；
//   - 整数版用 while (r - l > 2) 收缩，最后暴力检查剩余至多 3 个点，
//     对离散定义域同样正确；
//   - 要求 l <= r；区间为空时行为未定义，请调用前保证区间非空；
//   - 若 f 的计算有整数溢出风险（如 x*x），请自行改用更大类型或防溢出乘法。
// ============================================================================

#include "algo/common.hpp"  // algo::ll

namespace algo {

// 实数域三分求极小值点（单谷函数：先减后增）
template<typename F>
double ternary_min(double l, double r, F f, int iter = 200) {
    for (int i = 0; i < iter; ++i) {
        double m1 = l + (r - l) / 3.0;
        double m2 = r - (r - l) / 3.0;
        if (f(m1) < f(m2)) {
            r = m2;   // m2 一侧更高，极小值在 m1 左侧
        } else {
            l = m1;   // m1 一侧更高（或相等），极小值在 m2 右侧
        }
    }
    return (l + r) / 2.0;  // 区间已收缩到足够小，取中点作为结果
}

// 整数域三分求极小值点（单谷函数：先减后增）
template<typename F>
ll ternary_min_int(ll l, ll r, F f) {
    // 区间长度 > 2 时不断用三等分点收缩
    while (r - l > 2) {
        ll m1 = l + (r - l) / 3;
        ll m2 = r - (r - l) / 3;
        if (f(m1) < f(m2)) {
            r = m2;   // 极小值在 m1 左侧
        } else {
            l = m1;   // 极小值在 m2 右侧
        }
    }
    // 剩余至多 3 个点，逐个检查取最小
    ll best = l;
    ll best_val = f(l);
    for (ll x = l + 1; x <= r; ++x) {
        ll val = f(x);
        if (val < best_val) {
            best_val = val;
            best = x;
        }
    }
    return best;
}

}  // namespace algo
