#pragma once
// ============================================================================
// algo/lis_lcs.hpp —— 最长上升子序列（LIS）/ 最长公共子序列（LCS）
// ----------------------------------------------------------------------------
// 功能：
//   - lis(a, strict)：求数组 a 的最长上升子序列长度（strict=true，严格递增）
//     或最长不下降子序列长度（strict=false）；
//   - lcs(a, b)：求两个字符串的最长公共子序列长度。
// 时间复杂度：
//   lis：O(n log n)；
//   lcs：O(n * m)。
// 空间复杂度：
//   lis：O(n)（仅维护 tails 数组，若要还原具体子序列需额外记录前驱）；
//   lcs：O(n * m)。
// 使用示例：
//   vector<int> a = {2, 1, 5, 3, 6, 4, 8, 9, 7};
//   int len1 = algo::lis(a);          // 严格上升：5（如 1,3,6,8,9）
//   int len2 = algo::lis(a, false);   // 不下降长度
//   int len3 = algo::lcs("abcde", "ace");  // 3（"ace"）
// 注意事项：
//   - lis 用"贪心 + 二分"：tails[i] 表示长度为 i+1 的子序列的最小末尾；
//     strict 用 lower_bound（找第一个 >= x），非 strict 用 upper_bound
//     （找第一个 > x），两者务必区分；
//   - lis 求的是"长度"，不是子序列本身；要还原需另存每个位置的前驱下标；
//   - lcs 的 O(nm) 空间可用滚动数组优化到 O(min(n,m))（见函数内注释）；
//   - 空序列：lis 返回 0，lcs 返回 0；
//   - lis 的元素需支持 < 比较；lcs 的字符需支持 == 比较。
// ============================================================================

#include <vector>
#include <string>
#include <algorithm>  // std::lower_bound / std::upper_bound / std::max

namespace algo {

// 最长上升（strict=true）或最长不下降（strict=false）子序列长度
template<typename T>
int lis(const std::vector<T>& a, bool strict = true) {
    std::vector<T> tails;  // tails[i] = 长度为 i+1 的子序列的最小末尾元素
    for (const T& x : a) {
        if (strict) {
            // 严格递增：找第一个 >= x 的位置并替换，保证尾部尽量小
            typename std::vector<T>::iterator it =
                std::lower_bound(tails.begin(), tails.end(), x);
            if (it == tails.end()) tails.push_back(x);
            else *it = x;
        } else {
            // 非递减：找第一个 > x 的位置并替换（允许相等元素并列）
            typename std::vector<T>::iterator it =
                std::upper_bound(tails.begin(), tails.end(), x);
            if (it == tails.end()) tails.push_back(x);
            else *it = x;
        }
    }
    return (int)tails.size();
}

// 最长公共子序列长度：O(nm) 时间、O(nm) 空间
int lcs(const std::string& a, const std::string& b) {
    int n = (int)a.size();
    int m = (int)b.size();
    // dp[i][j] = a 前 i 个字符与 b 前 j 个字符的最长公共子序列长度
    std::vector<std::vector<int> > dp(n + 1, std::vector<int>(m + 1, 0));
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (a[i - 1] == b[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }
    return dp[n][m];
    // 空间优化提示：dp[i][*] 仅依赖 dp[i-1][*]，可用两行滚动数组
    //   vector<vector<int>> dp(2, vector<int>(m + 1, 0));
    //   令 cur = i & 1, pre = cur ^ 1，把空间降到 O(m)；
    //   若再保证 |a| <= |b|，可取 O(min(n, m))。
}

}  // namespace algo
