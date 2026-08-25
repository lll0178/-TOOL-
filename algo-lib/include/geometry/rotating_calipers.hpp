#pragma once
// ============================================================================
// geometry/rotating_calipers.hpp —— 旋转卡壳
// ----------------------------------------------------------------------------
// 功能：用旋转卡壳求凸包直径（最远点对距离）。
// 时间复杂度：O(n)。
// 空间复杂度：O(1)（不计输入）。
// 使用示例：
//   #include "geometry/rotating_calipers.hpp"
//   #include "geometry/convex_hull.hpp"
//   std::vector<algo::Point> hull = algo::convex_hull(pts);
//   double d = algo::convex_diameter(hull);
// 注意事项：
//   - 输入必须是凸包顶点：逆时针顺序、无重复点（可由 convex_hull 直接得到）。
//   - 对 n <= 2 的退化凸包做了特殊处理（返回 0 或两点距离）。
//   - 该算法求的是"点对"直径；对任意点集需先求凸包再调用。
// ============================================================================
#include "geometry/point.hpp"

#include <algorithm>
#include <vector>

namespace algo {

// 旋转卡壳求凸包直径（最远点对距离，返回距离值）。
// 输入：凸包顶点，逆时针顺序、无重复点。
inline double convex_diameter(const std::vector<Point>& hull) {
    int n = static_cast<int>(hull.size());
    if (n <= 1) return 0.0;
    if (n == 2) return (hull[1] - hull[0]).len();

    // 对每条边 hull[i] -> hull[i+1]，维护其对踵点 j（使三角形面积最大）。
    int j = 1;
    double ans = 0.0;
    for (int i = 0; i < n; ++i) {
        Point e = hull[(i + 1) % n] - hull[i];  // 当前边向量
        // 移动对踵点：比较三角形 (hull[i], hull[i+1], p) 的有向面积
        while (cross(e, hull[(j + 1) % n] - hull[i]) >
               cross(e, hull[j] - hull[i])) {
            j = (j + 1) % n;
        }
        // 最远点对可能出现在边端点与对踵点之间
        ans = std::max(ans, (hull[i] - hull[j]).len());
        ans = std::max(ans, (hull[(i + 1) % n] - hull[j]).len());
    }
    return ans;
}

}  // namespace algo
