#pragma once
// ============================================================================
// algo/math/inverse.hpp —— 逆元
// ----------------------------------------------------------------------------
// 功能：
//   - inv_fermat(a, mod)：费马小定理求逆元（mod 必须为质数）。
//   - inv_exgcd(a, mod)：扩展欧几里得求逆元（只需 a 与 mod 互质）。
//   - inv_table(n, mod)：O(n) 线性递推 1..n 的逆元（mod 质数且 mod > n）。
//   - fact_table(n, mod) / inv_fact_table(n, mod)：阶乘与阶乘逆元预处理。
// 时间复杂度：inv_fermat O(log mod)；inv_exgcd O(log mod)；三个表 O(n)。
// 空间复杂度：单点 O(1)；表 O(n)。
// 使用示例：
//   #include "math/inverse.hpp"
//   ll iv = algo::inv_fermat(3, 1000000007);
//   auto inv = algo::inv_table(100000, 1000000007);
// 注意事项：
//   1. inv_fermat 要求 mod 为质数；inv_exgcd 要求 gcd(a, mod) == 1。
//   2. inv_table 要求 mod 为质数且 mod > n（保证 mod % i != 0）。
//   3. fact_table/inv_fact_table 要求 mod 为质数且 n < mod（保证 fact[n] 非零）；
//      两者内部的乘法假设 mod 不大（如 <= 1e9，防止乘法溢出）。
// ============================================================================
#include "algo/common.hpp"
#include "math/fastpow.hpp"
#include "math/gcd.hpp"

#include <vector>

namespace algo {

// 费马小定理求逆元：a^(mod-2) mod mod，要求 mod 为质数且 a 非零。
inline ll inv_fermat(ll a, ll mod) {
    a %= mod;
    if (a < 0) a += mod;
    return qpow(a, mod - 2, mod);
}

// 扩展欧几里得求逆元：求 a 在 mod 下的逆元，要求 gcd(a, mod) == 1。
inline ll inv_exgcd(ll a, ll mod) {
    a %= mod;
    if (a < 0) a += mod;
    ll x, y;
    exgcd(a, mod, x, y);
    x %= mod;
    if (x < 0) x += mod;
    return x;
}

// O(n) 线性递推 1..n 的逆元（mod 质数且 mod > n）。
inline std::vector<ll> inv_table(int n, ll mod) {
    if (n < 0) n = 0;
    std::vector<ll> inv(n + 1, 0);
    if (n >= 1) inv[1] = 1 % mod;
    for (int i = 2; i <= n; ++i) {
        inv[i] = (mod - mod / i) * inv[mod % i] % mod;
    }
    return inv;
}

// 阶乘预处理：fact[i] = i! mod mod。
inline std::vector<ll> fact_table(int n, ll mod) {
    if (n < 0) n = 0;
    std::vector<ll> fact(n + 1, 1 % mod);
    for (int i = 1; i <= n; ++i) {
        fact[i] = fact[i - 1] * (i % mod) % mod;
    }
    return fact;
}

// 阶乘逆元预处理：inv_fact[i] = (i!)^(-1) mod mod（用费马小定理求最后一项）。
inline std::vector<ll> inv_fact_table(int n, ll mod) {
    if (n < 0) n = 0;
    std::vector<ll> fact = fact_table(n, mod);
    std::vector<ll> inv_fact(n + 1);
    inv_fact[n] = qpow(fact[n], mod - 2, mod);
    for (int i = n; i >= 1; --i) {
        inv_fact[i - 1] = inv_fact[i] * (i % mod) % mod;
    }
    return inv_fact;
}

}  // namespace algo
