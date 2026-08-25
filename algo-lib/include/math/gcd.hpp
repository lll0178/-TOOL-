#pragma once
// ============================================================================
// algo/math/gcd.hpp —— 最大公约数 / 扩展欧几里得 / 线性不定方程
// ----------------------------------------------------------------------------
// 功能：
//   - gcd(a, b)：最大公约数，兼容负数，返回非负值。
//   - lcm(a, b)：最小公倍数。
//   - exgcd(a, b, x, y)：求 ax + by = gcd(a, b) 的一组特解。
//   - linear_equation(a, b, c, x, y)：解 ax + by = c，有解返回 true 并给特解。
// 时间复杂度：均为 O(log min(|a|,|b|))
// 空间复杂度：O(1)
// 使用示例：
//   #include "math/gcd.hpp"
//   ll x, y;
//   ll g = algo::exgcd(3, 5, x, y);          // 3x + 5y = 1
//   bool ok = algo::linear_equation(3, 5, 7, x, y); // 3x + 5y = 7
// 注意事项：
//   1. exgcd 假定 a、b 非负（linear_equation 内部会先处理符号），返回非负 gcd。
//   2. lcm 采用"先除后乘"避免中间溢出，但最终结果仍可能超出 long long，需调用者保证。
//   3. linear_equation 的通解：x = x0 + (b/g)*t, y = y0 - (a/g)*t（t 为任意整数）。
//      求最小非负 x：令 step = b/g（若 step<0 取相反数），x = (x0 % step + step) % step。
// ============================================================================
#include "algo/common.hpp"

namespace algo {

// 最大公约数：兼容负数，返回非负值。
inline ll gcd(ll a, ll b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b != 0) {
        ll t = a % b;
        a = b;
        b = t;
    }
    return a;
}

// 最小公倍数：a / gcd * b（先除后乘防溢出；结果本身仍可能越界）。
inline ll lcm(ll a, ll b) {
    if (a == 0 || b == 0) return 0;
    return a / gcd(a, b) * b;
}

// 扩展欧几里得：求 ax + by = gcd(a, b) 的一组特解，返回 gcd(a, b)。
// 约定 a、b 非负；linear_equation 会先规约符号。
inline ll exgcd(ll a, ll b, ll& x, ll& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    ll d = exgcd(b, a % b, y, x);
    y -= a / b * x;
    return d;
}

// 解线性不定方程 ax + by = c。
// 有解返回 true，并把一组特解写入 x、y；无解返回 false。
inline bool linear_equation(ll a, ll b, ll c, ll& x, ll& y) {
    // 退化情形：a = b = 0
    if (a == 0 && b == 0) {
        if (c == 0) {
            x = 0;
            y = 0;
            return true;
        }
        return false;
    }
    ll g = gcd(a, b);
    if (c % g != 0) return false;
    // 在非负系数上求解，再按原符号还原 x、y。
    ll sa = a < 0 ? -1 : 1;
    ll sb = b < 0 ? -1 : 1;
    ll x0, y0;
    exgcd(sa * a, sb * b, x0, y0);  // 此时 sa*a, sb*b 均非负
    x = sa * x0 * (c / g);
    y = sb * y0 * (c / g);
    return true;
}

}  // namespace algo
