#pragma once
// ============================================================================
// geometry/circle.hpp —— 圆
// ----------------------------------------------------------------------------
// 功能：圆与直线/圆的交点、点到圆的切点、最小圆覆盖。
//       提供 Circle 类型、circumcircle（三点外接圆，内部辅助，命名唯一）与
//       min_circle_cover（随机增量法最小圆覆盖）。
// 时间复杂度：
//   - circle_line_intersection / circle_circle_intersection / tangent_points：O(1)
//   - circumcircle：O(1)
//   - min_circle_cover：期望 O(n)（随机增量法），最坏 O(n^2)
// 空间复杂度：交点/切点 O(1)；min_circle_cover O(n)（输入拷贝 + shuffle）。
// 使用示例：
//   #include "geometry/circle.hpp"
//   algo::Circle c(algo::Point(0, 0), 1.0);
//   algo::Line l(algo::Point(-2, 0), algo::Point(2, 0));
//   std::vector<algo::Point> pts = algo::circle_line_intersection(c, l);
//   std::vector<algo::Point> tp = algo::tangent_points(algo::Point(2, 0), c);
// 注意事项：
//   - 圆与直线相切返回 1 个交点；相离返回空；相交返回 2 个（沿直线方向顺序）。
//   - 两圆相交：外离 / 内含返回空；内切 / 外切返回 1 个；相交返回 2 个；
//     重合圆（无穷交点）按约定返回空。
//   - tangent_points：点在圆内返回空；点在圆上返回该点；圆外返回 2 个切点。
//   - circumcircle 在三点共线时退化为"最远两点为直径"的圆（供最小圆覆盖使用）。
//   - min_circle_cover 使用固定种子的 mt19937 shuffle，结果确定、期望 O(n)。
// ============================================================================
#include "geometry/point.hpp"
#include "geometry/line.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace algo {

// 圆：圆心 c，半径 r
struct Circle {
    Point c;
    double r;
    Circle() : c(), r(0.0) {}
    Circle(const Point& c_, double r_) : c(c_), r(r_) {}
};

// 三点外接圆（内部辅助函数，命名唯一，供最小圆覆盖使用）。
// 三点共线时退化为"最远两点为直径"的圆（此时不存在真正的外接圆）。
inline Circle circumcircle(const Point& a, const Point& b, const Point& c) {
    // 共线退化：取距离最远的两点作为直径
    if (sgn(cross(b - a, c - a)) == 0) {
        double dab = (a - b).len();
        double dac = (a - c).len();
        double dbc = (b - c).len();
        if (dab >= dac && dab >= dbc) return Circle((a + b) * 0.5, dab * 0.5);
        if (dac >= dab && dac >= dbc) return Circle((a + c) * 0.5, dac * 0.5);
        return Circle((b + c) * 0.5, dbc * 0.5);
    }

    // 标准外接圆公式（解两条垂直平分线方程）
    double ax = a.x, ay = a.y;
    double bx = b.x, by = b.y;
    double cx = c.x, cy = c.y;
    double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
    double ux = ((ax * ax + ay * ay) * (by - cy) +
                 (bx * bx + by * by) * (cy - ay) +
                 (cx * cx + cy * cy) * (ay - by)) / d;
    double uy = ((ax * ax + ay * ay) * (cx - bx) +
                 (bx * bx + by * by) * (ax - cx) +
                 (cx * cx + cy * cy) * (bx - ax)) / d;
    Point center(ux, uy);
    return Circle(center, (center - a).len());
}

// 圆与直线的交点（返回 0/1/2 个点，沿直线方向依次为 +dir、-dir）。
inline std::vector<Point> circle_line_intersection(const Circle& c, const Line& l) {
    std::vector<Point> res;
    Point pr = projection(c.c, l);          // 圆心在直线上的投影
    double dist = (c.c - pr).len();         // 圆心到直线的距离

    if (dist > c.r + EPS) return res;       // 相离：无交点
    if (std::fabs(dist - c.r) <= EPS) {     // 相切：1 个交点
        res.push_back(pr);
        return res;
    }

    // 相交：2 个交点，沿直线方向 ± half
    double half = std::sqrt(std::max(0.0, c.r * c.r - dist * dist));
    Point dir = (l.b - l.a).normalized();
    res.push_back(pr + dir * half);
    res.push_back(pr - dir * half);
    return res;
}

