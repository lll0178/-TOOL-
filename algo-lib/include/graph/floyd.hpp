#pragma once
// ============================================================================
// graph/floyd.hpp —— Floyd-Warshall 全源最短路与传递闭包
// ----------------------------------------------------------------------------
// 功能：
//   - floyd：O(n^3) 求全源最短路（支持负权边，不允许负环）。
//   - transitive_closure：求有向图的传递闭包（i 是否能到达 j）。
// 时间复杂度：floyd O(n^3)；传递闭包 O(n^3)。
// 空间复杂度：O(n^2)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/floyd.hpp"
//   using namespace algo;
//   int n = 3;
//   vector<vector<ll>> d(n + 1, vector<ll>(n + 1, INF));
//   for (int i = 1; i <= n; ++i) d[i][i] = 0;   // 对角线置 0
//   d[1][2] = 5; d[2][3] = 2;                   // 填入边权
//   floyd(d);                                    // 之后 d[i][j] = i->j 最短路
//
//   vector<pair<int,int>> edges;
//   edges.push_back({1, 2}); edges.push_back({2, 3});
//   vector<vector<bool>> reach = transitive_closure(n, edges);
// 注意事项：
//   1) d 为 (n+1)x(n+1)，使用下标 1..n；调用前需自行初始化
//      （全部 INF、对角线 0、边权填入）。
//   2) 不允许负环：存在负环时会出现 d[i][i] < 0。
//   3) 判断不可达用 d[i][j] == INF；代码用 != INF 保护，避免 INF 加法溢出。
//   4) 传递闭包可优化：若 n 很大，每行用 bitset 按位或可做到 O(n^3/64)，
//      本实现以正确性优先使用 bool 矩阵。
// ============================================================================

#include "algo/common.hpp"

#include <utility>
#include <vector>

namespace algo {

// Floyd-Warshall：原地更新 d（下标 1..n）。
// 调用前：d[i][i] = 0，无边处 d[i][j] = INF，有边处 d[i][j] = 边权。
inline void floyd(std::vector<std::vector<ll>>& d) {
    int n = (int)d.size() - 1;  // 顶点编号 1..n
    for (int k = 1; k <= n; ++k) {
        for (int i = 1; i <= n; ++i) {
            if (d[i][k] == INF) {
                continue;  // i 无法到达 k，跳过（同时避免 INF 加法溢出）
            }
            for (int j = 1; j <= n; ++j) {
                if (d[k][j] == INF) {
                    continue;
                }
                if (d[i][j] > d[i][k] + d[k][j]) {
                    d[i][j] = d[i][k] + d[k][j];
                }
            }
        }
    }
}

// 传递闭包：返回 (n+1)x(n+1) 的 bool 矩阵，reach[i][j] 表示 i 是否能到达 j。
// edges 为有向边列表（顶点编号 1..n）。包含自反性：reach[i][i] 恒为 true。
inline std::vector<std::vector<bool>> transitive_closure(
    int n, const std::vector<std::pair<int, int>>& edges) {
    std::vector<std::vector<bool>> reach(n + 1, std::vector<bool>(n + 1, false));
    for (int i = 1; i <= n; ++i) {
        reach[i][i] = true;
    }
    for (const std::pair<int, int>& e : edges) {
        reach[e.first][e.second] = true;
    }
    for (int k = 1; k <= n; ++k) {
        for (int i = 1; i <= n; ++i) {
            if (!reach[i][k]) {
                continue;
            }
            for (int j = 1; j <= n; ++j) {
                if (reach[k][j]) {
                    reach[i][j] = true;
                }
            }
        }
    }
    return reach;
}

}  // namespace algo
