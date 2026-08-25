#pragma once
// ============================================================================
// algo/ds/kdtree.hpp —— 二维 KD-Tree
// ----------------------------------------------------------------------------
// 功能：对二维点集建立 KD 树，支持：
//   - nearest_dist2(p)  查询离 p 最近点的距离平方；
//   - count_in_rect(...) 统计矩形（含边界）内点数。
// 时间复杂度：建树 O(n log n)（每层用 nth_element 找中位数）；
//            单次查询期望 O(sqrt(n))，最坏 O(n)。
// 空间复杂度：O(n)。
// 使用示例：
//   std::vector<Point2D> pts = {{0,0},{1,1},{3,2},{5,3}};
//   KDTree kd(pts);
//   ll d = kd.nearest_dist2(Point2D{3, 1});      // 4（最近点是 (1,1) 或 (3,2)）
//   int c = kd.count_in_rect(0, 0, 3, 2);        // 3（含边界）
// 注意事项：
//   - 距离平方使用 long long 计算；坐标绝对值过大（如 >3e9）可能溢出，请留意。
//   - 空点集：nearest_dist2 返回 INF，count_in_rect 返回 0。
//   - count_in_rect 要求 x1<=x2、y1<=y2；统计包含矩形边界上的点。
// ============================================================================

#include <algorithm>  // std::nth_element / std::max / std::min
#include <vector>

#include "algo/common.hpp"

namespace algo {

// 二维点
struct Point2D {
    ll x;
    ll y;
};

class KDTree {
public:
    // 以点集 pts 建树
    explicit KDTree(const std::vector<Point2D>& pts) : pts_(pts), root_(0) {
        int n = static_cast<int>(pts_.size());
        ord_.resize(n);
        for (int i = 0; i < n; ++i) ord_[i] = i;
        tree_.clear();
        tree_.push_back(KdNode{});  // 0 号空节点
        if (n > 0) root_ = build(0, n - 1, 0);
    }

    // 查询离 p 最近点的距离平方（空集返回 INF）
    ll nearest_dist2(const Point2D& p) const {
        if (root_ == 0) return INF;
        ll best = INF;
        query_nearest(root_, p, best);
        return best;
    }

    // 统计矩形 [x1,x2]×[y1,y2] 内的点数（含边界）
    int count_in_rect(ll x1, ll y1, ll x2, ll y2) const {
        return count_in_rect(root_, x1, y1, x2, y2);
    }

private:
    struct KdNode {
        int idx;             // 对应原 points 数组的下标
        int lc, rc;          // 左右儿子（0 表示空）
        int dim;             // 本节点切分的维度（0 按 x，1 按 y）
        ll minx, maxx;       // 子树包围盒
        ll miny, maxy;
        int sz;              // 子树点数

        KdNode()
            : idx(0), lc(0), rc(0), dim(0),
              minx(0), maxx(0), miny(0), maxy(0), sz(0) {}
    };

    std::vector<Point2D> pts_;  // 原始点
    std::vector<int> ord_;      // 建树过程中的下标排列（被 nth_element 打乱）
    std::vector<KdNode> tree_;
    int root_;

    int new_node(int idx, int dim) {
        KdNode nd{};
        nd.idx = idx;
        nd.lc = nd.rc = 0;
        nd.dim = dim;
        nd.minx = nd.maxx = pts_[idx].x;
        nd.miny = nd.maxy = pts_[idx].y;
        nd.sz = 1;
        tree_.push_back(nd);
        return static_cast<int>(tree_.size()) - 1;
    }

