#pragma once
// ============================================================================
// graph/mst.hpp —— 最小生成树（Kruskal + Prim）
// ----------------------------------------------------------------------------
// 功能：求无向连通图的最小生成树（MST）权值。
//   - kruskal：边排序 + 并查集（边表输入）。
//   - prim：堆优化 Prim（邻接表输入）。
// 时间复杂度：kruskal O(m log m)；prim O((n + m) log n)。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/mst.hpp"
//   using namespace algo;
//   int n = 3;
//   vector<MSTEdge> edges;
//   edges.push_back({1, 2, 5});
//   edges.push_back({2, 3, 2});
//   edges.push_back({1, 3, 4});
//   ll w1 = kruskal(n, edges);   // 6
//
//   vector<vector<Edge>> g(n + 1);
//   g[1].push_back({2, 5}); g[2].push_back({1, 5});
//   g[2].push_back({3, 2}); g[3].push_back({2, 2});
//   g[1].push_back({3, 4}); g[3].push_back({1, 4});
//   ll w2 = prim(n, g);          // 6
// 注意事项：
//   1) 图不连通时返回 -1（MST 不存在）。
//   2) 顶点编号 1..n，要求 n >= 1；n == 1 时 MST 权值为 0。
//   3) kruskal 会修改传入的 edges（内部排序）；重边、负权边均正确处理。
//   4) prim 的 g 为无向带权邻接表：每条无向边必须在两端各存一次。
// ============================================================================

#include "algo/common.hpp"

#include <algorithm>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

namespace algo {

// Kruskal 使用的带权无向边（命名唯一，避免与 Edge / BFEdge 冲突）
struct MSTEdge {
    int u, v;
    ll w;
};

// Kruskal 内部并查集（命名唯一，保持本文件自包含，不引入其它 DSU 头文件）
struct MSTDSU {
    std::vector<int> fa, sz;
    explicit MSTDSU(int n) : fa(n + 1), sz(n + 1, 1) {
        for (int i = 1; i <= n; ++i) {
            fa[i] = i;
        }
    }
    int find(int x) { return fa[x] == x ? x : fa[x] = find(fa[x]); }
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) {
            return false;
        }
        if (sz[a] < sz[b]) {  // 按秩合并，保证树高 O(log n)
            int t = a;
            a = b;
            b = t;
        }
        fa[b] = a;
        sz[a] += sz[b];
        return true;
    }
};

// Kruskal：返回 MST 权值；图不连通返回 -1。
inline ll kruskal(int n, std::vector<MSTEdge>& edges) {
    std::sort(edges.begin(), edges.end(),
              [](const MSTEdge& a, const MSTEdge& b) { return a.w < b.w; });
    MSTDSU dsu(n);
    ll total = 0;
    int used = 0;
    for (const MSTEdge& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total += e.w;
            ++used;
            if (used == n - 1) {
                break;  // 已选出 n-1 条边
            }
        }
    }
    if (used != n - 1) {
        return -1;  // 选不出 n-1 条边，图不连通
    }
    return total;
}

// 堆优化 Prim：返回 MST 权值；图不连通返回 -1。
// g 为无向带权邻接表（每条无向边在两端各存一次）。
inline ll prim(int n, const std::vector<std::vector<Edge>>& g) {
    std::vector<char> in_tree(n + 1, 0);
    typedef std::pair<ll, int> P;  // {到当前生成树的最小边权, 顶点}
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push(P(0, 1));  // 从顶点 1 开始（MST 权值与起点无关）
    ll total = 0;
    int cnt = 0;
    while (!pq.empty()) {
        P cur = pq.top();
        pq.pop();
        ll w = cur.first;
        int u = cur.second;
        if (in_tree[u]) {
            continue;  // 已在生成树中，跳过
        }
        in_tree[u] = 1;
        total += w;
        ++cnt;
        for (const Edge& e : g[u]) {
            if (!in_tree[e.to]) {
                pq.push(P(e.w, e.to));
            }
        }
    }
    if (cnt != n) {
        return -1;  // 无法覆盖所有顶点，图不连通
    }
    return total;
}

}  // namespace algo
