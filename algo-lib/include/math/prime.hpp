#pragma once
// ============================================================================
// algo/math/prime.hpp —— 素数筛 / Miller-Rabin 素性测试 / Pollard-Rho 分解
// ----------------------------------------------------------------------------
// 功能：
//   - sieve(n)：埃氏筛，返回 is_prime[0..n]。
//   - euler_sieve(n)：欧拉线性筛，返回 1..n 内所有素数。
//   - is_prime(n)：Miller-Rabin 素性测试，确定性（n < 2^64）。
//   - factorize(n)：Pollard-Rho + Miller-Rabin 质因数分解，返回 {质因子, 次数} 升序。
// 时间复杂度：
//   - 埃氏筛 O(n log log n)；欧拉筛 O(n)。
//   - is_prime：O(k log^2 n)，k 为固定基底个数（12）。
//   - factorize：期望 O(n^{1/4})（Pollard-Rho，实际对 64 位整数极快）。
// 空间复杂度：埃氏筛 O(n)；欧拉筛 O(n)；is_prime/factorize O(1)（递归栈另计）。
// 使用示例：
//   #include "math/prime.hpp"
//   auto primes = algo::euler_sieve(100);
//   bool p = algo::is_prime(1000000007LL);
//   auto fac = algo::factorize(600851475143LL);  // { {71,1},{839,1},{1471,1},{6857,1} }
// 注意事项：
//   1. is_prime 对 n < 2 返回 false；固定基底 {2,3,5,7,11,13,17,19,23,29,31,37} 在
//      n < 2^64 范围内是确定性的，无需随机。
//   2. factorize 内部先试除 <= 100 的素数，再用 Pollard-Rho 处理大因子，返回结果升序。
//   3. factorize(n = 1) 返回空 vector。
// ============================================================================
#include "algo/common.hpp"
#include "math/fastpow.hpp"
#include "math/gcd.hpp"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

namespace algo {

// 埃氏筛：返回 is_prime[0..n]，其中 is_prime[i] 表示 i 是否为素数。
inline std::vector<bool> sieve(int n) {
    if (n < 0) n = 0;
    std::vector<bool> is_prime(n + 1, true);
    if (n >= 0) is_prime[0] = false;
    if (n >= 1) is_prime[1] = false;
    for (ll i = 2; i * i <= n; ++i) {
        if (is_prime[(int)i]) {
            for (ll j = i * i; j <= n; j += i) is_prime[(int)j] = false;
        }
    }
    return is_prime;
}

// 欧拉线性筛：返回 1..n 内所有素数（升序）。
inline std::vector<int> euler_sieve(int n) {
    if (n < 0) n = 0;
    std::vector<int> primes;
    std::vector<bool> is_comp(n + 1, false);
    for (int i = 2; i <= n; ++i) {
        if (!is_comp[i]) primes.push_back(i);
        for (int p : primes) {
            if ((ll)p * i > n) break;
            is_comp[p * i] = true;
            if (i % p == 0) break;  // 保证每个合数只被其最小质因子筛掉
        }
    }
    return primes;
}

// Miller-Rabin 素性测试（确定性，适用于 n < 2^64）。
inline bool is_prime(ll n) {
    if (n < 2) return false;
    // 先试除小素数，加速并处理小 n。
    static const ll small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (ll p : small) {
        if (n % p == 0) return n == p;
    }
    // 记 n - 1 = d * 2^r，d 为奇数。
    ll d = n - 1;
    int r = 0;
    while ((d & 1) == 0) {
        d >>= 1;
        ++r;
    }
    for (ll p : small) {
        ll x = qpow(p % n, d, n);
        if (x == 1 || x == n - 1) continue;
        bool ok = false;
        for (int i = 0; i < r - 1; ++i) {
            x = qmul(x, x, n);
            if (x == n - 1) {
                ok = true;
                break;
            }
        }
        if (!ok) return false;
    }
    return true;
}

namespace {

// Pollard-Rho 迭代函数 f(x) = (x^2 + c) mod n，用 ull 加法避免溢出。
inline ll rho_f(ll x, ll c, ll n) {
    ull v = (ull)qmul(x, x, n) + (ull)c;
    return (ll)(v % (ull)n);
}

// Pollard-Rho 寻找 n 的一个非平凡因子（n 必须为合数且 > 3）。
inline ll pollard_rho(ll n) {
    static std::mt19937_64 rng(0x9E3779B97F4A7C15ULL);
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;
    while (true) {
        // 随机起点与随机参数 c（固定种子，保证可复现）。
        ll c = (ll)(rng() % (ull)(n - 1)) + 1;
        ll x = (ll)(rng() % (ull)(n - 1)) + 1;
        ll y = x;
        ll d = 1;
        while (d == 1) {
            x = rho_f(x, c, n);
            y = rho_f(rho_f(y, c, n), c, n);  // Floyd 判环
            ll diff = x > y ? x - y : y - x;
            d = gcd(diff, n);
        }
        if (d != n) return d;  // 找到非平凡因子；d == n 则重试
    }
}

// 递归分解，把每个质因子（可能重复）追加到 out 中。
inline void factor_rec(ll n, std::vector<ll>& out) {
    if (n == 1) return;
    if (is_prime(n)) {
        out.push_back(n);
        return;
    }
    ll d = pollard_rho(n);
    factor_rec(d, out);
    factor_rec(n / d, out);
}

}  // namespace

// 质因数分解：返回 {质因子, 次数}，按质因子升序；n <= 1 返回空。
inline std::vector<std::pair<ll, ll>> factorize(ll n) {
    std::vector<std::pair<ll, ll>> res;
    if (n <= 1) return res;
    // 先试除小素数（<= 100）。
    static const ll small[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
                               41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};
    for (ll p : small) {
        if (p * p > n) break;
        if (n % p == 0) {
            ll cnt = 0;
            while (n % p == 0) {
                n /= p;
                ++cnt;
            }
            res.push_back({p, cnt});
        }
    }
    if (n == 1) return res;  // 已全部分解完（res 天然升序）
    // 剩余部分用 Pollard-Rho 递归分解。
    std::vector<ll> rest;
    factor_rec(n, rest);
    std::sort(rest.begin(), rest.end());
    for (ll p : rest) {
        if (!res.empty() && res.back().first == p) {
            res.back().second++;
        } else {
            res.push_back({p, 1});
        }
    }
    return res;
}

}  // namespace algo
