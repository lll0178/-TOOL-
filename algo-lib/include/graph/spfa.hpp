#pragma once
// ============================================================================
// graph/spfa.hpp —— SPFA 与 Bellman-Ford 单源最短路（可含负权）
// ----------------------------------------------------------------------------
// 功能：求解带权图（可含负权边）的单源最短路，并检测负环。
//   - spfa：队列优化的 Bellman-Ford（SPFA），检测从源点可达的负环。
//   - bellman_ford：边表版 Bellman-Ford，O(nm)，检测负环。
// 时间复杂度：SPFA 最坏 O(nm)，平均 O(km)（k 通常很小）；Bellman-Ford O(nm)。
// 空间复杂度：SPFA O(n)；Bellman-Ford O(n)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/spfa.hpp"
//   using namespace algo;
//   int n = 3;
//   vector<vector<Edge>> g(n + 1);
//   g[1].push_back({2, 5});
//   g[2].push_back({3, -2});          // 负权边
//   vector<ll> dist(n + 1, INF);
//   bool neg = spfa(n, g, 1, dist);   // dist[3] == 3，neg == false
//
//   vector<BFEdge> edges;
//   edges.push_back({1, 2, 5});
//   edges.push_back({2, 3, -2});
//   bool has_neg = false;
//   vector<ll> d2 = bellman_ford(n, edges, 1, has_neg);
// 注意事项：
//   1) SPFA 在稠密图或构造数据上可能退化到 O(nm)；非负权最短路请用 dijkstra。
//   2) spfa 的 dist 需由调用者初始化为 INF（函数内会把 dist[s] 置 0）。
//   3) 两者都只能检测「从源点 s 可达」的负环；不可达的负环不会触发松弛。
//   4) 判断不可达用 dist[v] == INF；代码用 != INF 保护，避免 INF + w 溢出。
// ============================================================================

#include "algo/common.hpp"

#include <queue>
#include <vector>

namespace algo {

// Bellman-Ford 边表版使用的边（命名唯一，避免与 Edge / MSTEdge 等冲突）
struct BFEdge {
    int u, v;  // 有向边 u -> v
    ll w;      // 权值（可为负）
};

// SPFA：dist 需预先初始化为 INF（函数内把 dist[s] 置 0）。
// 返回 true 表示检测到「从 s 可达」的负环，此时 dist 无意义。
inline bool spfa(int n, const std::vector<std::vector<Edge>>& g, int s,
                 std::vector<ll>& dist) {
    std::vector<char> inq(n + 1, 0);  // 顶点是否在队列中
    std::vector<int> cnt(n + 1, 0);   // 各顶点入队次数（判负环用）
    std::queue<int> q;
    dist[s] = 0;
    q.push(s);
    inq[s] = 1;
    cnt[s] = 1;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        inq[u] = 0;
        for (const Edge& e : g[u]) {
            // 用 != INF 防止 INF + w 溢出
            if (dist[u] != INF && dist[e.to] > dist[u] + e.w) {
                dist[e.to] = dist[u] + e.w;
                if (!inq[e.to]) {
                    q.push(e.to);
                    inq[e.to] = 1;
                    if (++cnt[e.to] >= n) {
                        return true;  // 某点入队次数 >= n，存在负环
                    }
                }
            }
        }
    }
    return false;
}

// Bellman-Ford 边表版：edges 为边列表（每条边含起点 u、终点 v、权 w）。
// 返回 dist[1..n]；has_neg_cycle 输出是否存在「从 s 可达」的负环。
inline std::vector<ll> bellman_ford(int n, const std::vector<BFEdge>& edges, int s,
                                    bool& has_neg_cycle) {
    std::vector<ll> dist(n + 1, INF);
    dist[s] = 0;
    has_neg_cycle = false;
    // 松弛 n-1 轮（不含负环的最短路最多含 n-1 条边）
    for (int i = 0; i < n - 1; ++i) {
        bool updated = false;
        for (const BFEdge& e : edges) {
            if (dist[e.u] != INF && dist[e.v] > dist[e.u] + e.w) {
                dist[e.v] = dist[e.u] + e.w;
                updated = true;
            }
        }
        if (!updated) {
            break;  // 本轮无更新，已收敛，提前结束
        }
    }
    // 第 n 轮：若还能松弛，说明存在负环
    for (const BFEdge& e : edges) {
        if (dist[e.u] != INF && dist[e.v] > dist[e.u] + e.w) {
            has_neg_cycle = true;
            break;
        }
    }
    return dist;
}

}  // namespace algo
