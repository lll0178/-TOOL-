#pragma once
// ============================================================================
// algo/math/fastpow.hpp —— 快速幂 / 快速乘
// ----------------------------------------------------------------------------
// 功能：提供防溢出的模意义快速乘 qmul 与快速幂 qpow。
//       qmul 用"二进制拆分"把乘法转成多次加法取模，避免 a*b 中间值溢出，
//       因此可支持 mod 高达 ~9e18（long long 上限）的场景。
//       qpow 内部全部使用 qmul，mod 很大时同样不会溢出。
// 时间复杂度：O(log b)
// 空间复杂度：O(1)
// 使用示例：
//   #include "math/fastpow.hpp"
//   ll x = algo::qmul(1000000000000000000LL, 1000000000000000000LL, 1000000007);
//   ll y = algo::qpow(2, 60, 1000000007);   // 2^60 mod p
// 注意事项：
//   1. mod 必须为正数；a、b 可以为负数（内部先转成 [0, mod) 再计算）。
//   2. 指数 b 必须非负。
//   3. qmul 内部使用 unsigned long long 做加法：当 mod <= LLONG_MAX 时，
//      中间和恒不超过 ULLONG_MAX，因此不会溢出。
// ============================================================================
#include "algo/common.hpp"

namespace algo {

// 二进制快速乘：返回 (a * b) % mod，防溢出。
// 负数先取模并修正到 [0, mod)。
inline ll qmul(ll a, ll b, ll mod) {
    a %= mod;
    if (a < 0) a += mod;
    b %= mod;
    if (b < 0) b += mod;
    ull ua = (ull)a, ub = (ull)b, um = (ull)mod;
    ull res = 0;
    while (ub > 0) {
        if (ub & 1ULL) res = (res + ua) % um;
        ua = (ua + ua) % um;
        ub >>= 1;
    }
    return (ll)res;
}

// 快速幂：返回 (a^b) % mod，要求 b >= 0。
// 内部乘法使用 qmul，防止大模数下溢出。
inline ll qpow(ll a, ll b, ll mod) {
    a %= mod;
    if (a < 0) a += mod;
    ll res = 1 % mod;  // 处理 mod == 1 的退化情形（结果恒为 0）
    while (b > 0) {
        if (b & 1LL) res = qmul(res, a, mod);
        a = qmul(a, a, mod);
        b >>= 1;
    }
    return res;
}

}  // namespace algo
