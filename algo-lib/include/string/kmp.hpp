#pragma once
// ============================================================================
// algo/string/kmp.hpp —— KMP 字符串匹配与最小循环节
// ----------------------------------------------------------------------------
// 功能：
//   1. kmp_build_next(s)：构建前缀函数（next 数组）。next[i] 表示前缀
//      s[0..i] 的“最长真公共前后缀”长度（真前后缀 = 不包含自身的前缀/后缀）。
//      这是 KMP 的标准实现，next[0] 恒为 0。
//   2. kmp_match(text, pat)：返回模式串 pat 在文本 text 中所有出现位置的
//      起始下标（0-based，允许重叠）。
//   3. min_cycle(s)：求字符串 s 的最小循环节长度。
// 时间复杂度：
//   - kmp_build_next：O(n)，n = |s|。
//   - kmp_match：O(|text| + |pat|)。
//   - min_cycle：O(n)。
// 空间复杂度：O(n)（next 数组）。
// 使用示例：
//   string text = "abababa", pat = "aba";
//   vector<int> pos = algo::kmp_match(text, pat);  // {0, 2, 4}（可重叠）
//   vector<int> nxt = algo::kmp_build_next("aabaaab"); // {0,1,0,1,2,2,3}
//   int cyc = algo::min_cycle("abcabc");               // 3
// 注意事项：
//   - next[i] 是“真”前后缀长度，不含自身，因此 next[0] = 0。
//   - 空模式串约定不返回任何匹配位置（调用方需自行处理）。
//   - min_cycle 对空串返回 0；令 len = n - next[n-1]，当且仅当 n % len == 0
//     时字符串由完整循环节组成（返回 len），否则返回 n（整串自身）。
// ============================================================================
#include <string>
#include <vector>

namespace algo {

// 构建 KMP 前缀函数：next[i] = 前缀 s[0..i] 的最长真公共前后缀长度
inline std::vector<int> kmp_build_next(const std::string& s) {
    int n = (int)s.size();
    std::vector<int> next(n, 0);
    for (int i = 1; i < n; i++) {
        int j = next[i - 1];  // 先尝试沿用前一个前缀的 border 长度
        while (j > 0 && s[i] != s[j]) {
            j = next[j - 1];  // 失配则回退到更短的 border
        }
        if (s[i] == s[j]) j++;
        next[i] = j;
    }
    return next;
}

// KMP 匹配：返回 pat 在 text 中所有出现位置的起始下标（0-based，可重叠）
inline std::vector<int> kmp_match(const std::string& text, const std::string& pat) {
    std::vector<int> res;
    int n = (int)text.size();
    int m = (int)pat.size();
    if (m == 0) return res;  // 空模式串：约定不返回任何匹配
    std::vector<int> next = kmp_build_next(pat);
    int j = 0;  // 当前已匹配的 pat 前缀长度
    for (int i = 0; i < n; i++) {
        while (j > 0 && text[i] != pat[j]) {
            j = next[j - 1];  // 失配：利用 next 回退，不重复比较
        }
        if (text[i] == pat[j]) j++;
        if (j == m) {
            res.push_back(i - m + 1);  // 记录本次匹配的起始下标
            j = next[j - 1];           // 回退以支持重叠匹配
        }
    }
    return res;
}

// 最小循环节长度：len = n - next[n-1]；n % len == 0 时返回 len，否则返回 n
inline int min_cycle(const std::string& s) {
    int n = (int)s.size();
    if (n == 0) return 0;
    std::vector<int> next = kmp_build_next(s);
    int len = n - next[n - 1];
    // 只有当 n 能被 len 整除时，s 才由若干完整循环节组成
    return (n % len == 0) ? len : n;
}

}  // namespace algo
