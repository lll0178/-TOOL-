#pragma once
// ============================================================================
// graph/bridge.hpp —— 割点 / 桥 / 边双连通分量（Tarjan）
// ----------------------------------------------------------------------------
// 功能：
//   - cut_vertices：求无向图的割点（关节点）。
//   - bridges：求无向图的桥（用边编号正确处理重边）。
//   - EdgeBCC：求边双连通分量（删去所有桥后的连通块）。
// 时间复杂度：均为 O(n + m)。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "graph/bridge.hpp"
//   using namespace algo;
//   int n = 4;
//   vector<vector<int>> g(n + 1);
//   g[1].push_back(2); g[2].push_back(1);
//   g[2].push_back(3); g[3].push_back(2);
//   g[3].push_back(4); g[4].push_back(3);
//   vector<int> cuts = cut_vertices(n, g);   // {2, 3}
//
//   vector<vector<pair<int,int>>> g2(n + 1);  // {v, edge_id}
//   int eid = 0;
//   g2[1].push_back({2, eid}); g2[2].push_back({1, eid}); ++eid;
//   g2[2].push_back({3, eid}); g2[3].push_back({2, eid}); ++eid;
//   g2[3].push_back({4, eid}); g2[4].push_back({3, eid});
//   vector<pair<int,int>> br = bridges(n, g2);  // 每条边都是桥
//
//   EdgeBCC bcc(4);
//   bcc.add_edge(1, 2); bcc.add_edge(2, 3); bcc.add_edge(3, 4);
//   bcc.run();
//   int cnt = bcc.count();              // 4（每条边都是桥，各点独立成块）
// 注意事项：
//   1) cut_vertices 的 g 为简单无向图（无自环、无重边）；重边需用边编号版本判断。
//   2) bridges / EdgeBCC 使用边编号（edge_id）防重边：只跳过「来时经过的同一条边」，
//      而非跳过「父节点」，从而不把重边误判为桥。
//   3) EdgeBCC 内部为每对 (u,v) 分配唯一边编号（自 0 递增），重边会被正确区分。
//   4) Tarjan DFS 为递归实现，超大图需注意栈空间。
// ============================================================================

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

