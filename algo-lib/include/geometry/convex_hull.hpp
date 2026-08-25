#pragma once
// ============================================================================
// geometry/convex_hull.hpp —— 凸包（Andrew 单调链）
// ----------------------------------------------------------------------------
// 功能：求点集的凸包顶点（逆时针、不含共线中间点），以及凸包周长。
// 时间复杂度：convex_hull O(n log n)（排序主导）；convex_perimeter O(n)。
// 空间复杂度：O(n)。
// 使用示例：
//   #include "geometry/convex_hull.hpp"
//   std::vector<algo::Point> pts = {{0,0},{1,1},{2,0},{1,0}};
//   std::vector<algo::Point> hull = algo::convex_hull(pts);
//   double per = algo::convex_perimeter(hull);
// 注意事项：
//   - 先按 (x, y) 排序并 eps 去重（去重用 operator== 的 eps 比较）。
//   - 返回逆时针凸包顶点，不含边上的共线中间点（仅保留"角点"）。
//   - 去重后点数 < 3（如全共线、单点、两点）时，返回去重后的全部点；
//     全共线时返回左右两个端点。
//   - 凸包周长对 2 点退化凸包定义为往返距离（2 * 边长），一般调用方传入
//     点数 >= 3 的合法凸包。
// ============================================================================
#include "geometry/point.hpp"

#include <algorithm>
#include <vector>

namespace algo {

// Andrew 单调链求凸包。
// 输入：任意顺序的点集（按值传入，函数内部会排序、去重，不修改调用方数据）。
// 输出：逆时针凸包顶点（不含共线中间点）。
inline std::vector<Point> convex_hull(std::vector<Point> pts) {
    // 1. 按 (x, y) 排序
    std::sort(pts.begin(), pts.end());

    // 2. eps 去重（相邻相等即去重）
    std::vector<Point> uniq;
    uniq.reserve(pts.size());
    for (std::size_t i = 0; i < pts.size(); ++i) {
        if (uniq.empty() || !(uniq.back() == pts[i])) {
            uniq.push_back(pts[i]);
        }
    }

    int n = static_cast<int>(uniq.size());
    // 退化：去重后点数 < 3，无法构成凸多边形，返回去重后的全部点
    if (n < 3) return uniq;

    // 3. Andrew 单调链：先下凸包，再上凸包
    std::vector<Point> hull(2 * n);
    int k = 0;

    // 下凸包（从左到右）
    for (int i = 0; i < n; ++i) {
        // 维护"严格左转"：cross <= 0 表示右转或共线，弹出（去除共线中间点）
        while (k >= 2 &&
               sgn(cross(hull[k - 1] - hull[k - 2], uniq[i] - hull[k - 1])) <= 0) {
            --k;
        }
        hull[k++] = uniq[i];
    }

    // 上凸包（从右到左），t 保证不会弹掉下凸包的起点
    for (int i = n - 2, t = k + 1; i >= 0; --i) {
        while (k >= t &&
               sgn(cross(hull[k - 1] - hull[k - 2], uniq[i] - hull[k - 1])) <= 0) {
            --k;
        }
        hull[k++] = uniq[i];
    }

    hull.resize(k - 1);  // 末尾的起点与首点重复，去掉
    return hull;
}

// 凸包周长：输入为凸包顶点（逆时针、无重复点），返回周长。
inline double convex_perimeter(const std::vector<Point>& hull) {
    int n = static_cast<int>(hull.size());
    if (n <= 1) return 0.0;
    double per = 0.0;
    for (int i = 0; i < n; ++i) {
        per += (hull[(i + 1) % n] - hull[i]).len();
    }
    return per;
}

}  // namespace algo
