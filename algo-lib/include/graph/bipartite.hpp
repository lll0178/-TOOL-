#pragma once
// ============================================================================
// graph/bipartite.hpp —— 二分图判定与最大匹配（匈牙利算法）
// ----------------------------------------------------------------------------
// 功能：
//   - is_bipartite：BFS 染色判断无向图是否为二分图。
//   - hungarian：匈牙利算法求二分图最大匹配。
// 时间复杂度：is_bipartite O(n + m)；hungarian O(n * 边数)（n 为左部点数）。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "graph/bipartite.hpp"
//   using namespace algo;
//   int n = 3;
//   vector<vector<int>> g(n + 1);
//   g[1].push_back(2); g[2].push_back(1);
//   g[2].push_back(3); g[3].push_back(2);
//   vector<int> color;
//   bool ok = is_bipartite(n, g, color);   // true，color 为 0/1 染色
//
//   int L = 2, R = 2;                       // 左部 1..L，右部 1..R
//   vector<vector<int>> adj(L + 1);
//   adj[1].push_back(1);
//   adj[2].push_back(1);
//   adj[2].push_back(2);
//   int match = hungarian(L, R, adj);       // 最大匹配 = 2
// 注意事项：
//   1) is_bipartite 适用于无向图（自动处理多个连通块）；color 输出为 0/1，
//      未染色点在内部用 -1 表示。
//   2) hungarian 左部编号 1..n、右部编号 1..m；adj[u] 为左部点 u 能匹配的右部点集合。
//   3) 相关结论（König 定理，对无孤立点的二分图）：
//       最小点覆盖数 = 最大匹配数；
//       最大独立集大小 = n + m - 最大匹配数；
//       最小边覆盖数 = 顶点数 - 最大匹配数。
// ============================================================================

#include <functional>
#include <queue>
#include <vector>

namespace algo {

// 二分图判定：BFS 染色。color 输出每个点的颜色 0/1。
// 返回 true 表示是二分图；false 表示不是（此时 color 无意义）。
inline bool is_bipartite(int n, const std::vector<std::vector<int>>& g,
                         std::vector<int>& color) {
    color.assign(n + 1, -1);
    std::queue<int> q;
    for (int s = 1; s <= n; ++s) {
        if (color[s] != -1) {
            continue;  // 已染色，跳过（处理多个连通块）
        }
        color[s] = 0;
        q.push(s);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : g[u]) {
                if (color[v] == -1) {
                    color[v] = color[u] ^ 1;
                    q.push(v);
                } else if (color[v] == color[u]) {
                    return false;  // 相邻两点同色，非二分图
                }
            }
        }
    }
    return true;
}

// 匈牙利算法求二分图最大匹配。
// 左部点 1..n，右部点 1..m；adj[u] 为左部点 u 可匹配的右部点集合。
// 返回最大匹配数。
inline int hungarian(int n, int m, const std::vector<std::vector<int>>& adj) {
    std::vector<int> match(m + 1, 0);  // match[v] = 匹配到右部点 v 的左部点（0 表示未匹配）
    std::vector<char> vis(m + 1, 0);
    // 增广路 DFS（递归 lambda 需要 std::function）
    std::function<bool(int)> dfs = [&](int u) -> bool {
        for (int v : adj[u]) {
            if (vis[v]) {
                continue;
            }
            vis[v] = 1;
            // 右部点 v 未匹配，或 v 当前匹配的左部点能改配到别的点
            if (match[v] == 0 || dfs(match[v])) {
                match[v] = u;
                return true;
            }
        }
        return false;
    };
    int ans = 0;
    for (int u = 1; u <= n; ++u) {
        vis.assign(m + 1, 0);
        if (dfs(u)) {
            ++ans;
        }
    }
    return ans;
}

}  // namespace algo