namespace algo {

// 割点（articulation points）：Tarjan，根节点特判子树数。
// 返回所有割点（顶点编号）。
// 注意：本实现跳过指向父节点的边（v == parent），适用于无自环、无重边的简单图；
//       若存在重边请改用边编号版本（参见 bridges）。
inline std::vector<int> cut_vertices(int n, const std::vector<std::vector<int>>& g) {
    std::vector<int> dfn(n + 1, 0), low(n + 1, 0);
    std::vector<char> is_cut(n + 1, 0);
    int timer = 0;
    std::function<void(int, int)> dfs = [&](int u, int parent) {
        dfn[u] = low[u] = ++timer;
        int child = 0;  // 以 u 为根的子 DFS 树个数
        for (int v : g[u]) {
            if (v == u) {
                continue;  // 忽略自环
            }
            if (v == parent) {
                continue;  // 跳过父边（简单图）
            }
            if (dfn[v] == 0) {
                ++child;
                dfs(v, u);
                low[u] = std::min(low[u], low[v]);
                if (parent != 0 && low[v] >= dfn[u]) {
                    is_cut[u] = 1;  // 子树 v 不能回到 u 的祖先
                }
            } else {
                low[u] = std::min(low[u], dfn[v]);
            }
        }
        if (parent == 0 && child > 1) {
            is_cut[u] = 1;  // 根节点：子树数 > 1 才是割点
        }
    };
    for (int i = 1; i <= n; ++i) {
        if (dfn[i] == 0) {
            dfs(i, 0);
        }
    }
    std::vector<int> res;
    for (int i = 1; i <= n; ++i) {
        if (is_cut[i]) {
            res.push_back(i);
        }
    }
    return res;
}

// 桥（bridges）：g[u] = {(v, edge_id)}，边编号防重边。
// 返回所有桥的端点对（{u, v}）。
// 为什么用边编号：若只用「v == parent」跳过父边，遇到重边（u-v 之间多条边）时，
//   会把其中一条当作父边跳过，从而把其余重边误判为桥；改用边编号后只跳过
//   「来时经过的同一条边」，重边的另一条仍会被当作回边处理，结果正确。
inline std::vector<std::pair<int, int>> bridges(
    int n, const std::vector<std::vector<std::pair<int, int>>>& g) {
    std::vector<int> dfn(n + 1, 0), low(n + 1, 0);
    std::vector<std::pair<int, int>> res;
    int timer = 0;
    std::function<void(int, int)> dfs = [&](int u, int from_edge) {
        dfn[u] = low[u] = ++timer;
        for (const std::pair<int, int>& p : g[u]) {
            int v = p.first;
            int eid = p.second;
            if (eid == from_edge) {
                continue;  // 跳过同一条边（正确处理重边）
            }
            if (dfn[v] == 0) {
                dfs(v, eid);
                low[u] = std::min(low[u], low[v]);
                if (low[v] > dfn[u]) {
                    res.push_back(std::pair<int, int>(u, v));
                }
            } else {
                low[u] = std::min(low[u], dfn[v]);
            }
        }
    };
    for (int i = 1; i <= n; ++i) {
        if (dfn[i] == 0) {
            dfs(i, -1);
        }
    }
    return res;
}

// 边双连通分量（Edge-BCC）：删去所有桥后，每个连通块是一个边双连通分量。
class EdgeBCC {
private:
    int n;
    int ecnt;  // 已分配的边编号（从 0 开始，用于处理重边）
    std::vector<std::vector<std::pair<int, int>>> g;  // {v, edge_id}
    std::vector<int> dfn, low, comp;
    std::vector<char> is_bridge;
    int timer, comp_cnt;

    void tarjan(int u, int from_edge) {
        dfn[u] = low[u] = ++timer;
        for (const std::pair<int, int>& p : g[u]) {
            int v = p.first;
            int eid = p.second;
            if (eid == from_edge) {
                continue;
            }
            if (dfn[v] == 0) {
                tarjan(v, eid);
                low[u] = std::min(low[u], low[v]);
                if (low[v] > dfn[u]) {
                    is_bridge[eid] = 1;
                }
            } else {
                low[u] = std::min(low[u], dfn[v]);
            }
        }
    }

    // 用迭代栈在非桥边上做 flood fill，得到每个顶点所属分量
    void flood_fill(int s) {
        std::vector<int> stk;
        stk.push_back(s);
        comp[s] = comp_cnt;
        while (!stk.empty()) {
            int u = stk.back();
            stk.pop_back();
            for (const std::pair<int, int>& p : g[u]) {
                int v = p.first;
                int eid = p.second;
                if (comp[v] == 0 && !is_bridge[eid]) {
                    comp[v] = comp_cnt;
                    stk.push_back(v);
                }
            }
        }
    }

public:
    explicit EdgeBCC(int n)
        : n(n), ecnt(0), g(n + 1), dfn(n + 1, 0), low(n + 1, 0), comp(n + 1, 0),
          timer(0), comp_cnt(0) {}

    void add_edge(int u, int v) {
        g[u].push_back(std::pair<int, int>(v, ecnt));
        g[v].push_back(std::pair<int, int>(u, ecnt));
        ++ecnt;
    }

    void run() {
        is_bridge.assign(ecnt, 0);
        for (int i = 1; i <= n; ++i) {
            if (dfn[i] == 0) {
                tarjan(i, -1);
            }
        }
        for (int i = 1; i <= n; ++i) {
            if (comp[i] == 0) {
                ++comp_cnt;
                flood_fill(i);
            }
        }
    }

    int count() const { return comp_cnt; }

    // id[x]：顶点 x 所属边双连通分量编号（1..count）
    const std::vector<int>& id() const { return comp; }
};

}  // namespace algo