// 两圆的交点（返回 0/1/2 个点）。
// 外离 / 内含返回空；内切 / 外切返回 1 个；相交返回 2 个；重合返回空。
inline std::vector<Point> circle_circle_intersection(const Circle& c1, const Circle& c2) {
    std::vector<Point> res;
    double d = (c2.c - c1.c).len();  // 圆心距

    if (d < EPS) return res;                        // 同心（重合或同心不同半径），无法唯一定义交点
    if (d > c1.r + c2.r + EPS) return res;          // 外离
    if (d < std::fabs(c1.r - c2.r) - EPS) return res;  // 内含

    // a：c1 圆心到"交点弦与连心线交点"的距离
    double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2.0 * d);
    double h2 = c1.r * c1.r - a * a;
    double h = (h2 > EPS) ? std::sqrt(h2) : 0.0;

    Point v = c2.c - c1.c;              // 连心线方向
    Point base = c1.c + v * (a / d);    // 交点弦中点
    Point perp(-v.y / d, v.x / d);      // 连心线的垂直单位向量

    if (h < EPS) {                      // 相切（内切/外切）：1 个交点
        res.push_back(base);
        return res;
    }
    res.push_back(base + perp * h);
    res.push_back(base - perp * h);
    return res;
}

// 点 p 到圆 c 的切点。
// 点在圆内返回空；点在圆上返回该点；点在圆外返回 2 个切点。
inline std::vector<Point> tangent_points(const Point& p, const Circle& c) {
    std::vector<Point> res;
    double d = (p - c.c).len();  // 点到圆心距离

    if (d < c.r - EPS) return res;          // 点在圆内：无切点
    if (std::fabs(d - c.r) <= EPS) {        // 点在圆上：切点即该点
        res.push_back(p);
        return res;
    }

    // 点在圆外：两个切点。
    // 切点弦垂直于 OP，其到圆心距离 a = r^2 / d，弦半长 h = sqrt(r^2 - a^2)。
    double a = c.r * c.r / d;
    double h = std::sqrt(std::max(0.0, c.r * c.r - a * a));
    Point dir = (p - c.c) / d;              // 圆心指向 p 的单位向量
    Point base = c.c + dir * a;             // 两个切点连线的中点
    Point perp(-dir.y, dir.x);              // 垂直方向
    res.push_back(base + perp * h);
    res.push_back(base - perp * h);
    return res;
}

// 最小圆覆盖（随机增量法，期望 O(n)）。
// 先 std::shuffle 打乱（内部用 mt19937，固定种子保证结果可复现），再增量构造。
inline Circle min_circle_cover(std::vector<Point> pts) {
    int n = static_cast<int>(pts.size());
    if (n == 0) return Circle(Point(0.0, 0.0), 0.0);
    if (n == 1) return Circle(pts[0], 0.0);

    // 固定种子打乱，避免最坏 O(n^2)；结果与打乱无关，故可用固定种子
    std::mt19937 rng(20230601u);
    std::shuffle(pts.begin(), pts.end(), rng);

    Circle c(pts[0], 0.0);
    for (int i = 1; i < n; ++i) {
        if ((pts[i] - c.c).len() > c.r + EPS) {
            // 点 i 在圆外：新圆必须经过 i
            c = Circle(pts[i], 0.0);
            for (int j = 0; j < i; ++j) {
                if ((pts[j] - c.c).len() > c.r + EPS) {
                    // 新圆必须经过 i 和 j：以 ij 为直径
                    c.c = (pts[i] + pts[j]) * 0.5;
                    c.r = (pts[i] - pts[j]).len() * 0.5;
                    for (int k = 0; k < j; ++k) {
                        if ((pts[k] - c.c).len() > c.r + EPS) {
                            // 新圆必须经过 i、j、k：三点定圆
                            c = circumcircle(pts[i], pts[j], pts[k]);
                        }
                    }
                }
            }
        }
    }
    return c;
}

}  // namespace algo
