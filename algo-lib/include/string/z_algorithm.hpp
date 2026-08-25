#pragma once
// ============================================================================
// algo/string/z_algorithm.hpp —— Z 函数（扩展 KMP）与 Z 匹配
// ----------------------------------------------------------------------------
// 功能：
//   1. z_function(s)：对字符串 s 求 Z 数组。z[i] = LCP(s, s[i..])，即 s 与
//      以 i 开头的后缀的最长公共前缀长度。约定 z[0] = 0（前缀与自身比较无意义，
//      故置 0）。标准 O(n) 线性实现，维护当前最右匹配区间 [l, r]。
//   2. z_match(text, pat)：构造 s = pat + '#' + text 的 Z 数组，返回 pat 在
//      text 中所有出现位置的起始下标（0-based，可重叠）。
// 时间复杂度：两者均为 O(|s|)（z_match 为 O(|text| + |pat|)）。
// 空间复杂度：O(n)（Z 数组）。
// 使用示例：
//   vector<int> z = algo::z_function("aabxaabx"); // {0,1,0,0,4,1,0,0}
//   vector<int> pos = algo::z_match("ababa", "aba"); // {0, 2}
// 注意事项：
//   - z[0] 恒为 0（本实现从 i=1 开始计算，天然满足）。
//   - z_match 使用 '#' 作为分隔符；由于匹配判断只看 z[i] >= |pat|，即使
//     text 中出现 '#' 也不会破坏正确性（前 |pat| 个字符的比较只涉及 pat 与
//     text，不会越过分隔符）。
//   - 空模式串约定返回空结果。
// ============================================================================
#include <algorithm>
#include <string>
#include <vector>

namespace algo {

// Z 函数：z[i] = LCP(s, s[i..])，z[0] = 0；线性时间，维护最右匹配区间 [l, r)
inline std::vector<int> z_function(const std::string& s) {
    int n = (int)s.size();
    std::vector<int> z(n, 0);
    if (n == 0) return z;
    int l = 0, r = 0;  // 当前最右匹配段 [l, r)（右开区间），z[l..r-1] 已知
    for (int i = 1; i < n; i++) {
        if (i < r) {
            // 利用对称性：先取已有结果作为下界，但不能超过当前区间右边界
            z[i] = std::min(r - i, z[i - l]);
        }
        // 朴素扩展（总的扩展量是 O(n)，摊还线性）
        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            z[i]++;
        }
        // 更新最右匹配区间
        if (i + z[i] > r) {
            l = i;
            r = i + z[i];
        }
    }
    return z;  // z[0] 保持为 0
}

// Z 匹配：返回 pat 在 text 中所有出现位置的起始下标（0-based，可重叠）
inline std::vector<int> z_match(const std::string& text, const std::string& pat) {
    std::vector<int> res;
    int m = (int)pat.size();
    if (m == 0) return res;  // 空模式串：约定返回空结果
    std::string s = pat + '#' + text;  // 用分隔符 '#' 隔开模式串与文本
    std::vector<int> z = z_function(s);
    // 文本段从下标 m+1 开始；z[i] >= m 表示在 text 的 (i - m - 1) 处匹配
    for (int i = m + 1; i < (int)z.size(); i++) {
        if (z[i] >= m) {
            res.push_back(i - m - 1);
        }
    }
    return res;
}

}  // namespace algo
