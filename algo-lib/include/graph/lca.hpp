#pragma once
// ============================================================================
// graph/lca.hpp —— 倍增 LCA 与树上差分
// ----------------------------------------------------------------------------
// 功能：
//   - LCA：倍增法求树（无向无权）上两点的最近公共祖先，以及距离、k 级祖先。
//   - path_diff_add / path_diff_finalize：树上差分，统计路径对点/边的覆盖次数。
// 时间复杂度：build O(n log n)；每次 lca / dist / kth_ancestor O(log n)；
//             差分 finalize O(n)。
// 空间复杂度：O(n log n)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/lca.hpp"
//   using namespace algo;
//   int n = 4;
//   LCA lca(n);
//   lca.add_edge(1, 2); lca.add_edge(2, 3); lca.add_edge(3, 4);
//   lca.build(1);
//   int w = lca.lca(2, 4);          // 2
//   int d = lca.dist(2, 4);         // 2
//   int a = lca.kth_ancestor(4, 2); // 2（4 向上 2 级）
//
//   // 树上点差分：统计每个点被多少条路径覆盖
//   vector<vector<int>> tree(n + 1);
//   tree[1].push_back(2); tree[2].push_back(1);
//   tree[2].push_back(3); tree[3].push_back(2);
//   tree[3].push_back(4); tree[4].push_back(3);
//   vector<ll> diff(n + 1, 0);
//   int u = 2, v = 4, l = lca.lca(u, v), fl = lca.kth_ancestor(l, 1);
//   path_diff_add(diff, u, v, l, fl, 1);
//   vector<ll> cnt = path_diff_finalize(tree, 1, diff);  // cnt[x] = 点 x 被覆盖次数
// 注意事项：
//   1) 图为无向树，顶点编号 1..n；build 需在建完所有边后调用一次。
//   2) kth_ancestor(u, k)：k = 0 返回 u 自身；k 超过 u 的深度返回 0。
//   3) up 采用 up[LOG][n+1]（按层存储），build 时逐层填表、内层遍历顶点，
//      访问连续，比 up[n+1][LOG] 更缓存友好。
//   4) build 的 DFS 为递归实现，树很深（如链）时需注意栈空间。
//   5) 差分说明：
//        - 点差分（统计点被覆盖）：diff[u]+=w, diff[v]+=w, diff[lca]-=w, diff[fa(lca)]-=w；
//          调用 path_diff_add 即实现此式。
//        - 边差分（统计边被覆盖）：diff[u]+=w, diff[v]+=w, diff[lca]-=2w（不涉及 fa_lca）；
//          此时 finalize 后 diff[y] 表示边 (fa(y), y) 被覆盖次数（y 为下端节点）。
// ============================================================================

#include "algo/common.hpp"

#include <functional>
#include <vector>

namespace algo {

class LCA {
private:
    int n, LOG;
    std::vector<std::vector<int>> g;
    // up[k][v]：顶点 v 向上 2^k 级祖先。
    // 内存布局选择 up[LOG][n+1]（按层存储），见文件头注释。
    std::vector<std::vector<int>> up;
    std::vector<int> depth;

public:
    explicit LCA(int n) : n(n), g(n + 1), depth(n + 1, 0) {
        LOG = 1;
        while ((1LL << LOG) <= n) {  // 用 1LL 避免 1 << 31 的整型溢出
            ++LOG;
        }
        up.assign(LOG, std::vector<int>(n + 1, 0));
    }

    void add_edge(int u, int v) {
        g[u].push_back(v);
        g[v].push_back(u);
    }

    // 建完所有边后调用一次；root 默认 1。
    void build(int root = 1) {
        std::function<void(int, int)> dfs = [&](int u, int p) {
            up[0][u] = p;
            for (int v : g[u]) {
                if (v == p) {
                    continue;
                }
                depth[v] = depth[u] + 1;
                dfs(v, u);
            }
        };
        depth[root] = 0;
        dfs(root, 0);  // 根的父节点记为 0（哨兵）
        for (int k = 1; k < LOG; ++k) {
            for (int v = 1; v <= n; ++v) {
                up[k][v] = up[k - 1][up[k - 1][v]];
            }
        }
    }

    int lca(int u, int v) {
        if (depth[u] < depth[v]) {
            int t = u;
            u = v;
            v = t;
        }
        int diff = depth[u] - depth[v];
        for (int k = 0; k < LOG; ++k) {
            if (diff & (1 << k)) {
                u = up[k][u];
            }
        }
        if (u == v) {
            return u;
        }
        for (int k = LOG - 1; k >= 0; --k) {
            if (up[k][u] != up[k][v]) {
                u = up[k][u];
                v = up[k][v];
            }
        }
        return up[0][u];
    }

    // 返回 u 与 v 之间（树上）的边数距离
    int dist(int u, int v) {
        return depth[u] + depth[v] - 2 * depth[lca(u, v)];
    }

    // u 向上 k 级祖先；k 超过 u 的深度返回 0，k = 0 返回 u 自身。
    int kth_ancestor(int u, int k) {
        if (k > depth[u]) {
            return 0;
        }
        for (int j = 0; j < LOG; ++j) {
            if (k & (1 << j)) {
                u = up[j][u];
            }
        }
        return u;
    }
};

// 树上点差分：给路径 u - v 上所有「点」的计数器加 w。
// lca_node = lca(u, v)；fa_lca = lca_node 的父节点（lca 为根时取 0）。
// 实现：diff[u] += w; diff[v] += w; diff[lca_node] -= w; diff[fa_lca] -= w。
inline void path_diff_add(std::vector<ll>& diff, int u, int v, int lca_node,
                          int fa_lca, ll w) {
    diff[u] += w;
    diff[v] += w;
    diff[lca_node] -= w;
    diff[fa_lca] -= w;
}

// 树上差分汇总：后序遍历，把每个点的差分值累加到父节点，返回每个点的最终值。
// diff 传入前已通过 path_diff_add 累加好；返回 vector<ll>（下标 1..n）。
// 注意：g 必须是以 root 为根的连通无向树。
inline std::vector<ll> path_diff_finalize(const std::vector<std::vector<int>>& g,
                                          int root, std::vector<ll> diff) {
    int n = (int)diff.size() - 1;
    std::vector<int> parent(n + 1, 0);
    std::vector<char> vis(n + 1, 0);
    std::vector<int> order;  // DFS 顺序（父在前、子在后）
    order.reserve(n);
    std::vector<int> stk;
    stk.push_back(root);
    vis[root] = 1;
    while (!stk.empty()) {
        int u = stk.back();
        stk.pop_back();
        order.push_back(u);
        for (int v : g[u]) {
            if (vis[v]) {
                continue;
            }
            vis[v] = 1;
            parent[v] = u;
            stk.push_back(v);
        }
    }
    // 逆序遍历：先处理完子树（子节点已把子树和累加到自身），再把自身累加到父节点
    for (int i = (int)order.size() - 1; i >= 0; --i) {
        int u = order[i];
        if (parent[u] != 0) {
            diff[parent[u]] += diff[u];
        }
    }
    return diff;
}

}  // namespace algo
