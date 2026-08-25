#pragma once
// ============================================================================
// algo/ds/persistent_segment_tree.hpp —— 主席树（可持久化线段树）与可持久化数组
// ----------------------------------------------------------------------------
// 功能：
//   1. PersistentSegTree 静态区间第 k 小：给定数组，多次询问 a[l..r] 中第 k 小
//      的值（k 从 1 开始），返回"原值"（离散化后还原）。
//   2. PersistentArray  可持久化数组：支持历史版本的单点修改与查询。
// 时间复杂度：建树 O(n log n)；每次查询/修改 O(log n)。
// 空间复杂度：O(n log n)（动态开点，每次修改新增 O(log n) 个节点）。
// 使用示例：
//   PersistentSegTree pst;
//   pst.build(std::vector<ll>{3,1,4,1,5});
//   ll v = pst.kth(1, 5, 2);   // 1（整体第 2 小）
//
//   PersistentArray pa(5);
//   int r0 = pa.build(std::vector<ll>{1,2,3,4,5});
//   int r1 = pa.update(r0, 3, 99);
//   ll a3 = pa.query(r1, 3);   // 99
//   ll a3_old = pa.query(r0, 3); // 3（历史版本不受影响）
// 注意事项：
//   - kth 的下标 l、r 为 1-based（含两端），要求 1 <= k <= r-l+1。
//   - 节点池从下标 0 开始，0 号节点作为"空节点"（左右儿子均指向 0）。
//   - 值域经离散化压缩到 [1, m]，查询时用 sorted_vals 还原原值。
// ============================================================================

#include <algorithm>  // std::sort / std::unique / std::lower_bound
#include <vector>

#include "algo/common.hpp"

namespace algo {

// ============================================================================
// PersistentSegTree —— 静态区间第 k 小（主席树）
// ============================================================================
class PersistentSegTree {
public:
    PersistentSegTree() : m_(0) {}

    // 以数组 a 建树：root[i] 表示前缀 a[1..i] 的版本（i 为 1-based）
    void build(const std::vector<ll>& a) {
        int n = static_cast<int>(a.size());
        // 1. 离散化：sorted_vals 为去重排序后的值
        sorted_vals_ = a;
        std::sort(sorted_vals_.begin(), sorted_vals_.end());
        sorted_vals_.erase(std::unique(sorted_vals_.begin(), sorted_vals_.end()),
                           sorted_vals_.end());
        m_ = static_cast<int>(sorted_vals_.size());

        // 2. 节点池：0 号空节点
        pool_.clear();
        pool_.push_back(PstNode{0, 0, 0});

        roots_.assign(n + 1, 0);
        roots_[0] = build_empty(1, m_);
        for (int i = 1; i <= n; ++i) {
            int pos = static_cast<int>(
                          std::lower_bound(sorted_vals_.begin(), sorted_vals_.end(), a[i - 1]) -
                          sorted_vals_.begin()) +
                      1;  // 离散化后的 1-based 排名
            roots_[i] = update(roots_[i - 1], 1, m_, pos);
        }
    }

    // 查询 a[l..r] 中第 k 小（1-based，返回原值）
    ll kth(int l, int r, int k) const {
        return query(roots_[l - 1], roots_[r], 1, m_, k);
    }

private:
    // 主席树节点：lc/rc 为左右儿子在节点池中的下标，cnt 为子树内元素个数
    struct PstNode {
        int lc;
        int rc;
        int cnt;
    };

    std::vector<PstNode> pool_;          // 动态开点节点池
    std::vector<int> roots_;             // 各前缀版本根
    std::vector<ll> sorted_vals_;        // 去重后的值（1-based 排名 → 原值）
    int m_;                              // 值域大小（离散化后的不同值个数）

    int new_node(int lc, int rc, int cnt) {
        PstNode nd{lc, rc, cnt};
        pool_.push_back(nd);
        return static_cast<int>(pool_.size()) - 1;
    }

