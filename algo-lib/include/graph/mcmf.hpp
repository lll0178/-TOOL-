#pragma once
// ============================================================================
// graph/mcmf.hpp —— 最小费用最大流（SPFA 增广）
// ----------------------------------------------------------------------------
// 功能：求从 s 到 t 的「最大流」中费用最小的方案（最小费用最大流）。
//       允许负费用边，但网络中不能存在负费用环。
//   - add_edge：加一条 u -> v、容量 cap、单位费用 cost 的边。
//   - min_cost_flow：返回 {最大流量, 最小总费用}。
// 时间复杂度：上界 O(F * n * m)（F 为最大流量；每次 SPFA O(nm) 增广一次）。
//             实际中通常远快于该上界。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/mcmf.hpp"
//   using namespace algo;
//   MCMF mf(4);
//   mf.add_edge(1, 2, 2, 1);   // 1->2 容量 2 费用 1
//   mf.add_edge(1, 3, 1, 5);
//   mf.add_edge(2, 4, 2, 1);
//   mf.add_edge(3, 4, 1, 1);
//   pair<ll,ll> res = mf.min_cost_flow(1, 4);  // {3, 10}（最大流 3，最小费用 10）
// 注意事项：
//   1) 顶点编号 1..n；容量与费用均为 long long，注意总费用可能较大。
//   2) add_edge 自动建立费用取负、容量为 0 的反向边（用于退流退费）。
//   3) 图中不能有负费用环，否则 SPFA 无法终止/结果错误。
//   4) 每次沿 s->t 的「费用最短路」增广（pre 数组记录路径边），逐次增广直至无法增广。
// ============================================================================

#include "algo/common.hpp"

#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

namespace algo {

class MCMF {
private:
    // 费用流网络边（命名唯一，嵌套于类内，避免冲突）
    struct CostEdge {
        int to, rev;
        ll cap, cost;
    };

    int n;
    std::vector<std::vector<CostEdge>> g;

public:
    explicit MCMF(int n) : n(n), g(n + 1) {}

    // 加一条 u -> v、容量 cap、单位费用 cost 的边
    void add_edge(int u, int v, ll cap, ll cost) {
        CostEdge a{v, (int)g[v].size(), cap, cost};
        CostEdge b{u, (int)g[u].size(), 0, -cost};
        g[u].push_back(a);
        g[v].push_back(b);
    }

    // 返回 {最大流量, 最小费用}
    std::pair<ll, ll> min_cost_flow(int s, int t) {
        ll flow = 0, cost = 0;
        std::vector<ll> dist(n + 1);
        std::vector<int> prev_v(n + 1), prev_e(n + 1);  // 前驱顶点与前驱边下标
        std::vector<char> inq(n + 1, 0);
        while (true) {
            // SPFA 找 s -> t 的费用最短路（允许负费用边）
            std::fill(dist.begin(), dist.end(), INF);
            std::fill(inq.begin(), inq.end(), (char)0);
            dist[s] = 0;
            std::queue<int> q;
            q.push(s);
            inq[s] = 1;
            while (!q.empty()) {
                int u = q.front();
                q.pop();
                inq[u] = 0;
                for (int i = 0; i < (int)g[u].size(); ++i) {
                    const CostEdge& e = g[u][i];
                    // e.cap > 0 保证只走有残余容量的边；!= INF 防止溢出
                    if (e.cap > 0 && dist[u] != INF && dist[e.to] > dist[u] + e.cost) {
                        dist[e.to] = dist[u] + e.cost;
                        prev_v[e.to] = u;
                        prev_e[e.to] = i;
                        if (!inq[e.to]) {
                            q.push(e.to);
                            inq[e.to] = 1;
                        }
                    }
                }
            }
            if (dist[t] == INF) {
                break;  // 无增广路，已得到最大流
            }
            // 沿最短路找最小残余容量
            ll add = INF;
            for (int v = t; v != s; v = prev_v[v]) {
                ll cap = g[prev_v[v]][prev_e[v]].cap;
                if (cap < add) {
                    add = cap;
                }
            }
            // 沿最短路增广
            for (int v = t; v != s; v = prev_v[v]) {
                CostEdge& e = g[prev_v[v]][prev_e[v]];
                e.cap -= add;
                g[v][e.rev].cap += add;
            }
            flow += add;
            cost += add * dist[t];
        }
        return std::pair<ll, ll>(flow, cost);
    }
};

}  // namespace algo
