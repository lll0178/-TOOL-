#pragma once
// ============================================================================
// algo/math/mobius.hpp —— 莫比乌斯函数与整除分块
// ----------------------------------------------------------------------------
// 功能：
//   - mobius_table(n)：线性筛求 μ(1..n)。
//   - floor_blocks(n)：整除分块，返回所有使 floor(n/i) 相等的连续段 [l, r]。
// 时间复杂度：mobius_table O(n)；floor_blocks O(sqrt(n))。
// 空间复杂度：mobius_table O(n)；floor_blocks O(sqrt(n))。
// 使用示例：
//   #include "math/mobius.hpp"
//   auto mu = algo::mobius_table(100);
//   for (auto& seg : algo::floor_blocks(100)) { /* [seg.first, seg.second] */ }
// 注意事项：
//   1. μ 的定义：μ(1)=1；含平方因子时为 0；否则为 (-1)^(不同质因子个数)。
//   2. floor_blocks 返回的段 [l, r] 满足 floor(n/l)==floor(n/r)，l 从 1 开始；
//      对 n <= 0 返回空。
//
//   莫比乌斯反演公式：
//     若 g(n) = Σ_{d|n} f(d)，则 f(n) = Σ_{d|n} μ(d) * g(n/d)。
//   典型应用（gcd=1 计数）：
//     Σ_{i=1..n} Σ_{j=1..m} [gcd(i,j)=1] = Σ_{d=1..min(n,m)} μ(d) * floor(n/d) * floor(m/d)
//     结合整除分块可把内层求和降到 O(sqrt n)。
//
//   整除分块加速示例（求 Σ_{i=1..n} floor(n/i)，O(sqrt n)）：
//     ll ans = 0;
//     for (ll l = 1, r; l <= n; l = r + 1) {
//         ll v = n / l;
//         r = n / v;
//         ans += v * (r - l + 1);
//     }
// ============================================================================
#include "algo/common.hpp"

#include <utility>
#include <vector>

namespace algo {

// 线性筛求莫比乌斯函数 μ(1..n)。
inline std::vector<int> mobius_table(int n) {
    if (n < 0) n = 0;
    std::vector<int> mu(n + 1, 0);
    std::vector<int> primes;
    std::vector<bool> is_comp(n + 1, false);
    if (n >= 1) mu[1] = 1;
    for (int i = 2; i <= n; ++i) {
        if (!is_comp[i]) {
            primes.push_back(i);
            mu[i] = -1;  // 素数：一个质因子
        }
        for (int p : primes) {
            if ((ll)p * i > n) break;
            is_comp[p * i] = true;
            if (i % p == 0) {
                mu[p * i] = 0;  // 含平方因子
                break;
            } else {
                mu[p * i] = -mu[i];
            }
        }
    }
    return mu;
}

// 整除分块：把 i=1..n 划分为若干段 [l, r]，每段内 floor(n/i) 相等。
inline std::vector<std::pair<ll, ll>> floor_blocks(ll n) {
    std::vector<std::pair<ll, ll>> res;
    if (n <= 0) return res;
    for (ll l = 1, r; l <= n; l = r + 1) {
        ll v = n / l;
        r = n / v;  // 使 floor(n/r) == v 的最大 r
        res.push_back({l, r});
    }
    return res;
}

}  // namespace algo
