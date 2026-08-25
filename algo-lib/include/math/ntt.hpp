#pragma once
// ============================================================================
// algo/math/ntt.hpp —— 数论变换 NTT 与多项式求逆
// ----------------------------------------------------------------------------
// 功能：
//   - NTT<MOD, G>::convolution(a, b)：模 MOD 意义下的整数卷积（MOD 为 NTT 友好质数）。
//   - poly_inv(a, n, mod)：多项式求逆（牛顿迭代倍增），返回 a 的逆元前 n 项。
// 时间复杂度：卷积 O(n log n)；poly_inv O(n log n)。
// 空间复杂度：O(n)。
// 使用示例：
//   #include "math/ntt.hpp"
//   std::vector<long long> a = {1, 2, 3}, b = {4, 5};
//   auto c = algo::NTT<998244353, 3>::convolution(a, b);
//   auto inv = algo::poly_inv({1, 1, 1}, 5, 998244353);
// 注意事项：
//   1. MOD 必须为 NTT 友好质数（形如 k*2^m+1），G 为其原根；补到 2 的幂的长度
//      必须整除 MOD-1。998244353 = 119*2^23+1，支持长度不超过 2^23。
//   2. 本实现假设 MOD 不大（如 <= 2e9），乘法不会溢出 long long。
//   3. poly_inv 固定使用 NTT<998244353, 3>，因此 mod 应传 998244353；
//      要求 a[0] 在该模数下非零（否则无逆），返回长度不足 n 时自动对齐 2 的幂。
// ============================================================================
#include "algo/common.hpp"
#include "math/fastpow.hpp"

#include <vector>

namespace algo {

// 数论变换（NTT）：MOD 为 NTT 友好质数，G 为其原根。
template <int MOD, int G>
struct NTT {
    static std::vector<ll> convolution(const std::vector<ll>& a, const std::vector<ll>& b) {
        if (a.empty() || b.empty()) return {};
        int need = (int)a.size() + (int)b.size() - 1;
        int n = 1;
        while (n < need) n <<= 1;  // 补到 2 的幂
        std::vector<ll> fa(a.begin(), a.end()), fb(b.begin(), b.end());
        fa.resize(n);
        fb.resize(n);
        for (ll& x : fa) {
            x %= MOD;
            if (x < 0) x += MOD;
        }
        for (ll& x : fb) {
            x %= MOD;
            if (x < 0) x += MOD;
        }
        ntt(fa, false);
        ntt(fb, false);
        for (int i = 0; i < n; ++i) fa[i] = fa[i] * fb[i] % MOD;
        ntt(fa, true);
        fa.resize(need);
        return fa;
    }

private:
    static void ntt(std::vector<ll>& a, bool invert) {
        int n = (int)a.size();
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            ll wlen = qpow(G, (MOD - 1) / len, MOD);
            if (invert) wlen = qpow(wlen, MOD - 2, MOD);
            for (int i = 0; i < n; i += len) {
                ll w = 1;
                for (int j = 0; j < len / 2; ++j) {
                    ll u = a[i + j];
                    ll v = a[i + j + len / 2] * w % MOD;
                    a[i + j] = (u + v) % MOD;
                    a[i + j + len / 2] = (u - v + MOD) % MOD;
                    w = w * wlen % MOD;
                }
            }
        }
        if (invert) {
            ll inv_n = qpow(n, MOD - 2, MOD);
            for (ll& x : a) x = x * inv_n % MOD;
        }
    }
};

// 多项式求逆：求 a 的逆元前 n 项（牛顿迭代倍增）。
// 固定使用 NTT<998244353, 3>，mod 应传 998244353；a[0] 必须非零。
inline std::vector<ll> poly_inv(const std::vector<ll>& a, int n, ll mod) {
    if (n <= 0 || a.empty()) return {};
    ll a0 = a[0] % mod;
    if (a0 < 0) a0 += mod;
    if (a0 == 0) return {};  // a[0] 不可逆
    std::vector<ll> res(1, qpow(a0, mod - 2, mod));
    int m = 1;
    while (m < n) {
        int m2 = m * 2;
        // fa = a 的前 m2 项。
        std::vector<ll> fa(m2, 0);
        for (int i = 0; i < (int)a.size() && i < m2; ++i) {
            fa[i] = a[i] % mod;
            if (fa[i] < 0) fa[i] += mod;
        }
        // fr = res（当前 m 项）。
        std::vector<ll> fr(m2, 0);
        for (int i = 0; i < m && i < (int)res.size(); ++i) fr[i] = res[i];
        // c = a * res mod x^{m2}。
        std::vector<ll> c = NTT<998244353, 3>::convolution(fa, fr);
        c.resize(m2);
        // t = 2 - c。
        for (int i = 0; i < m2; ++i) {
            ll base = (i == 0 ? 2 : 0);
            c[i] = (base - c[i]) % mod;
            if (c[i] < 0) c[i] += mod;
        }
        // res_new = res * t mod x^{m2}。
        std::vector<ll> fr2(m2, 0);
        for (int i = 0; i < m; ++i) fr2[i] = res[i];
        std::vector<ll> d = NTT<998244353, 3>::convolution(fr2, c);
        d.resize(m2);
        res.assign(d.begin(), d.begin() + m2);
        m = m2;
    }
    res.resize(n);
    return res;
}

}  // namespace algo
