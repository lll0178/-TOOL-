#pragma once
// ============================================================================
// algo/string/suffix_array.hpp —— 后缀数组（倍增 + 计数排序）
// ----------------------------------------------------------------------------
// 功能：
//   class SuffixArray：
//   - 构造函数 SuffixArray(s)：用“倍增 + 计数排序”构建后缀数组，O(n log n)。
//   - sa()：后缀数组，sa[i] 为字典序第 i 小的后缀的起始下标。
//   - rk()：名次数组，rk[i] 为后缀 i 的字典序名次（0-based）。
//   - height()：height[i] = LCP(sa[i], sa[i-1])（i >= 1），height[0] = 0。
//   - lcp(i, j)：O(1) 返回后缀 i 与后缀 j 的最长公共前缀长度（内部用倍增
//     稀疏表 RMQ 求 height 区间最小值）。
//   - distinct_substrings()：本质不同子串个数 = n(n+1)/2 - Σ height[i]。
// 时间复杂度：构建 O(n log n)；lcp 查询 O(1)；distinct_substrings O(n)。
// 空间复杂度：O(n log n)（主要为稀疏表）。
// 使用示例：
//   string s = "banana";
//   SuffixArray sa(s);
//   sa.sa();                  // 后缀按字典序排列的下标
//   sa.lcp(1, 3);             // LCP("anana", "ana") = 3
//   sa.distinct_substrings(); // 本质不同子串个数
// 注意事项：
//   - 字符按 unsigned char（0..255）计数排序，支持任意单字节字符（含任意
//     char）；若只含小写字母，内部自动等价于更小字符集，无需特殊处理。
//   - 空串：sa/rk/height 均为空，distinct_substrings() = 0；lcp 请勿在空串
//     上调用。
//   - distinct_substrings 返回 long long，避免 n 较大时溢出。
// ============================================================================
#include <algorithm>
#include <string>
#include <vector>
#include "algo/common.hpp"

namespace algo {

class SuffixArray {
public:
    SuffixArray() : n_(0) {}

    explicit SuffixArray(const std::string& s) { build(s); }

    const std::vector<int>& sa() const { return sa_; }
    const std::vector<int>& rk() const { return rk_; }
    const std::vector<int>& height() const { return height_; }

    // 后缀 i 与后缀 j 的最长公共前缀长度，O(1)（RMQ 区间最小值）
    int lcp(int i, int j) const {
        if (i == j) return n_ - i;  // 同一后缀：返回其后缀剩余长度
        int a = rk_[i], b = rk_[j];
        if (a > b) std::swap(a, b);
        // LCP(sa[a], sa[b]) = min{ height[a+1 .. b] }
        int len = b - a;
        int k = rmq_log_[len];
        return std::min(rmq_st_[k][a + 1], rmq_st_[k][b - (1 << k) + 1]);
    }

    // 本质不同子串个数 = n(n+1)/2 - Σ height[i]
    ll distinct_substrings() const {
        ll n = n_;
        ll total = n * (n + 1) / 2;
        ll sum_h = 0;
        for (int i = 1; i < n_; i++) {
            sum_h += height_[i];
        }
        return total - sum_h;
    }

private:
    int n_;
    std::vector<int> sa_;   // 后缀数组
    std::vector<int> rk_;   // 名次数组
    std::vector<int> height_; // height[i] = LCP(sa[i], sa[i-1])，height[0]=0
    std::vector<int> rmq_log_;                    // 倍增 RMQ 的 log 表
    std::vector<std::vector<int>> rmq_st_;        // 倍增 RMQ 稀疏表（对 height）

    void build(const std::string& s) {
        n_ = (int)s.size();
        if (n_ == 0) {
            sa_.clear();
            rk_.clear();
            height_.clear();
            rmq_log_.clear();
            rmq_st_.clear();
            return;
        }
        const int sigma = 256;  // 字符集大小：按 unsigned char（0..255）计数
        sa_.resize(n_);
        rk_.resize(n_);
        std::vector<int> x(n_), y(n_);  // x: 当前 rank；y: 辅助数组
        std::vector<int> cnt(std::max(sigma, n_), 0);

        // 第一轮：按单个字符计数排序
        for (int i = 0; i < n_; i++) {
            cnt[x[i] = (unsigned char)s[i]]++;
        }
        for (int i = 1; i < sigma; i++) {
            cnt[i] += cnt[i - 1];
        }
        for (int i = n_ - 1; i >= 0; i--) {
            sa_[--cnt[x[i]]] = i;
        }

        // 倍增：每轮按 (前 2k 个字符) 的排名排序，k 翻倍
        for (int k = 1; k < n_; k <<= 1) {
            // 1) 按第二关键字排序：第二关键字 = 后缀 i+k 的 rank，越界视为 -1
            int num = 0;
            for (int i = n_ - k; i < n_; i++) {
                y[num++] = i;  // 第二关键字为空的后缀排最前
            }
            for (int i = 0; i < n_; i++) {
                if (sa_[i] >= k) y[num++] = sa_[i] - k;
            }
            // 2) 按第一关键字 x 计数排序
            for (int i = 0; i < n_; i++) {
                cnt[i] = 0;  // 第一关键字取值范围 [0, num) ⊆ [0, n_)
            }
            for (int i = 0; i < n_; i++) {
                cnt[x[y[i]]]++;
            }
            for (int i = 1; i < n_; i++) {
                cnt[i] += cnt[i - 1];
            }
            for (int i = n_ - 1; i >= 0; i--) {
                sa_[--cnt[x[y[i]]]] = y[i];
            }
            // 3) 用 y 暂存旧 rank，重算 x（注意第二关键字越界视为 -1）
            std::swap(x, y);
            x[sa_[0]] = 0;
            num = 1;
            for (int i = 1; i < n_; i++) {
                int a = sa_[i], b = sa_[i - 1];
                int ka = (a + k < n_) ? y[a + k] : -1;
                int kb = (b + k < n_) ? y[b + k] : -1;
                x[a] = (y[a] == y[b] && ka == kb) ? num - 1 : num++;
            }
            if (num >= n_) break;  // 名次已两两不同，可提前结束
        }
        rk_ = x;  // 最终名次数组

        // 构建 height 数组（利用 rk 的相邻关系，O(n) 摊还）
        height_.assign(n_, 0);
        int k = 0;  // 当前 LCP 长度
        for (int i = 0; i < n_; i++) {
            if (rk_[i] == 0) {
                k = 0;  // 字典序最小的后缀没有前驱
                continue;
            }
            int j = sa_[rk_[i] - 1];  // 字典序中的前一个后缀
            while (i + k < n_ && j + k < n_ && s[i + k] == s[j + k]) {
                k++;
            }
            height_[rk_[i]] = k;
            if (k) k--;  // 下次 LCP 至少为 k-1
        }

        build_rmq();
    }

    // 对 height 构建倍增稀疏表，用于 O(1) 区间最小值
    void build_rmq() {
        rmq_log_.assign(n_ + 1, 0);
        for (int i = 2; i <= n_; i++) {
            rmq_log_[i] = rmq_log_[i / 2] + 1;
        }
        int K = rmq_log_[n_] + 1;
        rmq_st_.assign(K, std::vector<int>(n_, 0));
        for (int i = 0; i < n_; i++) {
            rmq_st_[0][i] = height_[i];
        }
        for (int k = 1; k < K; k++) {
            for (int i = 0; i + (1 << k) <= n_; i++) {
                rmq_st_[k][i] = std::min(rmq_st_[k - 1][i],
                                         rmq_st_[k - 1][i + (1 << (k - 1))]);
            }
        }
    }
};

}  // namespace algo
