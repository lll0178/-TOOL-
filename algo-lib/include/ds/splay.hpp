#pragma once
// ============================================================================
// algo/ds/splay.hpp —— Splay（文艺平衡树 / 序列 Splay）
// ----------------------------------------------------------------------------
// 功能：维护一个序列，支持指定位置插入/删除、区间翻转、区间加、区间求和、
//       中序遍历导出。基于 Splay 的伸展性质实现均摊 O(log n) 的区间操作。
// 时间复杂度：所有操作均摊 O(log n)。
// 空间复杂度：O(n)（节点池数组）。
// 使用示例：
//   SplaySeq seq;
//   seq.build(std::vector<ll>{1,2,3,4,5});
//   seq.reverse(2, 4);         // {1,4,3,2,5}
//   seq.add(1, 3, 10);         // {11,14,13,2,5}
//   ll s = seq.query_sum(2, 4);// 29
// 注意事项：
//   - 所有位置参数均为 1-based。
//   - 0 号节点为哨兵；树中额外加入 -INF、+INF 两个边界节点，避免越界特判。
//   - 区间提取方法：splay(第 l 个) 到根，再 splay(第 r+2 个) 到根的右儿子，
//     其左儿子即为 [l, r]（详见实现注释）。
//   - 懒标记下推顺序：先 rev（翻转，交换左右儿子）再 add（加）。
//     二者对 sum 可交换，顺序不影响最终结果。
// ============================================================================

#include <utility>  // std::swap
#include <vector>

#include "algo/common.hpp"

namespace algo {

class SplaySeq {
public:
    SplaySeq() {
        t_.clear();
        t_.push_back(Node());  // 0 号哨兵
        root_ = 0;
        sent_l_ = new_node(-INF, 0);
        sent_r_ = new_node(INF, sent_l_);
        t_[sent_l_].ch[1] = sent_r_;
        root_ = sent_l_;
        pushup(root_);
    }

    // 由数组 a 建序列（依次在末尾插入）
    void build(const std::vector<ll>& a) {
        int n = static_cast<int>(a.size());
        for (int i = 0; i < n; ++i) insert(i + 1, a[i]);
    }

    // 在位置 pos 插入 v（1-based，最终成为第 pos 个元素）
    void insert(int pos, ll v) {
        // 提出空位：第 pos 个节点与第 pos+1 个节点之间
        int x = kth_node(pos);
        splay(x, 0);
        int y = kth_node(pos + 1);
        splay(y, root_);
        int u = new_node(v, t_[root_].ch[1]);
        t_[t_[root_].ch[1]].ch[0] = u;
        pushup(t_[root_].ch[1]);
        pushup(root_);
    }

    // 删除位置 pos 的元素（1-based）
    void erase(int pos) {
        int x = kth_node(pos);
        splay(x, 0);
        int y = kth_node(pos + 2);
        splay(y, root_);
        // 第 pos 个元素就是 root 右儿子的左儿子，摘除之
        t_[t_[root_].ch[1]].ch[0] = 0;
        pushup(t_[root_].ch[1]);
        pushup(root_);
    }

    // 翻转区间 [l, r]（1-based）
    void reverse(int l, int r) {
        int x = kth_node(l);
        splay(x, 0);
        int y = kth_node(r + 2);
        splay(y, root_);
        apply_rev(t_[t_[root_].ch[1]].ch[0]);
    }

    // 区间加：a[l..r] 各 += v（1-based）
    void add(int l, int r, ll v) {
        int x = kth_node(l);
        splay(x, 0);
        int y = kth_node(r + 2);
        splay(y, root_);
        apply_add(t_[t_[root_].ch[1]].ch[0], v);
    }

    // 区间求和（1-based）
    ll query_sum(int l, int r) {
        int x = kth_node(l);
        splay(x, 0);
        int y = kth_node(r + 2);
        splay(y, root_);
        return t_[t_[t_[root_].ch[1]].ch[0]].sum;
    }

