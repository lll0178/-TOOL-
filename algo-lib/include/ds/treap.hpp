#pragma once
// ============================================================================
// algo/ds/treap.hpp —— FHQ Treap（无旋 Treap）
// ----------------------------------------------------------------------------
// 功能：
//   1. FHQTreap  按值分裂的有序集合：支持插入、删除、查排名、查第 k 小、前驱、后继，
//                重复值用节点内 cnt 计数。
//   2. TreapSeq   按大小分裂的序列（文艺平衡树）：支持指定位置插入/删除、
//                区间翻转、区间加、区间求和、中序遍历导出。
// 时间复杂度：所有操作期望 O(log n)（依赖随机优先级）。
// 空间复杂度：O(n)。
// 使用示例：
//   FHQTreap treap;
//   treap.insert(5); treap.insert(3); treap.insert(5);
//   int r = treap.rank(5);      // 2（小于 5 的元素个数 +1）
//   ll k = treap.kth(2);        // 5
//   ll p = treap.prev(5);       // 3
//   ll nxt = treap.next(3);     // 5
//
//   TreapSeq seq;
//   seq.build(std::vector<ll>{1,2,3,4,5});
//   seq.reverse(2, 4);          // {1,4,3,2,5}
//   seq.add(1, 3, 10);          // {11,14,13,2,5}
//   ll s = seq.query_sum(1, 5); // 45
// 注意事项：
//   - FHQTreap 的 rank/kth 均为 1-based；prev/next 不存在时返回 -INF / INF。
//   - TreapSeq 所有位置参数均为 1-based。
//   - 懒标记 rev（翻转）与 add（加）可交换作用于 sum，下推时先 rev 后 add（或反之均可），
//     本实现统一"先 rev 后 add"，见 pushdown 注释。
// ============================================================================

#include <chrono>   // 随机种子
#include <random>   // std::mt19937
#include <utility>  // std::swap
#include <vector>

#include "algo/common.hpp"

