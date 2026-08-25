#pragma once
// ============================================================================
// algo/ds/sparse_table.hpp —— ST 表（Sparse Table）
// ----------------------------------------------------------------------------
// 功能：离线 RMQ 类查询。给定序列，O(1) 回答区间"可重复贡献"型询问。
//       要求 Merge 幂等：merge(x, x) == x，典型如 max / min / gcd。
// 时间复杂度：预处理 O(n log n)，单次查询 O(1)。
// 空间复杂度：O(n log n)。
// 使用示例：
//   std::vector<ll> a = {1, 3, 2, 9, 7};
//   // 区间最大值
//   SparseTable<ll> st_max(a, [](ll x, ll y) { return std::max(x, y); });
//   ll mx = st_max.query(1, 3);   // a[1..3] 最大值 = 9（0-based 下标）
//   // 区间最小值
//   SparseTable<ll> st_min(a, [](ll x, ll y) { return std::min(x, y); });
//   // 区间 gcd（需自行实现 gcd，例如手写辗转相除；勿用 std::gcd 以保证 MSVC2017 兼容）
//   SparseTable<int> st_gcd(a, [](int x, int y) {
//       while (y) { int t = x % y; x = y; y = t; }
//       return x;
//   });
// 注意事项：
//   - 下标 l、r 为 0-based 闭区间（与输入 vector 下标一致）。
//   - Merge 必须满足幂等性（两段覆盖会重复取公共部分，见 query 注释）。
//   - 不支持带修改操作；需要动态修改请改用线段树。
// ============================================================================

#include <functional>  // std::function（作为 Merge 的默认类型）
#include <vector>

namespace algo {

// Merge 给出默认类型 std::function<T(T,T)>，使得 SparseTable<ll> st(a, lambda)
// 这类"只指定 T、由 lambda 自动适配"的写法在 C++17 下即可编译（无需部分类模板实参推导）。
template <typename T, typename Merge = std::function<T(T, T)>>
class SparseTable {
public:
    // 以序列 a 建表，merge 为二元合并运算（需幂等）
    SparseTable(const std::vector<T>& a, Merge merge)
        : n_(static_cast<int>(a.size())), merge_(merge) {
        // 计算 logn = floor(log2(n)) + 1（n>=1 时），避免使用 __builtin_clz 等扩展
        logn_ = 0;
        while ((1 << logn_) <= n_) ++logn_;

        st_.assign(logn_, std::vector<T>());
        if (logn_ >= 1) st_[0] = a;
        for (int k = 1; k < logn_; ++k) {
            int len = n_ - (1 << k) + 1;  // 本层区间个数
            st_[k].resize(len);
            for (int i = 0; i < len; ++i) {
                st_[k][i] = merge_(st_[k - 1][i], st_[k - 1][i + (1 << (k - 1))]);
            }
        }

        // 预计算 log2 表，使查询中 log2(len) 为 O(1)
        lg_.assign(n_ + 1, 0);
        for (int i = 2; i <= n_; ++i) lg_[i] = lg_[i >> 1] + 1;
    }

    // 查询 [l, r]（0-based，闭区间）的合并结果
    T query(int l, int r) const {
        int len = r - l + 1;
        int k = lg_[len];
        // 用两段 [l, l+2^k-1] 与 [r-2^k+1, r] 覆盖整个区间；
        // 两段可能有交叠，因此要求 Merge 幂等（如 max/min/gcd）。
        return merge_(st_[k][l], st_[k][r - (1 << k) + 1]);
    }

private:
    int n_;
    int logn_;
    Merge merge_;
    std::vector<std::vector<T>> st_;
    std::vector<int> lg_;
};

}  // namespace algo
