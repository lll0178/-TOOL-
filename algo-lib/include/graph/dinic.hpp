#pragma once
// ============================================================================
// graph/dinic.hpp —— Dinic 最大流
// ----------------------------------------------------------------------------
// 功能：BFS 分层 + DFS 多路增广 + 当前弧优化 的 Dinic 最大流。
//   - add_edge：加一条容量为 cap 的有向边 u -> v。
//   - maxflow：求 s 到 t 的最大流。
//   - min_cut_edges：maxflow 之后求最小割的原边端点对。
// 时间复杂度：一般图 O(n^2 m)；二分图/单位容量网络更快。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "algo/common.hpp"
//   #include "graph/dinic.hpp"
//   using namespace algo;
//   Dinic din(4);
//   din.add_edge(1, 2, 3);
//   din.add_edge(1, 3, 2);
//   din.add_edge(2, 4, 2);
//   din.add_edge(3, 4, 3);
//   ll f = din.maxflow(1, 4);              // 4
//   vector<pair<int,int>> cut = din.min_cut_edges(1, 4);
// 注意事项：
//   1) 顶点编号 1..n；容量用 long long，注意总流量可能较大。
//   2) add_edge 内部同时建立容量为 0 的反向边，用于退流。
//   3) min_cut_edges 必须在 maxflow(s, t) 之后调用（它基于当前残余网络做 BFS）。
//   4) DFS 多路增广为递归实现，最坏递归深度可达 n，超大图需注意栈空间。
// ============================================================================

#include "algo/common.hpp"

#include <algorithm>
#include <queue>
#include <utility>
#include <vector>

namespace algo {

class Dinic {
private:
    // 残余网络边（命名唯一，嵌套于类内，避免与其它文件的边结构冲突）
    struct FlowEdge {
        int to;   // 指向的顶点
        int rev;  // 反向边在 g[to] 中的下标
        ll cap;   // 残余容量
    };
    // 记录原边（用于 min_cut_edges）：u -> v，idx 为正向边在 g[u] 中的下标
    struct OrigEdge {
        int u, v, idx;
    };

    int n;
    std::vector<std::vector<FlowEdge>> g;
    std::vector<OrigEdge> orig;
    std::vector<int> level, iter;  // level：分层标号；iter：当前弧

    // BFS 建立分层图，返回 t 是否可达
    bool bfs(int s, int t) {
        std::fill(level.begin(), level.end(), -1);
        std::queue<int> q;
        level[s] = 0;
        q.push(s);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const FlowEdge& e : g[u]) {
                if (e.cap > 0 && level[e.to] < 0) {
                    level[e.to] = level[u] + 1;
                    q.push(e.to);
                }
            }
        }
        return level[t] >= 0;
    }

    // DFS 多路增广：从 u 出发向 t 推 f 的流量，返回实际推过的流量
    ll dfs(int u, int t, ll f) {
        if (u == t) {
            return f;
        }
        for (int& i = iter[u]; i < (int)g[u].size(); ++i) {  // 当前弧优化
            FlowEdge& e = g[u][i];
            if (e.cap > 0 && level[e.to] == level[u] + 1) {
                ll d = dfs(e.to, t, std::min(f, e.cap));
                if (d > 0) {
                    e.cap -= d;
                    g[e.to][e.rev].cap += d;
                    return d;
                }
            }
        }
        return 0;
    }

public:
    explicit Dinic(int n) : n(n), g(n + 1), level(n + 1), iter(n + 1) {}

    // 加一条 u -> v、容量为 cap 的边
    void add_edge(int u, int v, ll cap) {
        orig.push_back(OrigEdge{u, v, (int)g[u].size()});
        FlowEdge a{v, (int)g[v].size(), cap};
        FlowEdge b{u, (int)g[u].size(), 0};
        g[u].push_back(a);
        g[v].push_back(b);
    }

    // 求 s 到 t 的最大流
    ll maxflow(int s, int t) {
        ll flow = 0;
        while (bfs(s, t)) {
            std::fill(iter.begin(), iter.end(), 0);
            ll f;
            while ((f = dfs(s, t, INF)) > 0) {
                flow += f;
            }
        }
        return flow;
    }

    // 最小割边集：前提是已调用过 maxflow(s, t)。
    // 先 BFS 找残余网络中从 s 可达的顶点集合，再输出「可达 -> 不可达」且已饱和的原边。
    std::vector<std::pair<int, int>> min_cut_edges(int s, int t) {
        (void)t;  // t 仅用于语义提示，本实现不需要
        std::vector<char> reach(n + 1, 0);
        std::queue<int> q;
        reach[s] = 1;
        q.push(s);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (const FlowEdge& e : g[u]) {
                if (e.cap > 0 && !reach[e.to]) {
                    reach[e.to] = 1;
                    q.push(e.to);
                }
            }
        }
        std::vector<std::pair<int, int>> res;
        for (const OrigEdge& oe : orig) {
            // 原边从可达集到不可达集，且残余容量为 0（已饱和）
            if (reach[oe.u] && !reach[oe.v] && g[oe.u][oe.idx].cap == 0) {
                res.push_back(std::pair<int, int>(oe.u, oe.v));
            }
        }
        return res;
    }
};

}  // namespace algo
