#pragma once
// ============================================================================
// algo/math/crt.hpp —— 中国剩余定理（CRT / 扩展 CRT）
// ----------------------------------------------------------------------------
// 功能：
//   - crt(a, m)：解 x ≡ a[i] (mod m[i])，要求所有 m[i] 两两互质，返回最小非负解。
//   - excrt(a, m)：扩展 CRT，m[i] 可不互质，两两合并；无解返回 -1。
// 时间复杂度：O(n log M)（每次合并做一次 exgcd）。
// 空间复杂度：O(1)。
// 使用示例：
//   #include "math/crt.hpp"
//   ll x = algo::crt({2, 3, 1}, {3, 5, 7});     // x ≡ 2(mod3),3(mod5),1(mod7)
//   ll y = algo::excrt({2, 3}, {4, 6});         // 无解 → -1
// 注意事项：
//   1. 所有 m[i] 必须为正；crt 要求两两互质，excrt 无此要求。
//   2. crt 中 M = ∏m[i] 必须在 long long 范围内；excrt 中每一步 lcm 也须在 ll 内。
//   3. a[i] 允许为负数（内部先规约到 [0, m[i])）。
// ============================================================================
#include "algo/common.hpp"
#include "math/fastpow.hpp"
#include "math/gcd.hpp"

#include <vector>

namespace algo {

// 中国剩余定理（m 两两互质）：返回最小非负解 x（0 <= x < M）。
inline ll crt(const std::vector<ll>& a, const std::vector<ll>& m) {
    if (a.empty()) return 0;
    ll M = 1;
    for (ll mi : m) M *= mi;  // 注意：∏m 须在 long long 范围内
    ll ans = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        ll ai = a[i] % m[i];
        if (ai < 0) ai += m[i];
        ll Mi = M / m[i];
        ll x, y;
        exgcd(Mi, m[i], x, y);
        ll inv = x % m[i];
        if (inv < 0) inv += m[i];  // Mi 关于 m[i] 的逆元
        // term = ai * Mi * inv mod M，用 qmul 防溢出。
        ll term = qmul(qmul(ai, Mi, M), inv, M);
        ans = (ll)(((ull)ans + (ull)term) % (ull)M);
    }
    return ans;
}

// 扩展中国剩余定理（m 可不互质）：两两合并；无解返回 -1。
inline ll excrt(const std::vector<ll>& a, const std::vector<ll>& m) {
    if (a.empty()) return 0;
    ll a1 = a[0] % m[0];
    if (a1 < 0) a1 += m[0];
    ll m1 = m[0];
    for (size_t i = 1; i < a.size(); ++i) {
        ll a2 = a[i] % m[i];
        if (a2 < 0) a2 += m[i];
        ll m2 = m[i];
        ll x, y;
        ll g = exgcd(m1, m2, x, y);  // m1*x + m2*y = g
        ll c = a2 - a1;
        if (c % g != 0) return -1;   // 无解
        ll m2g = m2 / g;
        ll xx = x % m2g;
        if (xx < 0) xx += m2g;
        ll cc = (c / g) % m2g;
        if (cc < 0) cc += m2g;
        ll t = qmul(xx, cc, m2g);    // t ≡ (c/g) * x (mod m2/g)
        ll m1_new = m1 / g * m2;     // 新的模数 = lcm(m1, m2)
        ll add = qmul(m1, t, m1_new);
        a1 = (ll)(((ull)a1 + (ull)add) % (ull)m1_new);
        m1 = m1_new;
    }
    return a1;
}

}  // namespace algo
