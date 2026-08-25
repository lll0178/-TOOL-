#pragma once
// ============================================================================
// algo/ds/lct.hpp —— Link-Cut Tree（动态树）
// ----------------------------------------------------------------------------
// 功能：维护一片森林，支持加边、删边、换根、查根、路径（点权）求和、
//       单点赋值。基于 Splay 维护实链 + 虚边连接实现。
// 时间复杂度：所有操作均摊 O(log n)。
// 空间复杂度：O(n)。
// 使用示例：
//   LinkCutTree lct(5);
//   lct.set_val(1, 10);
//   lct.link(1, 2);
//   lct.link(2, 3);
//   ll s = lct.query_path(1, 3);  // 10（点权 10+0+0）
//   lct.cut(1, 2);                // 断开边 (1,2)
//   int r = lct.findroot(3);      // 3 所在树的根
// 注意事项：
//   - 节点编号 1..n；0 号为哨兵节点。
//   - link(x, y) 前内部会先 makeroot(x)，并避免成环；cut(x, y) 仅当 x、y 直接相连才断开。
//   - 路径查询 query_path 维护的是"路径上点权之和"。
// ============================================================================

#include <utility>  // std::swap
#include <vector>

#include "algo/common.hpp"

namespace algo {

class LinkCutTree {
public:
    explicit LinkCutTree(int n) : n_(n), t_(n + 1) {
        // t_[0] 为哨兵；其余节点 val=sum=0
    }

    // 把 x 变为其所在树的根（换根）
    void makeroot(int x) {
        access(x);
        pushrev(x);
    }

    // 查找 x 所在树的根
    int findroot(int x) {
        access(x);
        while (t_[x].ch[0]) {
            pushdown(x);
            x = t_[x].ch[0];
        }
        splay(x);  // 把根旋回辅助树根，摊还复杂度
        return x;
    }

    // 连接边 (x, y)（若已连通则不操作）
    void link(int x, int y) {
        makeroot(x);
        if (findroot(y) != x) t_[x].fa = y;
    }

    // 断开边 (x, y)（仅当二者直接相连）
    void cut(int x, int y) {
        makeroot(x);
        access(y);
        // 此时 x 应为 y 的左儿子且无右儿子（即边直接相连）
        if (t_[y].ch[0] == x && t_[x].ch[1] == 0) {
            t_[y].ch[0] = 0;
            t_[x].fa = 0;
            pushup(y);
        }
    }

    // 单点赋值：节点 x 权值设为 v
    void set_val(int x, ll v) {
        access(x);
        t_[x].val = v;
        pushup(x);
    }

    // 查询路径 x->y 上所有节点权值之和
    ll query_path(int x, int y) {
        split(x, y);
        return t_[y].sum;
    }

private:
    struct LctNode {
        int ch[2];  // 辅助树（Splay）左右儿子
        int fa;     // 父亲（辅助树内为 splay 父亲；虚边则为树父亲）
        int rev;    // 翻转懒标记
        ll val;     // 点权
        ll sum;     // 辅助树子树和

        LctNode() : fa(0), rev(0), val(0), sum(0) {
            ch[0] = ch[1] = 0;
        }
    };

    int n_;
    std::vector<LctNode> t_;

    // x 是否为其辅助树（Splay）的根（父亲不直接指向 x）
    bool isroot(int x) const {
        return t_[t_[x].fa].ch[0] != x && t_[t_[x].fa].ch[1] != x;
    }

    // x 是否为其父亲的右儿子
    bool is_right(int x) const { return t_[t_[x].fa].ch[1] == x; }

    void pushup(int x) {
        t_[x].sum = t_[t_[x].ch[0]].sum + t_[t_[x].ch[1]].sum + t_[x].val;
    }

    // 翻转：交换左右儿子并打标记（代表实链方向反转）
    void pushrev(int x) {
        std::swap(t_[x].ch[0], t_[x].ch[1]);
        t_[x].rev ^= 1;
    }

    void pushdown(int x) {
        if (t_[x].rev) {
            if (t_[x].ch[0]) pushrev(t_[x].ch[0]);
            if (t_[x].ch[1]) pushrev(t_[x].ch[1]);
            t_[x].rev = 0;
        }
    }

    void rotate(int x) {
        int y = t_[x].fa;
        int z = t_[y].fa;
        int k = is_right(x);       // x 在 y 的哪一侧
        int b = t_[x].ch[k ^ 1];   // x 的对侧儿子，转给 y
        if (!isroot(y)) t_[z].ch[is_right(y)] = x;
        t_[x].fa = z;
        t_[x].ch[k ^ 1] = y;
        t_[y].fa = x;
        t_[y].ch[k] = b;
        if (b) t_[b].fa = y;
        pushup(y);
        pushup(x);
    }

    void splay(int x) {
        // 先把根到 x 路径上的翻转标记全部下推
        std::vector<int> st;
        int y = x;
        st.push_back(y);
        while (!isroot(y)) {
            y = t_[y].fa;
            st.push_back(y);
        }
        for (int i = static_cast<int>(st.size()) - 1; i >= 0; --i) pushdown(st[i]);

        while (!isroot(x)) {
            y = t_[x].fa;
            if (!isroot(y)) {
                if (is_right(x) == is_right(y)) rotate(y);  // 一字型
                else rotate(x);                              // 之字型
            }
            rotate(x);
        }
        pushup(x);
    }

    // 建立 x 到根（其所在树根）的实路径，并把 x 旋到辅助树根
    void access(int x) {
        int last = 0;
        for (int y = x; y; y = t_[y].fa) {
            splay(y);
            t_[y].ch[1] = last;  // 把右侧换成新的实链
            pushup(y);
            last = y;
        }
        splay(x);
    }

    // 提取路径 x->y：makeroot(x) 后 access(y)，y 的辅助树子树即整条路径
    void split(int x, int y) {
        makeroot(x);
        access(y);
    }
};

}  // namespace algo