namespace algo {

// ============================================================================
// FHQTreap —— 按值分裂的平衡树（重复值计数）
// ============================================================================
class FHQTreap {
public:
    FHQTreap() : root_(0), node_(1) {
        // node_[0] 作为空节点占位
        node_[0] = FhqNode{0, 0, 0, 0, 0, 0};
        rng_.seed(static_cast<unsigned int>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    }

    // 插入 x（已存在则计数 +1）
    void insert(ll x) {
        int a, b, c, d;
        split_val(root_, x, a, b);       // a: < x, b: >= x
        split_val(b, x + 1, c, d);       // c: == x, d: > x
        if (c) {
            ++node_[c].cnt;
            pull(c);
        } else {
            c = new_node(x);
        }
        root_ = merge(merge(a, c), d);
    }

    // 删除 x（计数 -1，减到 0 时移除节点）
    void erase(ll x) {
        int a, b, c, d;
        split_val(root_, x, a, b);
        split_val(b, x + 1, c, d);
        if (c) {
            --node_[c].cnt;
            if (node_[c].cnt == 0) c = merge(node_[c].lc, node_[c].rc);  // 丢弃该节点
            else pull(c);
        }
        root_ = merge(merge(a, c), d);
    }

    // 查询 x 的排名：小于 x 的元素个数 + 1（1-based）
    int rank(ll x) {
        int a, b;
        split_val(root_, x, a, b);
        int ans = size_of(a) + 1;
        root_ = merge(a, b);
        return ans;
    }

    // 查询第 k 小（1-based）
    ll kth(int k) { return kth_in(root_, k); }

    // 前驱：小于 x 的最大元素；不存在返回 -INF
    ll prev(ll x) {
        int a, b;
        split_val(root_, x, a, b);
        ll res = a ? kth_in(a, size_of(a)) : -INF;
        root_ = merge(a, b);
        return res;
    }

    // 后继：大于 x 的最小元素；不存在返回 INF
    ll next(ll x) {
        int a, b;
        split_val(root_, x + 1, a, b);  // a: <= x, b: > x
        ll res = b ? kth_in(b, 1) : INF;
        root_ = merge(a, b);
        return res;
    }

    // 元素个数
    int size() const { return size_of(root_); }

private:
    struct FhqNode {
        ll key;          // 键值
        unsigned int prio;  // 随机优先级
        int sz;          // 子树元素总数（含 cnt）
        int cnt;         // 本节点重复次数
        int lc, rc;      // 左右儿子下标
    };

    int root_;
    std::vector<FhqNode> node_;
    std::mt19937 rng_;

    int new_node(ll x) {
        FhqNode nd{x, rng_(), 1, 1, 0, 0};
        node_.push_back(nd);
        return static_cast<int>(node_.size()) - 1;
    }

    int size_of(int u) const { return u ? node_[u].sz : 0; }

    void pull(int u) {
        node_[u].sz = size_of(node_[u].lc) + size_of(node_[u].rc) + node_[u].cnt;
    }

    // 按值分裂：键 < key 进 a，键 >= key 进 b
    void split_val(int u, ll key, int& a, int& b) {
        if (!u) {
            a = b = 0;
            return;
        }
        if (node_[u].key < key) {
            split_val(node_[u].rc, key, node_[u].rc, b);
            a = u;
        } else {
            split_val(node_[u].lc, key, a, node_[u].lc);
            b = u;
        }
        pull(u);
    }

    // 合并两棵 treap（a 中所有键 < b 中所有键），按优先级大根堆
    int merge(int a, int b) {
        if (!a || !b) return a ? a : b;
        if (node_[a].prio > node_[b].prio) {
            node_[a].rc = merge(node_[a].rc, b);
            pull(a);
            return a;
        } else {
            node_[b].lc = merge(a, node_[b].lc);
            pull(b);
            return b;
        }
    }

    // 在子树 u 中找第 k 小（1-based），返回键值
    ll kth_in(int u, int k) const {
        while (u) {
            int lsz = size_of(node_[u].lc);
            if (k <= lsz) {
                u = node_[u].lc;
            } else if (k <= lsz + node_[u].cnt) {
                return node_[u].key;
            } else {
                k -= lsz + node_[u].cnt;
                u = node_[u].rc;
            }
        }
        return INF;  // 越界兜底
    }
};

// ============================================================================
// TreapSeq —— 按大小分裂的序列 Treap（区间操作，1-based）
// ============================================================================
class TreapSeq {
public:
    TreapSeq() : root_(0), node_(1) {
        node_[0] = TreapSeqNode{0, 0, 0, 0, 0, 0, 0, 0};
        rng_.seed(static_cast<unsigned int>(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    }

    // 由数组 a 建序列（按顺序依次合并）
    void build(const std::vector<ll>& a) {
        root_ = 0;
        for (ll v : a) root_ = merge(root_, new_node(v));
    }

    // 在位置 pos 插入 v（1-based，插到第 pos 个元素之前，最终成为第 pos 个）
    void insert(int pos, ll v) {
        int a, b;
        split_size(root_, pos - 1, a, b);
        root_ = merge(merge(a, new_node(v)), b);
    }

    // 删除位置 pos 的元素（1-based）
    void erase(int pos) {
        int a, b, c, d;
        split_size(root_, pos - 1, a, b);
        split_size(b, 1, c, d);
        root_ = merge(a, d);
    }

    // 翻转区间 [l, r]（1-based）
    void reverse(int l, int r) {
        int a, b, c, d;
        split_size(root_, l - 1, a, b);
        split_size(b, r - l + 1, c, d);
        apply_rev(c);
        root_ = merge(merge(a, c), d);
    }

    // 区间加：a[l..r] 各 += v（1-based）
    void add(int l, int r, ll v) {
        int a, b, c, d;
        split_size(root_, l - 1, a, b);
        split_size(b, r - l + 1, c, d);
        apply_add(c, v);
        root_ = merge(merge(a, c), d);
    }

    // 区间求和（1-based）
    ll query_sum(int l, int r) {
        int a, b, c, d;
        split_size(root_, l - 1, a, b);
        split_size(b, r - l + 1, c, d);
        ll res = node_[c].sum;
        root_ = merge(merge(a, c), d);
        return res;
    }

    // 中序遍历导出整个序列
    std::vector<ll> inorder() {
        std::vector<ll> res;
        inorder(root_, res);
        return res;
    }

private:
    struct TreapSeqNode {
        ll val;
        ll sum;          // 子树元素和
        ll add;          // 懒标记：加
        int rev;         // 懒标记：翻转
        unsigned int prio;
        int sz;
        int lc, rc;
    };

    int root_;
    std::vector<TreapSeqNode> node_;
    std::mt19937 rng_;

    int new_node(ll v) {
        TreapSeqNode nd{v, v, 0, 0, rng_(), 1, 0, 0};
        node_.push_back(nd);
        return static_cast<int>(node_.size()) - 1;
    }

    int size_of(int u) const { return u ? node_[u].sz : 0; }
    ll sum_of(int u) const { return u ? node_[u].sum : 0; }

    void pull(int u) {
        node_[u].sz = size_of(node_[u].lc) + size_of(node_[u].rc) + 1;
        node_[u].sum = sum_of(node_[u].lc) + sum_of(node_[u].rc) + node_[u].val;
    }

    void apply_add(int u, ll v) {
        if (!u) return;
        node_[u].val += v;
        node_[u].sum += v * node_[u].sz;
        node_[u].add += v;
    }

    void apply_rev(int u) {
        if (!u) return;
        std::swap(node_[u].lc, node_[u].rc);
        node_[u].rev ^= 1;
    }

    // 下推懒标记：add 对所有元素对称、rev 只交换子树，二者可交换作用；
    // 统一"先 rev 后 add"（先换子树再给子树加），顺序不影响最终结果。
    void pushdown(int u) {
        if (node_[u].rev) {
            apply_rev(node_[u].lc);
            apply_rev(node_[u].rc);
            node_[u].rev = 0;
        }
        if (node_[u].add != 0) {
            apply_add(node_[u].lc, node_[u].add);
            apply_add(node_[u].rc, node_[u].add);
            node_[u].add = 0;
        }
    }

    // 按大小分裂：前 k 个进 a，其余进 b
    void split_size(int u, int k, int& a, int& b) {
        if (!u) {
            a = b = 0;
            return;
        }
        pushdown(u);
        if (size_of(node_[u].lc) >= k) {
            split_size(node_[u].lc, k, a, node_[u].lc);
            b = u;
            pull(b);
        } else {
            split_size(node_[u].rc, k - size_of(node_[u].lc) - 1, node_[u].rc, b);
            a = u;
            pull(a);
        }
    }

    int merge(int a, int b) {
        if (!a || !b) return a ? a : b;
        if (node_[a].prio > node_[b].prio) {
            pushdown(a);
            node_[a].rc = merge(node_[a].rc, b);
            pull(a);
            return a;
        } else {
            pushdown(b);
            node_[b].lc = merge(a, node_[b].lc);
            pull(b);
            return b;
        }
    }

    void inorder(int u, std::vector<ll>& out) {
        if (!u) return;
        pushdown(u);
        inorder(node_[u].lc, out);
        out.push_back(node_[u].val);
        inorder(node_[u].rc, out);
    }
};

}  // namespace algo
