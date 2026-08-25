#pragma once
// ============================================================================
// algo/ds/segment_tree.hpp —— 线段树（求和/最值 + lazy、最大子段和、矩形面积并）
// ----------------------------------------------------------------------------
// 功能：
//   1. SegTreeSum          支持区间加、区间赋值、区间求和/最值的懒标记线段树。
//   2. SegTreeMaxSubarray  单点赋值、区间最大子段和查询。
//   3. rectangle_area_union 扫描线求若干矩形面积并（坐标离散化）。
// 时间复杂度：
//   SegTreeSum / SegTreeMaxSubarray：单次操作 O(log n)。
//   rectangle_area_union：O(k log k)，k 为矩形个数。
// 空间复杂度：SegTreeSum/SegTreeMaxSubarray O(n)（4n 数组）；
//            rectangle_area_union O(k)。
// 使用示例：
//   SegTreeSum st(std::vector<ll>{1,2,3,4,5});
//   st.range_add(1, 3, 10);
//   ll s = st.query_sum(1, 5);   // 45
//
//   SegTreeMaxSubarray msa(std::vector<ll>{-2,1,-3,4,-1,2,1,-5,4});
//   auto node = msa.query(1, 9); // node.best == 6
//
//   std::vector<Rect> rs = {{0,0,2,2},{1,1,3,3}};
//   ll area = rectangle_area_union(rs);  // 7
// 注意事项：
//   - SegTreeSum 的两个懒标记（add / set）下推顺序：先 set 后 add（见实现注释）。
//   - SegTreeSum 区间赋值会把该区间的 add 清零，再叠加后续 add。
//   - rectangle_area_union 的 Rect 要求 x1<x2、y1<y2；返回 long long。
// ============================================================================

#include <algorithm>  // std::sort / std::unique / std::max / std::min
#include <utility>    // std::swap
#include <vector>

#include "algo/common.hpp"

namespace algo {

// ============================================================================
// SegTreeSum —— 区间加 + 区间赋值 + 区间求和/最值（懒标记，数组实现，1-based）
// ----------------------------------------------------------------------------
// 每个节点维护：sum（区间和）、mx（最大值）、mn（最小值）。
// 懒标记：addv（加）、setv（赋值）；hasset 标记该节点是否带未下推的赋值。
// 下推顺序：赋值会清空 add，因此先下推 set 再下推 add。
// ============================================================================
class SegTreeSum {
public:
    // 构造一个大小为 n 的全 0 线段树
    explicit SegTreeSum(int n) { init(n); }

    // 由数组 a（下标 0 起）建树
    explicit SegTreeSum(const std::vector<ll>& a) {
        init(static_cast<int>(a.size()));
        build(1, 1, n_, a);
    }

    // 区间加：a[l..r] 各 += v
    void range_add(int l, int r, ll v) { range_add(1, 1, n_, l, r, v); }

    // 区间赋值：a[l..r] 全部置为 v
    void range_set(int l, int r, ll v) { range_set(1, 1, n_, l, r, v); }

    // 区间求和
    ll query_sum(int l, int r) { return query_sum(1, 1, n_, l, r); }

    // 区间最大值
    ll query_max(int l, int r) { return query_max(1, 1, n_, l, r); }

    // 区间最小值
    ll query_min(int l, int r) { return query_min(1, 1, n_, l, r); }

private:
    int n_;
    std::vector<ll> sum_, mx_, mn_;       // 节点信息
    std::vector<ll> add_, set_;           // 懒标记
    std::vector<char> has_set_;           // 是否有待下推的赋值

    void init(int n) {
        n_ = n;
        int cap = 4 * n + 5;
        sum_.assign(cap, 0);
        mx_.assign(cap, 0);
        mn_.assign(cap, 0);
        add_.assign(cap, 0);
        set_.assign(cap, 0);
        has_set_.assign(cap, 0);
    }

    // 自下而上汇总
    void pull(int p) {
        sum_[p] = sum_[p * 2] + sum_[p * 2 + 1];
        mx_[p] = std::max(mx_[p * 2], mx_[p * 2 + 1]);
        mn_[p] = std::min(mn_[p * 2], mn_[p * 2 + 1]);
    }

    // 对节点 p（覆盖 [l,r]）施加"加 v"
    void apply_add(int p, int l, int r, ll v) {
        add_[p] += v;
        sum_[p] += v * (r - l + 1);
        mx_[p] += v;
        mn_[p] += v;
    }

    // 对节点 p（覆盖 [l,r]）施加"赋值 v"
    void apply_set(int p, int l, int r, ll v) {
        set_[p] = v;
        has_set_[p] = 1;
        add_[p] = 0;  // 赋值覆盖旧的加标记
        sum_[p] = v * (r - l + 1);
        mx_[p] = mn_[p] = v;
    }

