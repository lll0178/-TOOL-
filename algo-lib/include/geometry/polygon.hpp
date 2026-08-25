#pragma once
// ============================================================================
// geometry/polygon.hpp —— 多边形
// ----------------------------------------------------------------------------
// 功能：提供多边形的面积（叉积）、点在多边形内判定（射线法）与重心计算。
// 时间复杂度：polygon_area O(n)；point_in_polygon O(n)；polygon_centroid O(n)。
// 空间复杂度：均为 O(1)（不计输入）。
// 使用示例：
//   #include "geometry/polygon.hpp"
//   std::vector<algo::Point> poly = {{0,0},{2,0},{2,2},{0,2}};
//   double area = algo::polygon_area(poly);        // 4
//   int where = algo::point_in_polygon({1,1}, poly); // 1（内部）
//   algo::Point g = algo::polygon_centroid(poly);  // (1,1)
// 注意事项：
//   - 顶点须按顺/逆时针顺序给出（不能乱序）；面积返回绝对值，与方向无关。
//   - point_in_polygon 返回 1（内部）/0（外部）/2（边上）；边上判定使用 eps。
//   - 退化多边形（n=0/1/2）已处理：面积为 0，点按"是否在退化线段上"判定。
// ============================================================================
#include "geometry/point.hpp"
#include "geometry/line.hpp"

#include <cmath>
#include <vector>

namespace algo {

// 多边形面积：顶点按顺序给出的简单多边形，返回绝对值（叉积法 / 鞋带公式）。
inline double polygon_area(const std::vector<Point>& p) {
    int n = static_cast<int>(p.size());
    if (n < 3) return 0.0;
    double area2 = 0.0;  // 二倍有向面积
    for (int i = 0; i < n; ++i) {
        const Point& a = p[i];
        const Point& b = p[(i + 1) % n];
        area2 += cross(a, b);
    }
    return std::fabs(area2) * 0.5;
}

// 点在多边形内判定（射线法）。
// 返回 1（内部）/ 0（外部）/ 2（边上）。
inline int point_in_polygon(const Point& p, const std::vector<Point>& poly) {
    int n = static_cast<int>(poly.size());
    if (n == 0) return 0;

    // 先判断是否落在某条边上（含退化情况：单点 / 线段）
    for (int i = 0; i < n; ++i) {
        const Point& a = poly[i];
        const Point& b = poly[(i + 1) % n];
        if (on_segment(p, a, b)) return 2;
    }

    // 射线法：从 p 水平向右发射射线，统计与多边形边的交点个数（奇数=内部）。
    // 使用半开规则 (a.y > p.y) != (b.y > p.y)，正确处理射线穿过顶点的情况。
    bool inside = false;
    for (int i = 0; i < n; ++i) {
        const Point& a = poly[i];
        const Point& b = poly[(i + 1) % n];
        if ((a.y > p.y) != (b.y > p.y)) {
            // 射线与边交点的 x 坐标
            double x_cross = a.x + (p.y - a.y) / (b.y - a.y) * (b.x - a.x);
            if (x_cross > p.x) inside = !inside;
        }
    }
    return inside ? 1 : 0;
}

// 多边形重心（三角形剖分加权平均）。
// 以 p[0] 为基准，将多边形剖分为三角形 (p0, p[i], p[i+1])，按有向面积加权求重心。
inline Point polygon_centroid(const std::vector<Point>& p) {
    int n = static_cast<int>(p.size());
    if (n == 0) return Point();
    if (n == 1) return p[0];

    const Point& p0 = p[0];
    double total_area2 = 0.0;   // 二倍有向面积之和
    double cx = 0.0, cy = 0.0;  // 面积加权累计
    for (int i = 1; i + 1 < n; ++i) {
        const Point& a = p[i];
        const Point& b = p[i + 1];
        double area2 = cross(a - p0, b - p0);  // 三角形二倍有向面积
        // 三角形重心 = (p0 + a + b) / 3，乘上有向面积加权
        cx += area2 * (p0.x + a.x + b.x) / 3.0;
        cy += area2 * (p0.y + a.y + b.y) / 3.0;
        total_area2 += area2;
    }

    // 退化（共线等面积为 0）时退化为所有顶点的算术平均，避免除零
    if (std::fabs(total_area2) < EPS) {
        double sx = 0.0, sy = 0.0;
        for (int i = 0; i < n; ++i) {
            sx += p[i].x;
            sy += p[i].y;
        }
        return Point(sx / n, sy / n);
    }
    return Point(cx / total_area2, cy / total_area2);
}

}  // namespace algo
