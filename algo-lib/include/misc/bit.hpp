#pragma once
// ============================================================================
// misc/bit.hpp —— 位运算工具
// ----------------------------------------------------------------------------
// 功能：lowbit、popcount（1 的个数）、msb_index（最高位下标）、子集枚举、
//       Gosper's hack（下一个同 popcount 的更大数）。
// 时间复杂度：lowbit/next_combination O(1)；popcount O(1)（SWAR）或内置；
//             msb_index O(64)（或内置 O(1)）；enumerate_subsets O(2^k)。
// 空间复杂度：enumerate_subsets O(2^k)；其余 O(1)。
// 使用示例：
//   #include "misc/bit.hpp"
//   int lb = algo::lowbit(12);              // 4（最低位 1）
//   int pc = algo::popcount(12LL);          // 2
//   int msb = algo::msb_index(12LL);        // 3
//   std::vector<int> subs = algo::enumerate_subsets(0b101); // {5,4,1,0}
//   long long nx = algo::next_combination(0b1011); // 下一组合同位数 1 的数
// 注意事项：
//   - popcount 在 __GNUC__（含 Clang）下用 __builtin_popcountll，否则手写 SWAR，
//     双端均可编译。
//   - msb_index：x = 0 或负数返回 -1（只对非负整数有意义）。
//   - enumerate_subsets 返回 mask 的全部子集（含 0），按从大到小排列。
//   - next_combination：x 应为非负位掩码；x = 0 返回 0；输入应为可用
//     long long 表示（建议 < 2^63）的位掩码，避免溢出到符号位。
// ============================================================================
#include "algo/common.hpp"

#include <vector>

namespace algo {

// lowbit：取 x 最低位的 1（x & (-x) 的无符号安全写法，避免 INT_MIN 取负溢出）
inline int lowbit(int x) {
    return static_cast<int>(static_cast<unsigned int>(x) &
                            (0u - static_cast<unsigned int>(x)));
}

// lowbit_ll：long long 版本
inline ll lowbit_ll(ll x) {
    return static_cast<ll>(static_cast<ull>(x) & (0ULL - static_cast<ull>(x)));
}

// popcount：x 的二进制中 1 的个数
inline int popcount(ll x) {
#if defined(__GNUC__) || defined(__clang__)
    // GCC / Clang：使用内置指令
    return __builtin_popcountll(static_cast<unsigned long long>(x));
#else
    // MSVC 等：手写 SWAR（SIMD within a register）算法
    unsigned long long v = static_cast<unsigned long long>(x);
    v = v - ((v >> 1) & 0x5555555555555555ULL);
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    v = (v * 0x0101010101010101ULL) >> 56;
    return static_cast<int>(v);
#endif
}

// msb_index：最高位 1 的下标（0-based）；x = 0 或负数返回 -1
inline int msb_index(ll x) {
    if (x <= 0) return -1;
#if defined(__GNUC__) || defined(__clang__)
    return 63 - __builtin_clzll(static_cast<unsigned long long>(x));
#else
    // 手写：逐位右移计数（最多 63 次，对 long long 足够快）
    unsigned long long v = static_cast<unsigned long long>(x);
    int idx = 0;
    while (v >>= 1) ++idx;
    return idx;
#endif
}

// enumerate_subsets：返回 mask 的所有子集（含 0），按从大到小排列
inline std::vector<int> enumerate_subsets(int mask) {
    std::vector<int> res;
    for (int sub = mask;; sub = (sub - 1) & mask) {
        res.push_back(sub);
        if (sub == 0) break;
    }
    return res;
}

// next_combination：Gosper's hack。
// 返回比 x 大且二进制中 1 的个数与 x 相同的下一个数；x = 0 返回 0。
inline ll next_combination(ll x) {
    if (x == 0) return 0;
    unsigned long long c = static_cast<unsigned long long>(x);
    unsigned long long r = c & (0ULL - c);     // 最低位的 1
    unsigned long long s = c + r;              // 把最低位 1 及其后的 0 进位
    unsigned long long res = s | (((c ^ s) >> 2) / r);
    return static_cast<ll>(res);
}

}  // namespace algo
