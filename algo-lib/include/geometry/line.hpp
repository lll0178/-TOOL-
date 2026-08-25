#pragma once
// ============================================================================
// geometry/line.hpp —— 直线与线段
// ----------------------------------------------------------------------------
// 功能：提供直线/线段类型 Line，以及点与线段的判定与距离计算：
//       点在线上（on_segment）、线段相交（segments_intersect）、
//       直线交点（line_intersection）、点到直线/线段距离、
//       点在直线上的投影（projection）。
// 时间复杂度：所有操作均为 O(1)。
// 空间复杂度：O(1)。
// 使用示例：
//   #include "geometry/line.hpp"
//   algo::Line l1(algo::Point(0, 0), algo::Point(1, 1));
//   algo::Line l2(algo::Point(0, 1), algo::Point(1, 0));
//   if (algo::segments_intersect(l1.a, l1.b, l2.a, l2.b)) {
//       algo::Point p = algo::line_intersection(l1, l2);
//   }
// 注意事项：
//   - line_intersection 要求两直线不平行（cross(方向向量) != 0），否则除零；
//     调用前可用 sgn(cross(l1.b-l1.a, l2.b-l2.a)) 判断。
//   - 所有判定均基于 algo::EPS 的 sgn 比较，处理浮点误差。
// ============================================================================
#include "geometry/point.hpp"

#include <cmath>

namespace algo {

// 直线 / 线段：由两个端点确定（a、b 表示直线经过的两点，也可是线段端点）
struct Line {
    Point a, b;
    Line() : a(), b() {}
    Line(const Point& a_, const Point& b_) : a(a_), b(b_) {}
};

// 点 p 是否在线段 [a, b] 上（含端点）。
// 先判共线（叉积为 0），再判范围（p-a 与 p-b 点积 <= 0，即 p 在 a、b 之间）。
inline bool on_segment(const Point& p, const Point& a, const Point& b) {
    if (sgn(cross(b - a, p - a)) != 0) return false;
    return dot(p - a, p - b) <= EPS;
}

// 线段 [a, b] 与 [c, d] 是否相交（含规范相交与端点/共线相交）。
// 采用跨立实验：两条线段必须互相"跨立"对方所在直线。
inline bool segments_intersect(const Point& a, const Point& b,
                               const Point& c, const Point& d) {
    int o1 = sgn(cross(b - a, c - a));  // c 在直线 ab 的哪一侧
    int o2 = sgn(cross(b - a, d - a));  // d 在直线 ab 的哪一侧
    int o3 = sgn(cross(d - c, a - c));  // a 在直线 cd 的哪一侧
    int o4 = sgn(cross(d - c, b - c));  // b 在直线 cd 的哪一侧

    // 规范相交：两对叉积符号都相反
    if (o1 * o2 < 0 && o3 * o4 < 0) return true;

    // 端点 / 共线相交：某个叉积为 0 且对应点落在另一条线段上
    if (o1 == 0 && on_segment(c, a, b)) return true;
    if (o2 == 0 && on_segment(d, a, b)) return true;
    if (o3 == 0 && on_segment(a, c, d)) return true;
    if (o4 == 0 && on_segment(b, c, d)) return true;
    return false;
}

// 两直线交点（要求两直线不平行，即 cross(l1.b-l1.a, l2.b-l2.a) != 0）。
// 公式：P = a + t * (b - a)，其中 t = cross(c - a, v2) / cross(v1, v2)。
inline Point line_intersection(const Line& l1, const Line& l2) {
    const Point& a = l1.a;
    const Point& b = l1.b;
    const Point& c = l2.a;
    const Point& d = l2.b;
    Point v1 = b - a;   // 直线 1 的方向向量
    Point v2 = d - c;   // 直线 2 的方向向量
    double denom = cross(v1, v2);
    // 调用方需保证 denom != 0（两直线不平行），否则除零
    double t = cross(c - a, v2) / denom;
    return a + v1 * t;
}

// 点 p 到直线 l 的距离：|叉积| / |方向向量|
inline double point_line_dist(const Point& p, const Line& l) {
    Point v = l.b - l.a;
    return std::fabs(cross(v, p - l.a)) / v.len();
}

// 点 p 到线段 [a, b] 的距离。
inline double point_segment_dist(const Point& p, const Point& a, const Point& b) {
    Point ab = b - a;
    Point ap = p - a;
    // 投影落在 a 之外（沿 ab 方向的投影 <= 0）：距离为 |p - a|
    if (dot(ab, ap) <= 0.0) return ap.len();
    // 投影落在 b 之外（等价于 dot(ab, p-b) >= 0）：距离为 |p - b|
    if (dot(ab, p - b) >= 0.0) return (p - b).len();
    // 投影在线段内部：距离为到直线 ab 的距离
    return std::fabs(cross(ab, ap)) / ab.len();
}

// 点 p 在直线 l 上的投影点。
inline Point projection(const Point& p, const Line& l) {
    Point v = l.b - l.a;
    double t = dot(p - l.a, v) / dot(v, v);
    return l.a + v * t;
}

}  // namespace algo
