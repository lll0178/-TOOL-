#pragma once
// ============================================================================
// algo/math/euler_phi.hpp —— 欧拉函数
// ----------------------------------------------------------------------------
// 功能：
//   - phi(n)：单点欧拉函数 φ(n)（1..n 中与 n 互质的数的个数）。
//   - phi_table(n)：线性筛预处理 1..n 的 φ 值。
// 时间复杂度：phi O(sqrt(n))；phi_table O(n)。
// 空间复杂度：phi O(1)；phi_table O(n)。
// 使用示例：
//   #include "math/euler_phi.hpp"
//   ll v = algo::phi(12);              // 4
//   auto tab = algo::phi_table(10);    // tab[6] == 2
// 注意事项：
//   1. φ 只对正整数定义：phi(n) 在 n < 1 时返回 0。
//   2. phi_table 返回 vector<int>，索引 0 为 0，φ(1) 定义为 1。
//   3. phi_table 的 n 受 int 上限约束；如需更大范围可改为 long long。
// ============================================================================
#include "algo/common.hpp"

#include <vector>

namespace algo {

// 单点欧拉函数：试除分解质因数，O(sqrt n)。
inline ll phi(ll n) {
    if (n < 1) return 0;
    ll res = n;
    for (ll p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            res = res / p * (p - 1);  // 先除后乘防溢出
            while (n % p == 0) n /= p;
        }
    }
    if (n > 1) res = res / n * (n - 1);
    return res;
}

// 线性筛预处理 1..n 的欧拉函数。
inline std::vector<int> phi_table(int n) {
    if (n < 0) n = 0;
    std::vector<int> phi(n + 1, 0);
    std::vector<int> primes;
    std::vector<bool> is_comp(n + 1, false);
    if (n >= 1) phi[1] = 1;
    for (int i = 2; i <= n; ++i) {
        if (!is_comp[i]) {
            primes.push_back(i);
            phi[i] = i - 1;  // 素数 p 的 φ = p - 1
        }
        for (int p : primes) {
            if ((ll)p * i > n) break;
            is_comp[p * i] = true;
            if (i % p == 0) {
                phi[p * i] = phi[i] * p;  // p 已含在 i 中
                break;
            } else {
                phi[p * i] = phi[i] * (p - 1);
            }
        }
    }
    return phi;
}

}  // namespace algo
