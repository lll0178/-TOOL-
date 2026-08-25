#pragma once
// ============================================================================
// algo/math/combinatorics.hpp —— 组合数学
// ----------------------------------------------------------------------------
// 功能：
//   - Comb：预处理阶乘/阶乘逆元，O(1) 求组合数 C、排列数 A、阶乘、阶乘逆元。
//   - lucas(n, k, p)：Lucas 定理求 C(n,k) mod p（p 为质数）。
//   - derangement(n, mod)：错排数 D(n)。
//   - catalan(n, mod)：卡特兰数。
//   - stirling2(n, mod)：第二类斯特林数递推表。
// 时间复杂度：
//   - Comb 预处理 O(max_n)，单次查询 O(1)。
//   - lucas O(p + log_p(n) * log p)。
//   - derangement / catalan O(n)。
//   - stirling2 O(n^2)。
// 空间复杂度：Comb O(max_n)；stirling2 O(n^2)；其余 O(1)（lucas 临时 O(p)）。
// 使用示例：
//   #include "math/combinatorics.hpp"
//   algo::Comb comb(100000, 1000000007);
//   ll c = comb.C(10, 3);
//   ll s2 = algo::stirling2(5, 1000000007)[5][2];
// 注意事项：
//   1. Comb 要求 mod 为质数且 max_n < mod（保证 fact[max_n] 可逆）；
//      n、k 越界（负数或超出 max_n）时 C/A/factorial/inv_factorial 返回 0。
//   2. lucas 要求 p 为质数，p 不宜太大（如 <= 1e6，因为要预处理 p 大小的阶乘表）。
//   3. catalan 要求 mod 为质数且 n+1 不被 mod 整除（通常 n < mod-1 即可）。
//   4. 本文件乘法假设 mod 不大（如 <= 1e9），否则乘法可能溢出 long long。
// ============================================================================
#include "algo/common.hpp"
#include "math/fastpow.hpp"

#include <vector>

namespace algo {

// 组合数预处理类：O(max_n) 预处理，O(1) 查询。
class Comb {
public:
    Comb(int max_n, ll mod)
        : max_n_(max_n), mod_(mod), fact_(max_n + 1), inv_fact_(max_n + 1) {
        fact_[0] = 1 % mod;
        for (int i = 1; i <= max_n; ++i) {
            fact_[i] = fact_[i - 1] * (i % mod) % mod;
        }
        inv_fact_[max_n] = qpow(fact_[max_n], mod - 2, mod);  // 费马小定理
        for (int i = max_n; i >= 1; --i) {
            inv_fact_[i - 1] = inv_fact_[i] * (i % mod) % mod;
        }
    }

    // 阶乘；越界返回 0。
    ll factorial(int n) const { return (n < 0 || n > max_n_) ? 0 : fact_[n]; }
    // 阶乘逆元；越界返回 0。
    ll inv_factorial(int n) const { return (n < 0 || n > max_n_) ? 0 : inv_fact_[n]; }

    // 组合数 C(n, k)；越界返回 0。
    ll C(int n, int k) const {
        if (n < 0 || k < 0 || k > n || n > max_n_) return 0;
        return fact_[n] * inv_fact_[k] % mod_ * inv_fact_[n - k] % mod_;
    }

    // 排列数 A(n, k)；越界返回 0。
    ll A(int n, int k) const {
        if (n < 0 || k < 0 || k > n || n > max_n_) return 0;
        return fact_[n] * inv_fact_[n - k] % mod_;
    }

private:
    int max_n_;
    ll mod_;
    std::vector<ll> fact_;
    std::vector<ll> inv_fact_;
};

// Lucas 定理：C(n, k) mod p，p 为质数（p 不宜太大，如 <= 1e6）。
inline ll lucas(ll n, ll k, ll p) {
    if (n < 0 || k < 0 || k > n) return 0;
    if (p < 2) return 0;  // p 必须为质数
    // 预处理 0..p-1 的阶乘。
    std::vector<ll> fact(p);
    fact[0] = 1 % p;
    for (ll i = 1; i < p; ++i) fact[i] = fact[i - 1] * i % p;
    ll res = 1 % p;
    // 递归式 C(n,k) = C(n/p, k/p) * C(n%p, k%p) 的迭代写法。
    while (n > 0 || k > 0) {
        ll ni = n % p, ki = k % p;
        if (ki > ni) return 0;  // 低位组合数为 0
        ll c = fact[ni] * qpow(fact[ki], p - 2, p) % p;
        c = c * qpow(fact[ni - ki], p - 2, p) % p;
        res = res * c % p;
        n /= p;
        k /= p;
    }
    return res;
}

// 错排数 D(n)：(n-1)*(D(n-1)+D(n-2))，D(0)=1, D(1)=0。
inline ll derangement(int n, ll mod) {
    if (n < 0) return 0;
    if (n == 0) return 1 % mod;
    if (n == 1) return 0;
    ll d0 = 1 % mod, d1 = 0;
    for (int i = 2; i <= n; ++i) {
        ll d2 = (ll)(i - 1) % mod * ((d0 + d1) % mod) % mod;
        d0 = d1;
        d1 = d2;
    }
    return d1;
}

// 卡特兰数：Catalan(n) = C(2n, n) / (n+1)。
inline ll catalan(int n, ll mod) {
    if (n < 0) return 0;
    ll c = lucas(2LL * n, (ll)n, mod);
    ll inv = qpow((ll)n + 1, mod - 2, mod);  // 要求 n+1 与 mod 互质
    return c * inv % mod;
}

// 第二类斯特林数递推表：S(n,k) = S(n-1,k-1) + k*S(n-1,k)，
// 返回 (n+1) x (n+1) 表，S[i][k] 可直接下标访问。
inline std::vector<std::vector<ll>> stirling2(int n, ll mod) {
    if (n < 0) n = 0;
    std::vector<std::vector<ll>> S(n + 1, std::vector<ll>(n + 1, 0));
    S[0][0] = 1 % mod;
    for (int i = 1; i <= n; ++i) {
        for (int k = 1; k <= i; ++k) {
            S[i][k] = (S[i - 1][k - 1] + (ll)k % mod * S[i - 1][k]) % mod;
        }
    }
    return S;
}

}  // namespace algo
