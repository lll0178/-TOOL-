#pragma once
// ============================================================================
// algo/math/bsgs.hpp —— BSGS 离散对数
// ----------------------------------------------------------------------------
// 功能：解 a^x ≡ b (mod p)，p 为质数；返回最小非负整数 x，无解返回 -1。
// 时间复杂度：O(sqrt(p))。
// 空间复杂度：O(sqrt(p))（哈希表存 baby 步）。
// 使用示例：
//   #include "math/bsgs.hpp"
//   ll x = algo::bsgs(2, 8, 13);   // 2^3 ≡ 8 (mod 13) → 3
// 注意事项：
//   1. p 必须为质数（保证 a 与 p 互质时 a 可逆）。
//   2. 边界：b ≡ 1 时直接返回 x = 0；a ≡ 0 (mod p) 时：0^0=1 已处理，
//      之后 0^x = 0（x>=1），故 b==0 返回 1，否则无解。
//   3. 需要 O(sqrt(p)) 内存，p 过大（如 > 1e7）时哈希表会过大，慎用。
// ============================================================================
#include "algo/common.hpp"
#include "math/fastpow.hpp"

#include <cmath>
#include <unordered_map>

namespace algo {

// BSGS：解 a^x ≡ b (mod p)，p 为质数。
inline ll bsgs(ll a, ll b, ll mod) {
    a %= mod;
    if (a < 0) a += mod;
    b %= mod;
    if (b < 0) b += mod;
    if (b == 1 % mod) return 0;  // x = 0（含 mod == 1 退化情形）
    if (a == 0) return (b == 0) ? 1 : -1;  // 0^x 情形

    ll m = (ll)std::sqrt((long double)mod);
    while (m * m < mod) ++m;  // m = ceil(sqrt(p))

    // baby 步：baby[j] = b * a^j，记录"最大的 j"（使 x = i*m - j 取最小）。
    std::unordered_map<ll, ll> baby;
    ll cur = b;
    for (ll j = 0; j < m; ++j) {
        baby[cur] = j;  // 覆盖保留最大 j
        cur = qmul(cur, a, mod);
    }

    // giant 步：枚举 (a^m)^i，i = 1..m，匹配 baby 表。
    ll step = qpow(a, m, mod);
    ll acc = step;
    for (ll i = 1; i <= m; ++i) {
        auto it = baby.find(acc);
        if (it != baby.end()) {
            ll x = i * m - it->second;
            if (x >= 0) return x;  // i 递增、区间不相交，首个即最小解
        }
        acc = qmul(acc, step, mod);
    }
    return -1;
}

}  // namespace algo
