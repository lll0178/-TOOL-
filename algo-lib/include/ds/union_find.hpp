#pragma once
// ============================================================================
// algo/ds/union_find.hpp —— 并查集三件套
// ----------------------------------------------------------------------------
// 功能：维护不相交集合的三种并查集变体：
//   1. DSU          普通并查集：路径压缩 + 按秩合并。
//   2. WeightedDSU  带权并查集：维护元素到根节点的"权值差"（偏移量），
//                    用于处理形如 val[a] - val[b] = w 的约束关系。
//   3. RollbackDSU  可撤销并查集：只按秩合并不做路径压缩，用历史栈支持回退。
// 时间复杂度：三种并查集的单次操作均为近似 O(α(n))（按秩合并 + 压缩）；
//            RollbackDSU 因不做路径压缩，单次 find 为 O(log n)，回退 O(1) 每步。
// 空间复杂度：O(n)。
// 使用示例：
//   DSU dsu(5);
//   dsu.unite(1, 2);
//   bool ok = dsu.same(1, 2);   // true
//   int sz = dsu.size(1);       // 2
//
//   WeightedDSU wdsu(5);                 // 食物链式偏移量，见下方注释
//   wdsu.unite(1, 2, 1);                 // val[1] - val[2] = 1
//   bool conflict = !wdsu.unite(2, 1, 1);// 与已有关系矛盾 → 返回 false
//
//   RollbackDSU rdsu(5);
//   rdsu.unite(1, 2);
//   int snap = rdsu.snapshot();
//   rdsu.unite(3, 4);
//   rdsu.rollback(snap);         // 撤销 (3,4) 的合并
// 注意事项：
//   - 所有编号均为 1..n，内部多开一位下标 0 不用。
//   - WeightedDSU 的权值做"累加"，适合取模（食物链 mod 3）或差值累加场景；
//     若权值会溢出 long long，请在调用前自行取模并保持一致性。
// ============================================================================

#include <utility>  // std::swap
#include <vector>

#include "algo/common.hpp"

namespace algo {

// ============================================================================
// DSU —— 普通并查集（路径压缩 + 按秩合并）
// ============================================================================
class DSU {
public:
    // 构造 n 个独立集合，编号 1..n
    explicit DSU(int n)
        : parent_(n + 1), rank_(n + 1, 0), sz_(n + 1, 1) {
        for (int i = 0; i <= n; ++i) parent_[i] = i;
    }

    // 查找 x 所在集合的根，并做路径压缩
    int find(int x) {
        if (parent_[x] != x) parent_[x] = find(parent_[x]);
        return parent_[x];
    }

    // 合并 a、b 所在集合；若本就在同一集合返回 false（未发生合并）
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        // 按秩合并：把秩小的根挂到秩大的根下
        if (rank_[a] < rank_[b]) std::swap(a, b);
        parent_[b] = a;
        if (rank_[a] == rank_[b]) ++rank_[a];
        sz_[a] += sz_[b];
        return true;
    }

    // 判断 a、b 是否在同一集合
    bool same(int a, int b) { return find(a) == find(b); }

    // 返回 x 所在集合的大小（元素个数）
    int size(int x) { return sz_[find(x)]; }

private:
    std::vector<int> parent_;  // 父节点（根指向自身）
    std::vector<int> rank_;    // 秩（近似树高，用于按秩合并）
    std::vector<int> sz_;      // 仅根节点维护：集合大小
};

// ============================================================================
// WeightedDSU —— 带权并查集（维护到根的偏移量）
// ----------------------------------------------------------------------------
// 约定：weight(x) 表示 val[x] - val[根]。unite(a, b, w) 声明 val[a]-val[b]=w，
//       若与既有关系矛盾则返回 false（不合并）。
// 食物链示例（POJ 1182）：
//   用 val[x] 表示 x 相对根的"种类偏移"，取模 3：
//     0 = 同类，1 = 吃根，2 = 被根吃。
//   若声明 "x 吃 y"，即 val[x] - val[y] ≡ 1 (mod 3)，调用 unite(x, y, 1)。
//   若声明 "x 与 y 同类"，调用 unite(x, y, 0)。
//   查关系：int d = ((weight(x) - weight(y)) % 3 + 3) % 3;  // 0/1/2
// ============================================================================
class WeightedDSU {
public:
    explicit WeightedDSU(int n)
        : parent_(n + 1), rank_(n + 1, 0), val_(n + 1, 0) {
        for (int i = 0; i <= n; ++i) parent_[i] = i;
    }