    // 汇总包围盒与子树大小
    void pull(int u) {
        int lc = tree_[u].lc;
        int rc = tree_[u].rc;
        tree_[u].minx = std::min(pts_[tree_[u].idx].x,
                                 std::min(lc ? tree_[lc].minx : pts_[tree_[u].idx].x,
                                          rc ? tree_[rc].minx : pts_[tree_[u].idx].x));
        tree_[u].maxx = std::max(pts_[tree_[u].idx].x,
                                 std::max(lc ? tree_[lc].maxx : pts_[tree_[u].idx].x,
                                          rc ? tree_[rc].maxx : pts_[tree_[u].idx].x));
        tree_[u].miny = std::min(pts_[tree_[u].idx].y,
                                 std::min(lc ? tree_[lc].miny : pts_[tree_[u].idx].y,
                                          rc ? tree_[rc].miny : pts_[tree_[u].idx].y));
        tree_[u].maxy = std::max(pts_[tree_[u].idx].y,
                                 std::max(lc ? tree_[lc].maxy : pts_[tree_[u].idx].y,
                                          rc ? tree_[rc].maxy : pts_[tree_[u].idx].y));
        tree_[u].sz = 1 + (lc ? tree_[lc].sz : 0) + (rc ? tree_[rc].sz : 0);
    }

    // 交替维度建树：按当前维度找中位数划分
    int build(int l, int r, int depth) {
        if (l > r) return 0;
        int mid = (l + r) >> 1;
        int dim = depth & 1;
        std::nth_element(ord_.begin() + l, ord_.begin() + mid, ord_.begin() + r + 1,
                         [&](int a, int b) {
                             return dim == 0 ? pts_[a].x < pts_[b].x
                                             : pts_[a].y < pts_[b].y;
                         });
        int u = new_node(ord_[mid], dim);
        // 先递归得到左右儿子，再写回 tree_[u]，避免 push_back 重分配后引用失效
        int lc = build(l, mid - 1, depth + 1);
        int rc = build(mid + 1, r, depth + 1);
        tree_[u].lc = lc;
        tree_[u].rc = rc;
        pull(u);
        return u;
    }

    ll dist2(int i, const Point2D& p) const {
        ll dx = pts_[i].x - p.x;
        ll dy = pts_[i].y - p.y;
        return dx * dx + dy * dy;
    }

    // 最近邻搜索（带剪枝）
    void query_nearest(int u, const Point2D& p, ll& best) const {
        if (!u) return;
        ll d = dist2(tree_[u].idx, p);
        if (d < best) best = d;

        // 先访问 p 更靠近的一侧
        int dim = tree_[u].dim;
        ll split = (dim == 0 ? pts_[tree_[u].idx].x : pts_[tree_[u].idx].y);
        ll delta = (dim == 0 ? p.x - split : p.y - split);
        int near = (delta <= 0) ? tree_[u].lc : tree_[u].rc;
        int far = (delta <= 0) ? tree_[u].rc : tree_[u].lc;
        query_nearest(near, p, best);

        // 剪枝：若到分割超平面的距离平方 >= 当前最优，则另一侧不可能更优。
        // 用 long double 避免 delta^2 溢出。
        long double dd = static_cast<long double>(delta) * static_cast<long double>(delta);
        if (dd < static_cast<long double>(best)) query_nearest(far, p, best);
    }

    // 矩形内点数统计（整棵剪枝）
    int count_in_rect(int u, ll x1, ll y1, ll x2, ll y2) const {
        if (!u) return 0;
        // 完全不相交：整棵剪掉
        if (tree_[u].maxx < x1 || tree_[u].minx > x2 ||
            tree_[u].maxy < y1 || tree_[u].miny > y2) {
            return 0;
        }
        // 完全在矩形内：整棵计入
        if (tree_[u].minx >= x1 && tree_[u].maxx <= x2 &&
            tree_[u].miny >= y1 && tree_[u].maxy <= y2) {
            return tree_[u].sz;
        }
        // 部分相交：检查当前点并递归
        int res = 0;
        ll px = pts_[tree_[u].idx].x;
        ll py = pts_[tree_[u].idx].y;
        if (px >= x1 && px <= x2 && py >= y1 && py <= y2) ++res;
        res += count_in_rect(tree_[u].lc, x1, y1, x2, y2);
        res += count_in_rect(tree_[u].rc, x1, y1, x2, y2);
        return res;
    }
};

}  // namespace algo