    // 下推懒标记（先 set 后 add）
    void pushdown(int p, int l, int r) {
        if (l == r) return;
        int mid = (l + r) >> 1;
        if (has_set_[p]) {
            apply_set(p * 2, l, mid, set_[p]);
            apply_set(p * 2 + 1, mid + 1, r, set_[p]);
            has_set_[p] = 0;
        }
        if (add_[p] != 0) {
            apply_add(p * 2, l, mid, add_[p]);
            apply_add(p * 2 + 1, mid + 1, r, add_[p]);
            add_[p] = 0;
        }
    }

    void build(int p, int l, int r, const std::vector<ll>& a) {
        if (l == r) {
            sum_[p] = mx_[p] = mn_[p] = a[l - 1];
            return;
        }
        int mid = (l + r) >> 1;
        build(p * 2, l, mid, a);
        build(p * 2 + 1, mid + 1, r, a);
        pull(p);
    }

    void range_add(int p, int l, int r, int ql, int qr, ll v) {
        if (ql <= l && r <= qr) {
            apply_add(p, l, r, v);
            return;
        }
        pushdown(p, l, r);
        int mid = (l + r) >> 1;
        if (ql <= mid) range_add(p * 2, l, mid, ql, qr, v);
        if (qr > mid) range_add(p * 2 + 1, mid + 1, r, ql, qr, v);
        pull(p);
    }

    void range_set(int p, int l, int r, int ql, int qr, ll v) {
        if (ql <= l && r <= qr) {
            apply_set(p, l, r, v);
            return;
        }
        pushdown(p, l, r);
        int mid = (l + r) >> 1;
        if (ql <= mid) range_set(p * 2, l, mid, ql, qr, v);
        if (qr > mid) range_set(p * 2 + 1, mid + 1, r, ql, qr, v);
        pull(p);
    }

    ll query_sum(int p, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return sum_[p];
        pushdown(p, l, r);
        int mid = (l + r) >> 1;
        ll res = 0;
        if (ql <= mid) res += query_sum(p * 2, l, mid, ql, qr);
        if (qr > mid) res += query_sum(p * 2 + 1, mid + 1, r, ql, qr);
        return res;
    }

    ll query_max(int p, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return mx_[p];
        pushdown(p, l, r);
        int mid = (l + r) >> 1;
        ll res = -INF;  // 注意：区间可能为空的情况不会出现，这里用极小值兜底
        if (ql <= mid) res = std::max(res, query_max(p * 2, l, mid, ql, qr));
        if (qr > mid) res = std::max(res, query_max(p * 2 + 1, mid + 1, r, ql, qr));
        return res;
    }

    ll query_min(int p, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return mn_[p];
        pushdown(p, l, r);
        int mid = (l + r) >> 1;
        ll res = INF;
        if (ql <= mid) res = std::min(res, query_min(p * 2, l, mid, ql, qr));
        if (qr > mid) res = std::min(res, query_min(p * 2 + 1, mid + 1, r, ql, qr));
        return res;
    }
};

// ============================================================================
// SegTreeMaxSubarray —— 单点赋值、区间最大子段和
// ----------------------------------------------------------------------------
// merge 规则（对相邻两段 A、B 合并）：
//   sum  = A.sum + B.sum
//   lmax = max(A.lmax, A.sum + B.lmax)   // 左起最大：全取左段或跨过中点
//   rmax = max(B.rmax, B.sum + A.rmax)   // 右起最大
//   best = max(A.best, B.best, A.rmax + B.lmax)  // 最大子段：左/右/横跨
// ============================================================================
class SegTreeMaxSubarray {
public:
    // 一段区间的聚合信息
    struct Node {
        ll sum;   // 区间总和
        ll lmax;  // 从左端开始的最大子段和
        ll rmax;  // 以右端结尾的最大子段和
        ll best;  // 区间内最大子段和
    };

    explicit SegTreeMaxSubarray(int n) : n_(n), tree_(4 * n + 5) {}

    explicit SegTreeMaxSubarray(const std::vector<ll>& a)
        : n_(static_cast<int>(a.size())), tree_(4 * n_ + 5) {
        build(1, 1, n_, a);
    }

    // 单点赋值：a[pos] = v（pos 1-based）
    void point_set(int pos, ll v) { point_set(1, 1, n_, pos, v); }

    // 区间查询：返回 [l,r] 的聚合信息（1-based，含两端）
    Node query(int l, int r) { return query(1, 1, n_, l, r); }

private:
    int n_;
    std::vector<Node> tree_;

    // 合并两段
    Node merge(const Node& a, const Node& b) const {
        Node c;
        c.sum = a.sum + b.sum;
        c.lmax = std::max(a.lmax, a.sum + b.lmax);
        c.rmax = std::max(b.rmax, b.sum + a.rmax);
        c.best = std::max(std::max(a.best, b.best), a.rmax + b.lmax);
        return c;
    }

    // 由单个值构造叶子节点
    Node make_leaf(ll v) const {
        Node c;
        c.sum = c.lmax = c.rmax = c.best = v;
        return c;
    }

