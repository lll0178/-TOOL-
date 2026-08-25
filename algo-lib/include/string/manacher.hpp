#pragma once
// ============================================================================
// algo/string/manacher.hpp —— Manacher 最长回文
// ----------------------------------------------------------------------------
// 功能：
//   1. manacher(s)：用插入分隔符 '#' 的方式统一奇偶回文，返回处理串的半径
//      数组 d。处理串 t 的长度为 2n+1（t[偶数]='#'，t[奇数]=s[i/2]）。
//      d[i] 表示以 t[i] 为中心的最长回文半径（含中心本身），即 t 中该回文
//      长度为 2*d[i]-1，对应原串回文长度为 d[i]-1。
//      转换公式：原串最长回文长度 = max(d) - 1。
//   2. longest_palindrome(s)：利用 d 数组直接还原出最长回文子串（注意奇偶）。
// 时间复杂度：O(n)，n = |s|（每个字符最多被比较常数次，线性）。
// 空间复杂度：O(n)（处理串 t 与半径数组 d）。
// 使用示例：
//   vector<int> d = algo::manacher("abba");  // 处理串 "#a#b#b#a#"
//   // d = {1,1,2,1,2,5,2,1,2,1,1}，max(d)=5，最长回文长度 = 4
//   string p = algo::longest_palindrome("babad"); // "bab"（或 "aba"，取其一）
// 注意事项：
//   - d[i] 含中心：回文在 t 中长度为 2*d[i]-1，在原串中为 d[i]-1。
//   - 原串最长回文子串的起点 = (i - d[i] + 1) / 2，长度 = d[i] - 1。
//   - 空串返回空串；单字符返回该字符。
// ============================================================================
#include <algorithm>
#include <string>
#include <vector>

namespace algo {

// Manacher：返回处理串（插入 '#'）的半径数组 d，d[i] 含中心，长度为 2n+1
inline std::vector<int> manacher(const std::string& s) {
    int n = (int)s.size();
    // 构造处理串 t：在每个字符前后插入 '#'
    std::string t;
    t.reserve(2 * n + 1);
    t.push_back('#');
    for (char ch : s) {
        t.push_back(ch);
        t.push_back('#');
    }
    int m = (int)t.size();  // 2n + 1
    std::vector<int> d(m, 0);
    int l = 0, r = -1;  // 当前已知的最右回文区间 [l, r]
    for (int i = 0; i < m; i++) {
        int k = 1;  // 至少包含中心自身
        if (i <= r) {
            // 利用关于中心 (l+r)/2 的对称点 i' = l + r - i 加速
            k = std::min(d[l + r - i], r - i + 1);
        }
        // 朴素扩展（总扩展量摊还 O(n)）
        while (i - k >= 0 && i + k < m && t[i - k] == t[i + k]) {
            k++;
        }
        d[i] = k;
        // 更新最右回文区间
        if (i + k - 1 > r) {
            l = i - k + 1;
            r = i + k - 1;
        }
    }
    return d;
}

// 返回最长回文子串（利用半径数组还原，自动区分奇偶）
inline std::string longest_palindrome(const std::string& s) {
    if (s.empty()) return "";
    std::vector<int> d = manacher(s);
    int best = 0;
    for (int i = 1; i < (int)d.size(); i++) {
        if (d[i] > d[best]) best = i;  // 取半径最大者（多个时取最靠左）
    }
    int len = d[best] - 1;                // 原串回文长度
    int start = (best - d[best] + 1) / 2; // 还原到原串起点
    return s.substr(start, len);
}

}  // namespace algo
