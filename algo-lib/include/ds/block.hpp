#pragma once
// ============================================================================
// algo/ds/block.hpp —— 分块（Sqrt Decomposition）
// ----------------------------------------------------------------------------
// 功能：对静态序列做"整块懒标记 + 散块暴力"的分块，支持：
//   - range_add(l, r, v)    区间加；
//   - range_sum(l, r)       区间求和；
//   - count_less(l, r, x)   统计区间内严格小于 x 的元素个数。
// 时间复杂度：range_add / range_sum O(sqrt(n))；
//            count_less O(sqrt(n) log n)（整块在排序副本上二分）。
// 空间复杂度：O(n)（每块维护一份排序副本）。
// 使用示例：
//   SqrtDecomp sd(std::vector<ll>{5,1,4,1,3,9});
//   sd.range_add(1, 4, 2);          // {7,3,6,3,3,9}
//   ll s = sd.range_sum(2, 5);      // 15
//   int c = sd.count_less(1, 6, 5); // 2（值为 3、3）
// 注意事项：
//   - 所有下标均为 1-based。
//   - 块大小取 max(1, (int)sqrt(n))。
//   - 散块修改后重建该块排序副本；整块只改懒标记，不重建。
// ============================================================================

#include <algorithm>  // std::sort / std::lower_bound / std::max
#include <cmath>      // std::sqrt
#include <vector>

#include "algo/common.hpp"

namespace algo {

class SqrtDecomp {
public:
    // 由数组 a（下标 0 起）建块
    explicit SqrtDecomp(const std::vector<ll>& a) {
        n_ = static_cast<int>(a.size());
        block_ = std::max(1, static_cast<int>(std::sqrt(static_cast<double>(n_))));
        num_ = (n_ + block_ - 1) / block_;

        a_.assign(n_ + 1, 0);  // 1-based 存储
        for (int i = 0; i < n_; ++i) a_[i + 1] = a[i];

        add_.assign(num_, 0);
        bsum_.assign(num_, 0);
        sorted_.assign(num_, std::vector<ll>());
        for (int b = 0; b < num_; ++b) rebuild(b);
    }

    // 区间加：a[l..r] 各 += v（1-based）
    void range_add(int l, int r, ll v) {
        if (n_ == 0) return;
        int bl = block_id(l);
        int br = block_id(r);
        if (bl == br) {
            for (int i = l; i <= r; ++i) a_[i] += v;
            rebuild(bl);
            return;
        }
        // 左散块
        for (int i = l; i <= block_r(bl); ++i) a_[i] += v;
        rebuild(bl);
        // 右散块
        for (int i = block_l(br); i <= r; ++i) a_[i] += v;
        rebuild(br);
        // 整块只打懒标记
        for (int b = bl + 1; b <= br - 1; ++b) add_[b] += v;
    }

    // 区间求和（1-based）
    ll range_sum(int l, int r) {
        if (n_ == 0) return 0;
        int bl = block_id(l);
        int br = block_id(r);
        ll res = 0;
        if (bl == br) {
            for (int i = l; i <= r; ++i) res += a_[i] + add_[bl];
            return res;
        }
        for (int i = l; i <= block_r(bl); ++i) res += a_[i] + add_[bl];
        for (int i = block_l(br); i <= r; ++i) res += a_[i] + add_[br];
        for (int b = bl + 1; b <= br - 1; ++b) {
            int len = block_r(b) - block_l(b) + 1;
            res += bsum_[b] + add_[b] * len;
        }
        return res;
    }

    // 统计区间内严格小于 x 的元素个数（1-based）
    int count_less(int l, int r, ll x) {
        if (n_ == 0) return 0;
        int bl = block_id(l);
        int br = block_id(r);
        int res = 0;
        if (bl == br) {
            for (int i = l; i <= r; ++i) {
                if (a_[i] + add_[bl] < x) ++res;
            }
            return res;
        }
        for (int i = l; i <= block_r(bl); ++i) {
            if (a_[i] + add_[bl] < x) ++res;
        }
        for (int i = block_l(br); i <= r; ++i) {
            if (a_[i] + add_[br] < x) ++res;
        }
        // 整块二分：a_[i] + add_[b] < x  ⇔  a_[i] < x - add_[b]
        for (int b = bl + 1; b <= br - 1; ++b) {
            ll target = x - add_[b];
            res += static_cast<int>(std::lower_bound(sorted_[b].begin(), sorted_[b].end(),
                                                     target) -
                                    sorted_[b].begin());
        }
        return res;
    }

private:
    int n_;
    int block_;  // 块大小
    int num_;    // 块数

    std::vector<ll> a_;                      // 原始数组（1-based）
    std::vector<ll> add_;                    // 每块懒标记
    std::vector<ll> bsum_;                   // 每块元素和（不含懒标记）
    std::vector<std::vector<ll>> sorted_;    // 每块排序副本（不含懒标记）

    int block_id(int i) const { return (i - 1) / block_; }
    int block_l(int b) const { return b * block_ + 1; }
    int block_r(int b) const { return std::min(n_, (b + 1) * block_); }

    // 重建第 b 块的排序副本与块内和（散块修改后调用）
    void rebuild(int b) {
        int l = block_l(b);
        int r = block_r(b);
        sorted_[b].clear();
        sorted_[b].reserve(r - l + 1);
        bsum_[b] = 0;
        for (int i = l; i <= r; ++i) {
            sorted_[b].push_back(a_[i]);
            bsum_[b] += a_[i];
        }
        std::sort(sorted_[b].begin(), sorted_[b].end());
    }
};

}  // namespace algo