    void build(int p, int l, int r, const std::vector<ll>& a) {
        if (l == r) {
            tree_[p] = make_leaf(a[l - 1]);
            return;
        }
        int mid = (l + r) >> 1;
        build(p * 2, l, mid, a);
        build(p * 2 + 1, mid + 1, r, a);
        tree_[p] = merge(tree_[p * 2], tree_[p * 2 + 1]);
    }

    void point_set(int p, int l, int r, int pos, ll v) {
        if (l == r) {
            tree_[p] = make_leaf(v);
            return;
        }
        int mid = (l + r) >> 1;
        if (pos <= mid) point_set(p * 2, l, mid, pos, v);
        else point_set(p * 2 + 1, mid + 1, r, pos, v);
        tree_[p] = merge(tree_[p * 2], tree_[p * 2 + 1]);
    }

    Node query(int p, int l, int r, int ql, int qr) {
        if (ql <= l && r <= qr) return tree_[p];
        int mid = (l + r) >> 1;
        // 区间只落在一边时直接返回，避免与"空节点"合并
        if (qr <= mid) return query(p * 2, l, mid, ql, qr);
        if (ql > mid) return query(p * 2 + 1, mid + 1, r, ql, qr);
        return merge(query(p * 2, l, mid, ql, qr), query(p * 2 + 1, mid + 1, r, ql, qr));
    }
};

// ============================================================================
// rectangle_area_union —— 扫描线求矩形面积并
// ============================================================================

// 轴对齐矩形（x1<x2, y1<y2）
struct Rect {
    ll x1, y1, x2, y2;
};

// 扫描线内部使用：维护"被覆盖的 y 长度"的线段树（勿直接使用）
// 叶节点 i 对应离散化后的区间 [ys[i], ys[i+1])。
struct AreaCoverTree {
    int m;                        // 离散化后 y 坐标点数
    std::vector<int> cnt;         // 覆盖计数
    std::vector<ll> len;          // 被覆盖长度
    const std::vector<ll>* ys;    // 指向离散化后的 y 坐标

    explicit AreaCoverTree(const std::vector<ll>& y)
        : m(static_cast<int>(y.size())),
          cnt(4 * m + 5, 0),
          len(4 * m + 5, 0),
          ys(&y) {}

    // 根据覆盖计数更新长度
    void apply_len(int p, int l, int r) {
        if (cnt[p] > 0) {
            len[p] = (*ys)[r + 1] - (*ys)[l];
        } else if (l == r) {
            len[p] = 0;
        } else {
            len[p] = len[p * 2] + len[p * 2 + 1];
        }
    }

    // 对叶区间 [ql,qr] 加覆盖 v（v 为 +1 / -1）
    void update(int p, int l, int r, int ql, int qr, int v) {
        if (ql <= l && r <= qr) {
            cnt[p] += v;
            apply_len(p, l, r);
            return;
        }
        int mid = (l + r) >> 1;
        if (ql <= mid) update(p * 2, l, mid, ql, qr, v);
        if (qr > mid) update(p * 2 + 1, mid + 1, r, ql, qr, v);
        apply_len(p, l, r);
    }
};

inline ll rectangle_area_union(const std::vector<Rect>& rects) {
    if (rects.empty()) return 0;

    // 1. 离散化 y 坐标
    std::vector<ll> ys;
    for (const Rect& r : rects) {
        ys.push_back(r.y1);
        ys.push_back(r.y2);
    }
    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());
    int m = static_cast<int>(ys.size());
    if (m < 2) return 0;  // 所有矩形退化为一条线，面积为 0

    // 2. 生成扫描事件：x 处加/减一条竖直边 [y1, y2)
    struct ScanEvent {
        ll x;
        int y1;   // 离散化下标（下界）
        int y2;   // 离散化下标（上界，开区间）
        int type; // +1 左边界进入，-1 右边界离开
    };
    std::vector<ScanEvent> events;
    events.reserve(rects.size() * 2);
    for (const Rect& r : rects) {
        int iy1 = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), r.y1) - ys.begin());
        int iy2 = static_cast<int>(std::lower_bound(ys.begin(), ys.end(), r.y2) - ys.begin());
        events.push_back(ScanEvent{r.x1, iy1, iy2, +1});
        events.push_back(ScanEvent{r.x2, iy1, iy2, -1});
    }
    std::sort(events.begin(), events.end(),
              [](const ScanEvent& a, const ScanEvent& b) { return a.x < b.x; });

    // 3. 扫描：相邻 x 之间，被覆盖 y 长度 × x 跨度即为面积增量
    AreaCoverTree seg(ys);
    ll area = 0;
    ll prev_x = events.front().x;
    for (const ScanEvent& e : events) {
        area += seg.len[1] * (e.x - prev_x);
        // 更新覆盖区间 [y1, y2-1]（叶节点编号 0..m-2）
        if (e.y1 < e.y2) seg.update(1, 0, m - 2, e.y1, e.y2 - 1, e.type);
        prev_x = e.x;
    }
    return area;
}

}  // namespace algo
