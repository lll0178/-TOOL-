#pragma once
// ============================================================================
// algo/string/rolling_hash.hpp —— 字符串哈希（自然溢出 / 双哈希）
// ----------------------------------------------------------------------------
// 功能：
//   1. class StringHash：自然溢出单哈希。用 unsigned long long 存储哈希值，
//      所有乘法/加法自动对 2^64 取模（自然溢出），固定 base = 131。预处理
//      前缀哈希 h 与 base 幂 p，get(l, r) 返回子串 s[l..r]（闭区间）的哈希。
//   2. class StringHashDouble：双哈希。第一维自然溢出（base 131），第二维
//      模 1e9+7（base 13331），get2(l, r) 返回 pair<ull, ull>。
// 时间复杂度：预处理 O(n)；单次 get / get2 O(1)。
// 空间复杂度：O(n)（前缀哈希 + base 幂）。
// 使用示例：
//   StringHash h("ababa");
//   ull x = h.get(0, 2);       // 子串 "aba" 的哈希
//   StringHashDouble hd("ababa");
//   pair<ull, ull> y = hd.get2(1, 3); // 子串 "bab" 的双哈希
// 注意事项：
//   - 乘法 a * b 在 unsigned long long 下自然溢出（等价于 mod 2^64），
//     无需 __int128。
//   - get(l, r) 要求 0 <= l <= r < n；越界是未定义行为，调用方保证。
//   - 单哈希存在碰撞风险（尤其字符串很长 / 数据量很大时），竞赛中建议使用
//     双哈希；若允许，也可用随机 base 进一步降低被针对的概率。
//   - 第二维模数取质数 1e9+7，两个不同 base 降低两维同时碰撞的概率。
// ============================================================================
#include <string>
#include <utility>
#include <vector>
#include "algo/common.hpp"

namespace algo {

// 自然溢出单哈希（ull 自动对 2^64 取模）
class StringHash {
public:
    static const ull BASE = 131ULL;  // 可改为其它奇数/质数 base

    explicit StringHash(const std::string& s) {
        int n = (int)s.size();
        h_.assign(n + 1, 0);
        p_.assign(n + 1, 1);
        for (int i = 0; i < n; i++) {
            ull c = (ull)(unsigned char)s[i];  // 转为无符号，避免符号扩展
            h_[i + 1] = h_[i] * BASE + c;      // 自然溢出
            p_[i + 1] = p_[i] * BASE;
        }
    }

    // 返回子串 s[l..r]（闭区间，0-based）的哈希值
    ull get(int l, int r) const {
        // h_[r+1] - h_[l] * p_[r-l+1]：减法在无符号下同样自然“回绕”，
        // 整体结果仍等价于模 2^64 的正确哈希。
        return h_[r + 1] - h_[l] * p_[r - l + 1];
    }

private:
    std::vector<ull> h_;  // 前缀哈希，h_[i] = s[0..i-1] 的哈希
    std::vector<ull> p_;  // p_[i] = BASE^i
};

// 双哈希：第一维自然溢出，第二维模 1e9+7
class StringHashDouble {
public:
    static const ull BASE1 = 131ULL;       // 自然溢出（mod 2^64）
    static const ull BASE2 = 13331ULL;     // 第二维 base
    static const ull MOD2 = 1000000007ULL; // 第二维模数（质数）

    explicit StringHashDouble(const std::string& s) {
        int n = (int)s.size();
        h1_.assign(n + 1, 0);
        p1_.assign(n + 1, 1);
        h2_.assign(n + 1, 0);
        p2_.assign(n + 1, 1);
        for (int i = 0; i < n; i++) {
            ull c = (ull)(unsigned char)s[i];
            h1_[i + 1] = h1_[i] * BASE1 + c;              // 自然溢出
            p1_[i + 1] = p1_[i] * BASE1;
            h2_[i + 1] = (h2_[i] * BASE2 + c) % MOD2;     // 模 1e9+7
            p2_[i + 1] = p2_[i] * BASE2 % MOD2;
        }
    }

    // 返回子串 s[l..r]（闭区间，0-based）的双哈希值
    std::pair<ull, ull> get2(int l, int r) const {
        ull a = h1_[r + 1] - h1_[l] * p1_[r - l + 1];  // 自然溢出
        // 模意义减法：先加 MOD2 保证非负
        ull b = (h2_[r + 1] + MOD2 - h2_[l] * p2_[r - l + 1] % MOD2) % MOD2;
        return std::make_pair(a, b);
    }

private:
    std::vector<ull> h1_, p1_;  // 第一维：自然溢出
    std::vector<ull> h2_, p2_;  // 第二维：模 1e9+7
};

}  // namespace algo
