#pragma once
// ============================================================================
// graph/tarjan.hpp —— 强连通分量（Tarjan）
// ----------------------------------------------------------------------------
// 功能：求有向图的强连通分量（SCC）。
//   - add_edge：加有向边 u -> v。
//   - run：加完所有边后调用一次，执行 Tarjan。
//   - count：返回 SCC 个数。
//   - id：id[x] 为顶点 x 所属 SCC 编号（1..count）。
//   - build_dag：返回缩点后的 DAG 邻接表（可能含重边）。
// 时间复杂度：O(n + m)。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "graph/tarjan.hpp"
//   using namespace algo;
//   SCC scc(4);
//   scc.add_edge(1, 2); scc.add_edge(2, 1);
//   scc.add_edge(2, 3); scc.add_edge(3, 4); scc.add_edge(4, 3);
//   scc.run();
//   int c = scc.count();                    // 2
//   int id2 = scc.id()[2];                  // 顶点 2 所属分量
//   vector<vector<int>> dag = scc.build_dag();
// 注意事项：
//   1) Tarjan 的 DFS 为递归实现，最坏递归深度可达 n，超大图需注意栈空间。
//   2) build_dag 的结果可能含重边（多个原边对应同一条缩点边），需要时自行去重。
//   3) Tarjan 给 SCC 编号的顺序是「反向拓扑序」：编号大的 SCC 在 DAG 中更靠前（源侧）。
// ============================================================================

#include <algorithm>
#include <vector>

namespace algo {

class SCC {
private:
    int n;
    std::vector<std::vector<int>> g;
    std::vector<int> dfn, low, scc_id;
    std::vector<int> stk;
    std::vector<char> in_stk;
    int timer, scc_cnt;

    void dfs(int u) {
        dfn[u] = low[u] = ++timer;
        stk.push_back(u);
        in_stk[u] = 1;
        for (int v : g[u]) {
            if (dfn[v] == 0) {
                dfs(v);
                low[u] = std::min(low[u], low[v]);
            } else if (in_stk[v]) {
                low[u] = std::min(low[u], dfn[v]);
            }
        }
        if (low[u] == dfn[u]) {  // u 是其 SCC 的根
            ++scc_cnt;
            while (true) {
                int x = stk.back();
                stk.pop_back();
                in_stk[x] = 0;
                scc_id[x] = scc_cnt;
                if (x == u) {
                    break;
                }
            }
        }
    }

public:
    explicit SCC(int n)
        : n(n), g(n + 1), dfn(n + 1, 0), low(n + 1, 0), scc_id(n + 1, 0),
          in_stk(n + 1, 0), timer(0), scc_cnt(0) {}

    void add_edge(int u, int v) { g[u].push_back(v); }

    // 加完所有边后调用一次
    void run() {
        for (int i = 1; i <= n; ++i) {
            if (dfn[i] == 0) {
                dfs(i);
            }
        }
    }

    int count() const { return scc_cnt; }

    // id[x]：顶点 x 所属 SCC 编号（1..count）
    const std::vector<int>& id() const { return scc_id; }

    // 缩点后的 DAG 邻接表（下标 1..count）；可能含重边，可自行去重。
    std::vector<std::vector<int>> build_dag() const {
        std::vector<std::vector<int>> dag(scc_cnt + 1);
        for (int u = 1; u <= n; ++u) {
            for (int v : g[u]) {
                if (scc_id[u] != scc_id[v]) {
                    dag[scc_id[u]].push_back(scc_id[v]);
                }
            }
        }
        return dag;
    }
};

}  // namespace algo
