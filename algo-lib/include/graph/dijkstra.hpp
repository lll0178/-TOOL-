#pragma once
// ============================================================================
// graph/dijkstra.hpp —— 堆优化 Dijkstra 单源最短路（非负权）
// ----------------------------------------------------------------------------
// 功能：求带非负边权图（有向或无向，顶点编号 1..n）从源点 s 到其余各点的最短距离；
//       可选记录每个点的前驱，用于还原最短路径。
// 时间复杂度：O((n + m) log n)，n 为顶点数，m 为边数。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/dijkstra.hpp"
//   using namespace algo;
//   int n = 3;
//   vector<vector<Edge>> g(n + 1);
//   g[1].push_back({2, 5});   // 1 -> 2 权 5
//   g[2].push_back({3, 2});   // 2 -> 3 权 2
//   vector<int> pre;
//   vector<ll> d = dijkstra(n, g, 1, &pre);
//   // 还原 1 -> 3 的最短路径：
//   vector<int> path;
//   for (int v = 3; v != -1; v = pre[v]) path.push_back(v);
//   reverse(path.begin(), path.end());   // path = {1, 2, 3}
// 注意事项：
//   1) 边权必须非负；有负权边请用 spfa / bellman_ford（见 spfa.hpp）。
//   2) 不可达点的距离为 algo::INF；判断不可达用 d[v] == INF，
//      不要对 INF 做加法（会溢出）。
//   3) 重边、自环均被正确处理（自环不会改善距离，因为不会满足松弛条件）。
// ============================================================================

#include "algo/common.hpp"

#include <functional>
#include <queue>
#include <utility>
#include <vector>

namespace algo {

// 堆优化 Dijkstra：返回 dist[1..n]，dist[s] = 0，不可达为 INF。
// pre 非空时：(*pre)[v] 记录到达 v 的前驱点，源点前驱为 -1，用于还原路径。
inline std::vector<ll> dijkstra(int n, const std::vector<std::vector<Edge>>& g,
                                int s, std::vector<int>* pre = nullptr) {
    std::vector<ll> dist(n + 1, INF);
    if (pre != nullptr) {
        pre->assign(n + 1, -1);
        (*pre)[s] = -1;
    }
    dist[s] = 0;

    typedef std::pair<ll, int> P;  // {当前最短距离, 顶点}
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
    pq.push(P(0, s));
    while (!pq.empty()) {
        P cur = pq.top();
        pq.pop();
        ll d = cur.first;
        int u = cur.second;
        if (d != dist[u]) {
            continue;  // 过期堆项（懒惰删除）：该点的距离已被更优值更新过
        }
        for (const Edge& e : g[u]) {
            if (dist[e.to] > dist[u] + e.w) {
                dist[e.to] = dist[u] + e.w;
                if (pre != nullptr) {
                    (*pre)[e.to] = u;
                }
                pq.push(P(dist[e.to], e.to));
            }
        }
    }
    return dist;
}

}  // namespace algo
