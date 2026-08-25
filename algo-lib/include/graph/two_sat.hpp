#pragma once
// ============================================================================
// graph/two_sat.hpp —— 2-SAT（基于强连通分量）
// ----------------------------------------------------------------------------
// 功能：判断 n 个布尔变量（编号 1..n）的一组「或子句」是否可同时满足，并给出赋值。
//   子句形式：(a == va) OR (b == vb)，其中 va、vb 为布尔值。
//   使用蕴含图 + 强连通分量求解（Tarjan）。
// 时间复杂度：O(n + m)，m 为子句数。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "graph/two_sat.hpp"
//   using namespace algo;
//   TwoSAT sat(2);                       // 2 个变量
//   sat.add_clause(1, true,  2, true);   // (x1) OR (x2)
//   sat.add_clause(1, false, 2, true);   // (¬x1) OR (x2)
//   bool ok = sat.solve();               // true
//   const vector<bool>& ans = sat.assignment();  // ans[1]、ans[2]
// 注意事项：
//   1) 节点编号约定：变量 x（1..n）对应两个节点——
//        node(x, true)  = 2x - 1（x 为真）
//        node(x, false) = 2x     （x 为假）
//      共 2n 个节点（编号 1..2n）。
//   2) 子句 (a==va) OR (b==vb) 等价于两条蕴含边：
//        ¬(a==va) -> (b==vb)  与  ¬(b==vb) -> (a==va)。
//   3) 若 x 与其否定 ¬x 落在同一 SCC，则不可满足（solve 返回 false）。
//   4) assignment() 需在 solve() 返回 true 之后调用；返回向量下标 1..n。
// ============================================================================

#include "graph/tarjan.hpp"

#include <vector>

namespace algo {

class TwoSAT {
private:
    int n;                        // 变量个数（编号 1..n）
    SCC scc;                      // 蕴含图（2n 个节点）
    std::vector<bool> answer;     // 每个变量的取值（solve 后有效，下标 1..n）

    // 节点编号：变量 x（1..n）的「真」节点为 2x-1，「假」节点为 2x。
    int node_id(int x, bool value) const { return 2 * x - (value ? 1 : 0); }

public:
    explicit TwoSAT(int n) : n(n), scc(2 * n), answer(n + 1, false) {}

    // 添加子句 (a == va) OR (b == vb)
    void add_clause(int a, bool va, int b, bool vb) {
        scc.add_edge(node_id(a, !va), node_id(b, vb));
        scc.add_edge(node_id(b, !vb), node_id(a, va));
    }

    // 判断是否可满足；可满足时同时计算好赋值。
    bool solve() {
        scc.run();
        for (int x = 1; x <= n; ++x) {
            if (scc.id()[node_id(x, true)] == scc.id()[node_id(x, false)]) {
                return false;  // x 与 ¬x 在同一 SCC，矛盾
            }
        }
        // Tarjan 的 SCC 编号满足：DAG 边从编号大的分量指向编号小的分量（反向拓扑序）。
        // 取编号更小的节点为真（该侧在拓扑序上更「下游」），可保证所有蕴含关系成立。
        for (int x = 1; x <= n; ++x) {
            answer[x] = scc.id()[node_id(x, true)] < scc.id()[node_id(x, false)];
        }
        return true;
    }

    // 返回每个变量的取值（下标 1..n；需先调用 solve 且其返回 true）
    const std::vector<bool>& assignment() const { return answer; }
};

}  // namespace algo