    // 中序遍历导出整个序列（不含 -INF/+INF 边界）
    std::vector<ll> inorder() {
        std::vector<ll> res;
        inorder(root_, res);
        return res;
    }

private:
    struct Node {
        int ch[2];   // 左右儿子
        int fa;      // 父亲
        ll val;      // 节点值
        ll sum;      // 子树和
        ll add;      // 懒标记：加
        int rev;     // 懒标记：翻转
        int sz;      // 子树大小（含本节点）

        Node() : fa(0), val(0), sum(0), add(0), rev(0), sz(0) {
            ch[0] = ch[1] = 0;
        }
    };

    std::vector<Node> t_;  // 节点池（0 号为哨兵）
    int root_;
    int sent_l_;  // 左边界节点（-INF）
    int sent_r_;  // 右边界节点（+INF）

    int new_node(ll v, int fa) {
        Node nd;
        nd.fa = fa;
        nd.val = nd.sum = v;
        nd.sz = 1;
        t_.push_back(nd);
        return static_cast<int>(t_.size()) - 1;
    }

    void pushup(int x) {
        t_[x].sz = t_[t_[x].ch[0]].sz + t_[t_[x].ch[1]].sz + 1;
        t_[x].sum = t_[t_[x].ch[0]].sum + t_[t_[x].ch[1]].sum + t_[x].val;
    }

    void apply_add(int x, ll v) {
        if (!x) return;
        t_[x].val += v;
        t_[x].sum += v * t_[x].sz;
        t_[x].add += v;
    }

    void apply_rev(int x) {
        if (!x) return;
        std::swap(t_[x].ch[0], t_[x].ch[1]);
        t_[x].rev ^= 1;
    }

    // 下推懒标记：先 rev 后 add（对 sum 可交换，顺序无影响）
    void pushdown(int x) {
        if (t_[x].rev) {
            apply_rev(t_[x].ch[0]);
            apply_rev(t_[x].ch[1]);
            t_[x].rev = 0;
        }
        if (t_[x].add != 0) {
            apply_add(t_[x].ch[0], t_[x].add);
            apply_add(t_[x].ch[1], t_[x].add);
            t_[x].add = 0;
        }
    }

    // 判断 x 是否为父亲的右儿子
    int is_right(int x) const { return t_[t_[x].fa].ch[1] == x; }

    void rotate(int x) {
        int y = t_[x].fa;
        int z = t_[y].fa;
        int k = is_right(x);        // x 在 y 的哪一侧
        int kz = is_right(y);       // y 在 z 的哪一侧（必须在修改 fa 前计算）
        int b = t_[x].ch[k ^ 1];    // x 的"对侧"儿子需要接到 y 下
        t_[y].ch[k] = b;
        if (b) t_[b].fa = y;
        t_[x].ch[k ^ 1] = y;
        t_[y].fa = x;
        t_[x].fa = z;
        if (z) t_[z].ch[kz] = x;
        pushup(y);
        pushup(x);
    }

    // 将 x 旋转到 goal 的儿子位置（goal == 0 表示旋到根）
    void splay(int x, int goal) {
        while (t_[x].fa != goal) {
            int y = t_[x].fa;
            int z = t_[y].fa;
            if (z != goal) {
                // 一字型先旋 y，之字型先旋 x
                if (is_right(x) == is_right(y)) rotate(y);
                else rotate(x);
            }
            rotate(x);
        }
        if (goal == 0) root_ = x;
    }

    // 定位中序第 k 个节点（1-based），不旋转
    int kth_node(int k) {
        int u = root_;
        while (true) {
            pushdown(u);
            int lsz = t_[t_[u].ch[0]].sz;
            if (k <= lsz) {
                u = t_[u].ch[0];
            } else if (k == lsz + 1) {
                return u;
            } else {
                k -= lsz + 1;
                u = t_[u].ch[1];
            }
        }
    }

    void inorder(int u, std::vector<ll>& out) {
        if (!u) return;
        pushdown(u);
        inorder(t_[u].ch[0], out);
        if (u != sent_l_ && u != sent_r_) out.push_back(t_[u].val);
        inorder(t_[u].ch[1], out);
    }
};

}  // namespace algo