    // 建立全 0 空树（区间 [l,r]）
    int build_empty(int l, int r) {
        int u = new_node(0, 0, 0);
        if (l != r) {
            int mid = (l + r) >> 1;
            // 注意：先递归得到子节点下标，再写回 pool_[u]，避免 push_back
            // 触发 vector 重分配后 pool_[u] 引用失效（求值顺序陷阱）。
            int lc = build_empty(l, mid);
            int rc = build_empty(mid + 1, r);
            pool_[u].lc = lc;
            pool_[u].rc = rc;
        }
        return u;
    }

    // 在 prev 版本基础上，给位置 pos 计数 +1，返回新版本根
    int update(int prev, int l, int r, int pos) {
        int u = new_node(pool_[prev].lc, pool_[prev].rc, pool_[prev].cnt + 1);
        if (l != r) {
            int mid = (l + r) >> 1;
            if (pos <= mid) {
                int nlc = update(pool_[prev].lc, l, mid, pos);
                pool_[u].lc = nlc;
            } else {
                int nrc = update(pool_[prev].rc, mid + 1, r, pos);
                pool_[u].rc = nrc;
            }
        }
        return u;
    }

    // 在 [root(l-1), root(r)] 两个版本的差集上求第 k 小
    ll query(int u, int v, int l, int r, int k) const {
        if (l == r) return sorted_vals_[l - 1];
        int mid = (l + r) >> 1;
        // 左子树内的元素个数 = v 左子树个数 - u 左子树个数
        int cnt_left = pool_[pool_[v].lc].cnt - pool_[pool_[u].lc].cnt;
        if (k <= cnt_left) return query(pool_[u].lc, pool_[v].lc, l, mid, k);
        return query(pool_[u].rc, pool_[v].rc, mid + 1, r, k - cnt_left);
    }
};

// ============================================================================
// PersistentArray —— 可持久化数组
// ============================================================================
class PersistentArray {
public:
    // 构造大小为 n 的可持久化数组（下标 1..n）
    explicit PersistentArray(int n) : n_(n) {
        pool_.clear();
        pool_.push_back(PstArrNode{0, 0, 0});  // 0 号占位节点
    }

    // 以数组 a 建立初始版本，返回该版本根
    int build(const std::vector<ll>& a) { return build(1, n_, a); }

    // 在 prev_root 版本上把 pos 置为 val，返回新版本根
    int update(int prev_root, int pos, ll val) {
        return update(prev_root, 1, n_, pos, val);
    }

    // 查询 root 版本中 pos 的值
    ll query(int root, int pos) const { return query(root, 1, n_, pos); }

private:
    // 可持久化数组节点：lc/rc 为左右儿子，val 为叶子值
    struct PstArrNode {
        int lc;
        int rc;
        ll val;
    };

    int n_;
    std::vector<PstArrNode> pool_;

    int new_node(const PstArrNode& src) {
        pool_.push_back(src);
        return static_cast<int>(pool_.size()) - 1;
    }

    int build(int l, int r, const std::vector<ll>& a) {
        PstArrNode nd{0, 0, 0};
        int u = new_node(nd);
        if (l == r) {
            pool_[u].val = a[l - 1];
        } else {
            int mid = (l + r) >> 1;
            int lc = build(l, mid, a);
            int rc = build(mid + 1, r, a);
            pool_[u].lc = lc;
            pool_[u].rc = rc;
        }
        return u;
    }

    int update(int u, int l, int r, int pos, ll val) {
        // 复制当前节点，形成新版本
        PstArrNode nd = pool_[u];
        int v = new_node(nd);
        if (l == r) {
            pool_[v].val = val;
        } else {
            int mid = (l + r) >> 1;
            if (pos <= mid) {
                int nlc = update(pool_[u].lc, l, mid, pos, val);
                pool_[v].lc = nlc;
            } else {
                int nrc = update(pool_[u].rc, mid + 1, r, pos, val);
                pool_[v].rc = nrc;
            }
        }
        return v;
    }

    ll query(int u, int l, int r, int pos) const {
        if (l == r) return pool_[u].val;
        int mid = (l + r) >> 1;
        if (pos <= mid) return query(pool_[u].lc, l, mid, pos);
        return query(pool_[u].rc, mid + 1, r, pos);
    }
};

}  // namespace algo
