#pragma once
// ============================================================================
// algo/ds/hld.hpp —— 树链剖分（Heavy-Light Decomposition）
// ----------------------------------------------------------------------------
// 功能：把树上路径/子树的操作转化为若干条 dfs 序连续区间，用线段树维护。
//       支持：路径区间加、路径求和、子树区间加、子树求和、单点赋值。
// 时间复杂度：build O(n)；路径操作 O(log^2 n)；子树操作 O(log n)。
// 空间复杂度：O(n)。
// 使用示例：
//   HLD hld(5);
//   hld.add_edge(1, 2);
//   hld.add_edge(1, 3);
//   hld.add_edge(3, 4);
//   hld.add_edge(3, 5);
//   hld.build(1);                 // 以 1 为根（add_edge 全部完成后调用）
//   hld.path_add(4, 5, 10);       // 路径 4-5 上所有节点权值 +10
//   hld.subtree_add(3, 2);        // 3 的子树（3,4,5）+2
//   ll s = hld.path_sum(4, 5);    // 查询
// 注意事项：
//   - 节点编号 1..n；build 必须在所有 add_edge 之后调用。
//   - 初始节点权值均为 0；可用 set_val 设置（内部即单点赋值）。
//   - 两遍 dfs 为递归实现，节点极深（如 1e6 的链）时注意栈空间。
// ============================================================================

#include <utility>  // std::swap
#include <vector>

#include "algo/common.hpp"

namespace algo {

class HLD {
public:
    explicit HLD(int n)
        : n_(n),
          g_(n + 1),
          fa_(n + 1, 0),
          dep_(n + 1, 0),
          sz_(n + 1, 0),
          heavy_(n + 1, 0),
          top_(n + 1, 0),
          dfn_(n + 1, 0),
          rnk_(n + 1, 0),
          seg_(n) {}

    // 加一条无向边（可在 build 前任意多次调用）
    void add_edge(int u, int v) {
        g_[u].push_back(v);
        g_[v].push_back(u);
    }

    // 以 root 为根做两遍 dfs 并建线段树；必须在所有 add_edge 之后调用
    void build(int root = 1) {
        dfs1(root, 0);
        timer_ = 0;
        dfs2(root, root);
        seg_ = HldSegTree(n_);
    }

    // 单点赋值：节点 u 权值设为 v
    void set_val(int u, ll v) { seg_.point_set(dfn_[u], v); }

    // 路径 u->v 上所有节点权值 += w
    void path_add(int u, int v, ll w) {
        while (top_[u] != top_[v]) {
            if (dep_[top_[u]] < dep_[top_[v]]) std::swap(u, v);
            seg_.range_add(dfn_[top_[u]], dfn_[u], w);
            u = fa_[top_[u]];
        }
        if (dep_[u] > dep_[v]) std::swap(u, v);
        seg_.range_add(dfn_[u], dfn_[v], w);
    }

    // 路径 u->v 上所有节点权值之和
    ll path_sum(int u, int v) {
        ll res = 0;
        while (top_[u] != top_[v]) {
            if (dep_[top_[u]] < dep_[top_[v]]) std::swap(u, v);
            res += seg_.range_sum(dfn_[top_[u]], dfn_[u]);
            u = fa_[top_[u]];
        }
        if (dep_[u] > dep_[v]) std::swap(u, v);
        res += seg_.range_sum(dfn_[u], dfn_[v]);
        return res;
    }

    // 节点 u 的整棵子树 += w
    void subtree_add(int u, ll w) { seg_.range_add(dfn_[u], dfn_[u] + sz_[u] - 1, w); }

    // 节点 u 的整棵子树权值之和
    ll subtree_sum(int u) { return seg_.range_sum(dfn_[u], dfn_[u] + sz_[u] - 1); }

private:
    // 内部线段树：区间加 + 区间和 + 单点赋值（按 dfs 序建）
    struct HldSegTree {
        int n;
        std::vector<ll> sum, add;

        explicit HldSegTree(int n) : n(n), sum(4 * n + 5, 0), add(4 * n + 5, 0) {}

        void apply(int p, int l, int r, ll v) {
            sum[p] += v * (r - l + 1);
            add[p] += v;
        }
        void pull(int p) { sum[p] = sum[p * 2] + sum[p * 2 + 1]; }
        void pushdown(int p, int l, int r) {
            if (add[p] == 0) return;
            int mid = (l + r) >> 1;
            apply(p * 2, l, mid, add[p]);
            apply(p * 2 + 1, mid + 1, r, add[p]);
            add[p] = 0;
        }

        void range_add(int p, int l, int r, int ql, int qr, ll v) {
            if (ql <= l && r <= qr) {
                apply(p, l, r, v);
                return;
            }
            pushdown(p, l, r);
            int mid = (l + r) >> 1;
            if (ql <= mid) range_add(p * 2, l, mid, ql, qr, v);
            if (qr > mid) range_add(p * 2 + 1, mid + 1, r, ql, qr, v);
            pull(p);
        }
        ll range_sum(int p, int l, int r, int ql, int qr) {
            if (ql <= l && r <= qr) return sum[p];
            pushdown(p, l, r);
            int mid = (l + r) >> 1;
            ll res = 0;
            if (ql <= mid) res += range_sum(p * 2, l, mid, ql, qr);
            if (qr > mid) res += range_sum(p * 2 + 1, mid + 1, r, ql, qr);
            return res;
        }
        void point_set(int p, int l, int r, int pos, ll v) {
            if (l == r) {
                sum[p] = v;
                return;
            }
            pushdown(p, l, r);
            int mid = (l + r) >> 1;
            if (pos <= mid) point_set(p * 2, l, mid, pos, v);
            else point_set(p * 2 + 1, mid + 1, r, pos, v);
            pull(p);
        }

        // 对外包装（1-based 区间）
        void range_add(int l, int r, ll v) { range_add(1, 1, n, l, r, v); }
        ll range_sum(int l, int r) { return range_sum(1, 1, n, l, r); }
        void point_set(int pos, ll v) { point_set(1, 1, n, pos, v); }
    };

    int n_;
    std::vector<std::vector<int>> g_;
    std::vector<int> fa_, dep_, sz_, heavy_, top_, dfn_, rnk_;
    HldSegTree seg_;
    int timer_ = 0;

    // 第一遍 dfs：求 fa / dep / sz / heavy（重儿子）
    void dfs1(int u, int p) {
        fa_[u] = p;
        dep_[u] = dep_[p] + 1;
        sz_[u] = 1;
        heavy_[u] = 0;
        int best = 0;
        for (int v : g_[u]) {
            if (v == p) continue;
            dfs1(v, u);
            sz_[u] += sz_[v];
            if (sz_[v] > best) {
                best = sz_[v];
                heavy_[u] = v;
            }
        }
    }

    // 第二遍 dfs：求 top / dfn / rnk（重链优先，保证子树 dfn 连续）
    void dfs2(int u, int tp) {
        top_[u] = tp;
        dfn_[u] = ++timer_;
        rnk_[timer_] = u;
        if (heavy_[u]) dfs2(heavy_[u], tp);  // 先走重儿子，保证重链 dfn 连续
        for (int v : g_[u]) {
            if (v == fa_[u] || v == heavy_[u]) continue;
            dfs2(v, v);  // 轻儿子作为新重链起点
        }
    }
};

}  // namespace algo
