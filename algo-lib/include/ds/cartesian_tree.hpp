#pragma once
// ============================================================================
// algo/ds/cartesian_tree.hpp —— 笛卡尔树
// ----------------------------------------------------------------------------
// 功能：O(n) 建立小根笛卡尔树：每个节点对应原数组一个位置，满足
//   - 中序遍历恰好是原数组顺序；
//   - 堆性质：父节点值 <= 左右儿子值（小根），相等时下标小者更靠近根。
// 时间复杂度：O(n)（单调栈，每个元素恰好入栈、出栈一次）。
// 空间复杂度：O(n)。
// 使用示例：
//   std::vector<int> a = {5, 3, 7, 1, 6};
//   auto tree = build_cartesian_tree(a);
//   // tree[i].l/.r/.fa 分别为节点 i 的左/右儿子/父亲（0-based 下标，无则 -1）
//   // 根节点 fa == -1；此处根为下标 3（值 1）。
//   // 区间最值应用：a[l..r] 的最小值即 lca(l, r)（笛卡尔树上）对应的值。
// 注意事项：
//   - 返回 vector 大小 == a.size()，节点编号与原下标一致（0..n-1）。
//   - 空数组返回空 vector。
//   - a 小者优先级高（小根）；相等时下标小者优先级高（下标小者为祖先）。
// ============================================================================

#include <vector>

namespace algo {

// 笛卡尔树节点：l/r 为左右儿子下标，fa 为父亲下标，均以 -1 表示不存在
template <typename T>
struct CartesianNode {
    int l;
    int r;
    int fa;
};

// 建小根笛卡尔树，返回节点数组（下标 0..n-1）
template <typename T>
std::vector<CartesianNode<T>> build_cartesian_tree(const std::vector<T>& a) {
    int n = static_cast<int>(a.size());
    std::vector<CartesianNode<T>> tree(n);
    if (n == 0) return tree;

    for (int i = 0; i < n; ++i) {
        tree[i].l = tree[i].r = tree[i].fa = -1;
    }

    // 单调栈：栈内元素的值严格递增（自底向上），即维护"最右侧链"
    std::vector<int> st;
    st.reserve(n);

    for (int i = 0; i < n; ++i) {
        int last = -1;
        // 弹出所有比 a[i] 大的元素（小根：更小的应成为祖先）
        // 使用严格大于，保证相等时下标小者（先入栈）不被弹出，成为祖先。
        while (!st.empty() && a[st.back()] > a[i]) {
            last = st.back();
            st.pop_back();
        }
        // 最后弹出的那个元素（连同其子树）成为 i 的左儿子
        if (last != -1) {
            tree[i].l = last;
            tree[last].fa = i;
        }
        // 栈顶（值 <= a[i]）成为 i 的父亲，i 是其右儿子
        if (!st.empty()) {
            tree[i].fa = st.back();
            tree[st.back()].r = i;
        }
        st.push_back(i);
    }

    return tree;
}

}  // namespace algo
