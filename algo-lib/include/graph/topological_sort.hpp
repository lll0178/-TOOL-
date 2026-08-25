#pragma once
// ============================================================================
// graph/topological_sort.hpp —— 拓扑排序（Kahn 算法）
// ----------------------------------------------------------------------------
// 功能：对有向无环图（DAG）求拓扑序。
//   - topo_sort：Kahn 算法（queue），返回一种合法拓扑序。
//   - topo_sort_lex：字典序最小的拓扑序（小顶堆）。
// 时间复杂度：O(n + m)。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "graph/topological_sort.hpp"
//   using namespace algo;
//   int n = 3;
//   vector<vector<int>> g(n + 1);      // 无权有向图，顶点编号 1..n
//   g[1].push_back(2);
//   g[1].push_back(3);
//   g[2].push_back(3);
//   vector<int> order = topo_sort(n, g);     // 一种拓扑序，如 {1, 2, 3}
//   vector<int> lex  = topo_sort_lex(n, g);  // 字典序最小：{1, 2, 3}
// 注意事项：
//   1) 有环时返回空 vector（拓扑序不存在）。
//   2) 返回值为顶点编号序列；孤立点（入度出度均为 0）也会出现在拓扑序中。
// ============================================================================

#include <functional>
#include <queue>
#include <vector>

namespace algo {

// Kahn 拓扑排序：返回拓扑序（顶点编号）；有环返回空 vector。
inline std::vector<int> topo_sort(int n, const std::vector<std::vector<int>>& g) {
    std::vector<int> indeg(n + 1, 0);
    for (int u = 1; u <= n; ++u) {
        for (int v : g[u]) {
            ++indeg[v];
        }
    }
    std::queue<int> q;
    for (int u = 1; u <= n; ++u) {
        if (indeg[u] == 0) {
            q.push(u);
        }
    }
    std::vector<int> order;
    order.reserve(n);
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);
        for (int v : g[u]) {
            if (--indeg[v] == 0) {
                q.push(v);
            }
        }
    }
    if ((int)order.size() != n) {
        return std::vector<int>();  // 有环，拓扑序不存在
    }
    return order;
}

// 字典序最小的拓扑序：用小顶堆，每次取编号最小的入度为 0 的顶点。
inline std::vector<int> topo_sort_lex(int n, const std::vector<std::vector<int>>& g) {
    std::vector<int> indeg(n + 1, 0);
    for (int u = 1; u <= n; ++u) {
        for (int v : g[u]) {
            ++indeg[v];
        }
    }
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (int u = 1; u <= n; ++u) {
        if (indeg[u] == 0) {
            pq.push(u);
        }
    }
    std::vector<int> order;
    order.reserve(n);
    while (!pq.empty()) {
        int u = pq.top();
        pq.pop();
        order.push_back(u);
        for (int v : g[u]) {
            if (--indeg[v] == 0) {
                pq.push(v);
            }
        }
    }
    if ((int)order.size() != n) {
        return std::vector<int>();  // 有环
    }
    return order;
}

}  // namespace algo
