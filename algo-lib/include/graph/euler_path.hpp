#pragma once
// ============================================================================
// graph/euler_path.hpp —— 欧拉路径 / 欧拉回路（Hierholzer 算法）
// ----------------------------------------------------------------------------
// 功能：求无向图或有向图的欧拉路径/回路。
//   函数内部先做存在性检查（度数条件 + 非零度点连通性），不满足返回空 vector。
// 时间复杂度：O(n + m)。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "graph/euler_path.hpp"
//   using namespace algo;
//   int n = 4;
//   vector<vector<pair<int,int>>> g(n + 1);  // {v, edge_id}，edge_id 从 0 开始
//   int eid = 0;
//   // 无向图：每条无向边以相同 eid 在两端各存一次
//   g[1].push_back({2, eid}); g[2].push_back({1, eid}); ++eid;
//   g[2].push_back({3, eid}); g[3].push_back({2, eid}); ++eid;
//   g[3].push_back({4, eid}); g[4].push_back({3, eid});
//   vector<int> path = euler_path(n, g, false);  // 长度为 4（3 条边 + 1）
// 注意事项：
//   1) g[u] = {(v, edge_id)}，edge_id 从 0 开始；无向图同一条边两端用相同 edge_id。
//   2) 存在性条件：
//        无向图：奇度点数为 0（回路）或 2（路径）；且所有非零度点连通。
//        有向图：至多一个点出度-入度 = 1（起点），至多一个点入度-出度 = 1（终点），
//                其余点出入度相等；且底图（忽略方向）非零度点连通。
//   3) 无向图奇度点恰为 2 个时，以其中一个奇度点为起点；否则取任意非零度点。
//   4) 返回路径长度 = 边数 + 1（含起点）；不存在时返回空 vector。
//   5) 无向图重边、自环均被正确处理（自环需在 g[u] 中存两个相同 eid 的条目）。
// ============================================================================

#include <algorithm>
#include <utility>
#include <vector>

namespace algo {

// 欧拉路径 / 欧拉回路（Hierholzer 算法）。
// g[u] = {(v, edge_id)}，edge_id 从 0 开始。
//   - 无向图（directed=false）：每条无向边 (u,v) 以相同 edge_id 在 g[u]、g[v] 各存一次；
//   - 有向图（directed=true）：每条有向边 u->v 以唯一 edge_id 在 g[u] 存一次。
// 返回路径点序列（长度 = 边数 + 1，含起点）；不存在时返回空 vector。
inline std::vector<int> euler_path(int n, std::vector<std::vector<std::pair<int, int>>> g,
                                   bool directed) {
    // 1) 统计入度、出度、总边条目数与最大边编号
    std::vector<int> indeg(n + 1, 0), outdeg(n + 1, 0);
    int total = 0;  // 邻接表中边条目总数（无向图每条边算两次）
    int max_eid = -1;
    for (int u = 1; u <= n; ++u) {
        outdeg[u] = (int)g[u].size();
        for (const std::pair<int, int>& p : g[u]) {
            ++indeg[p.first];
            ++total;
            if (p.second > max_eid) {
                max_eid = p.second;
            }
        }
    }

    // 2) 度数存在性检查
    int start = -1;
    if (directed) {
        int start_cnt = 0, end_cnt = 0;
        for (int u = 1; u <= n; ++u) {
            int d = outdeg[u] - indeg[u];
            if (d == 1) {
                ++start_cnt;
                start = u;
            } else if (d == -1) {
                ++end_cnt;
            } else if (d != 0) {
                return std::vector<int>();
            }
        }
        if (!((start_cnt == 0 && end_cnt == 0) || (start_cnt == 1 && end_cnt == 1))) {
            return std::vector<int>();
        }
    } else {
        int odd = 0;
        for (int u = 1; u <= n; ++u) {
            if (outdeg[u] % 2 == 1) {
                ++odd;
                start = u;  // 记录最后一个奇度点（作为起点）
            }
        }
        if (odd != 0 && odd != 2) {
            return std::vector<int>();
        }
    }

    if (total == 0) {
        // 无边：欧拉路径退化为单点（约定返回顶点 1）
        return (n >= 1) ? std::vector<int>(1, 1) : std::vector<int>();
    }

    // 3) 连通性检查：非零度点必须同属一个连通块（孤立点忽略）。
    //    有向图检查其底图（忽略方向）的连通性。
    std::vector<int> fa(n + 1);
    for (int i = 1; i <= n; ++i) {
        fa[i] = i;
    }
    // 迭代 find + 路径压缩（非递归，避免深递归）
    auto find = [&fa](int x) {
        int r = x;
        while (fa[r] != r) {
            r = fa[r];
        }
        while (fa[x] != r) {
            int nx = fa[x];
            fa[x] = r;
            x = nx;
        }
        return r;
    };
    for (int u = 1; u <= n; ++u) {
        for (const std::pair<int, int>& p : g[u]) {
            int a = find(u);
            int b = find(p.first);
            if (a != b) {
                fa[a] = b;
            }
        }
    }
    int comp = -1;
    for (int u = 1; u <= n; ++u) {
        if (outdeg[u] + indeg[u] == 0) {
            continue;  // 孤立点
        }
        int r = find(u);
        if (comp == -1) {
            comp = r;
        } else if (comp != r) {
            return std::vector<int>();  // 非零度点不连通
        }
    }

    // 4) 确定起点（若回路则取任意非零度点）
    if (start == -1) {
        for (int u = 1; u <= n; ++u) {
            if (outdeg[u] > 0) {
                start = u;
                break;
            }
        }
    }

    // 5) Hierholzer：used 边标记 + 栈
    std::vector<char> used(max_eid + 1, 0);
    std::vector<int> stk, path;
    stk.push_back(start);
    while (!stk.empty()) {
        int u = stk.back();
        bool advanced = false;
        while (!g[u].empty()) {
            std::pair<int, int> p = g[u].back();
            g[u].pop_back();
            int v = p.first;
            int eid = p.second;
            if (used[eid]) {
                continue;  // 无向图的反向边（同 eid）已用过，跳过
            }
            used[eid] = 1;
            stk.push_back(v);
            advanced = true;
            break;
        }
        if (!advanced) {
            path.push_back(u);
            stk.pop_back();
        }
    }
    std::reverse(path.begin(), path.end());

    // 防御性校验：路径长度应为「边数 + 1」
    int edge_cnt = directed ? total : total / 2;
    if ((int)path.size() != edge_cnt + 1) {
        return std::vector<int>();
    }
    return path;
}

}  // namespace algo