    // 查找根并压缩：压缩时把 val_[x] 累加成到根的偏移量
    int find(int x) {
        if (parent_[x] == x) return x;
        int root = find(parent_[x]);
        val_[x] += val_[parent_[x]];
        parent_[x] = root;
        return root;
    }

    // 返回 val[x] - val[根]（调用后 x 被压缩，直接读 val_[x]）
    ll weight(int x) {
        find(x);
        return val_[x];
    }

    // 声明 val[a] - val[b] = w；与已有关系矛盾时返回 false
    bool unite(int a, int b, ll w) {
        int ra = find(a);
        int rb = find(b);
        if (ra == rb) {
            // 同一集合：检查 val[a] - val[b] 是否恰为 w
            return val_[a] - val_[b] == w;
        }
        // 按秩合并，并设置新根的偏移量使关系成立
        // 推导：val[a]=val[ra]+val_[a]，val[b]=val[rb]+val_[b]，
        //   要求 val[a]-val[b]=w ⇒ val[ra]-val[rb] = w - val_[a] + val_[b]。
        if (rank_[ra] < rank_[rb]) {
            // 把 ra 挂到 rb 下：需要 val_[ra] = val[ra] - val[rb]
            parent_[ra] = rb;
            val_[ra] = val_[b] - val_[a] + w;
        } else {
            // 把 rb 挂到 ra 下：需要 val_[rb] = val[rb] - val[ra]
            parent_[rb] = ra;
            val_[rb] = val_[a] - val_[b] - w;
            if (rank_[ra] == rank_[rb]) ++rank_[ra];
        }
        return true;
    }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
    std::vector<ll> val_;  // val_[x] = val[x] - val[parent_[x]]（到父节点的差）
};

// ============================================================================
// RollbackDSU —— 可撤销并查集（按秩合并 + 历史栈，不做路径压缩）
// ============================================================================
class RollbackDSU {
public:
    explicit RollbackDSU(int n) : parent_(n + 1), rank_(n + 1, 0) {
        for (int i = 0; i <= n; ++i) parent_[i] = i;
    }

    // 查找根（不做路径压缩，保证可撤销）
    int find(int x) {
        while (parent_[x] != x) x = parent_[x];
        return x;
    }

    // 合并 a、b；若已同集合返回 false
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rank_[a] < rank_[b]) std::swap(a, b);
        // 把 b 挂到 a 下，并记录本次操作以便回退
        bool rank_inc = (rank_[a] == rank_[b]);
        if (rank_inc) ++rank_[a];
        parent_[b] = a;
        history_.push_back(RollbackOp{b, a, rank_inc});
        return true;
    }

    // 返回当前历史栈长度，作为回退点
    int snapshot() const { return static_cast<int>(history_.size()); }

    // 回退到快照 s：撤销 snapshot 之后的所有合并
    void rollback(int s) {
        while (static_cast<int>(history_.size()) > s) {
            RollbackOp op = history_.back();
            history_.pop_back();
            parent_[op.child] = op.child;      // 恢复 child 为自身根
            if (op.rank_inc) --rank_[op.parent];  // 撤销父节点秩的 +1
        }
    }

private:
    // 单次撤销所需的信息
    struct RollbackOp {
        int child;      // 被挂起的节点
        int parent;     // 挂起时的父节点（即合并时的根 a）
        bool rank_inc;  // 挂起时是否令父节点秩 +1
    };

    std::vector<int> parent_;
    std::vector<int> rank_;
    std::vector<RollbackOp> history_;
};

}  // namespace algo
