#pragma once
// ============================================================================
// geometry/point.hpp —— 二维点与向量
// ----------------------------------------------------------------------------
// 功能：二维计算几何的基础类型。提供点的构造、向量加减、标量乘除、叉积/点积、
//       长度、单位化、旋转、极角、排序与 eps 相等比较，以及符号函数 sgn、
//       极角排序 polar_less 等自由函数。其余几何头文件都直接或间接依赖本文件。
// 时间复杂度：所有操作均为 O(1)。
// 空间复杂度：O(1)。
// 使用示例：
//   #include "geometry/point.hpp"
//   using algo::Point;
//   Point a(0, 0), b(3, 4);
//   double d = (b - a).len();                     // 5
//   double c = algo::cross(b, Point(1, 0));       // 向量叉积
//   if (algo::sgn(c) > 0) { /* b 在 x 轴逆时针一侧 */ }
// 注意事项：
//   - 浮点比较统一使用 algo::EPS（1e-9）；判零请用 sgn(x) 而非 x == 0。
//   - operator< 使用"精确"的 x 后 y 比较，保证严格弱序、可安全用于 std::sort；
//     operator== 使用 eps 比较，仅用于近似去重等场景（eps 相等不可传递）。
//   - 成员 cross(a, b) 返回 (a - *this) × (b - *this)（以 *this 为参考点）；
//     两个向量的叉积请使用自由函数 algo::cross(a, b)。
// ============================================================================
#include "algo/common.hpp"

#include <cmath>

namespace algo {

// 二维点 / 向量：既表示坐标点，也表示从原点出发的向量
struct Point {
    double x, y;

    // 默认构造：原点 (0, 0)
    Point() : x(0.0), y(0.0) {}

    // 用坐标构造
    Point(double x_, double y_) : x(x_), y(y_) {}

    // ---- 向量加减 ----
    Point operator+(const Point& o) const { return Point(x + o.x, y + o.y); }
    Point operator-(const Point& o) const { return Point(x - o.x, y - o.y); }

    // ---- 标量乘除 ----
    Point operator*(double k) const { return Point(x * k, y * k); }
    Point operator/(double k) const { return Point(x / k, y / k); }

    // 叉积（以 *this 为参考点）：返回向量 (a - *this) 与 (b - *this) 的叉积。
    // 用途：判断 a、b 相对 *this 的旋转方向（>0 逆时针，<0 顺时针，=0 共线）。
    double cross(const Point& a, const Point& b) const {
        return (a.x - x) * (b.y - y) - (a.y - y) * (b.x - x);
    }

    // 点积：*this 与 a 的点积（把 *this 当作向量）
    double dot(const Point& a) const { return x * a.x + y * a.y; }

    // 向量长度（模）
    double len() const { return std::sqrt(x * x + y * y); }

    // 单位向量（方向相同、长度为 1）；零向量归一化得到零向量（避免除零）
    Point normalized() const {
        double l = len();
        if (l < EPS) return Point(0.0, 0.0);
        return Point(x / l, y / l);
    }

    // 逆时针旋转 ang 弧度后的向量
    Point rotated(double ang) const {
        double c = std::cos(ang);
        double s = std::sin(ang);
        return Point(x * c - y * s, x * s + y * c);
    }

    // 极角：atan2(y, x)，取值范围 [-pi, pi]
    double polar() const { return std::atan2(y, x); }

    // 用于排序：先按 x，再按 y（精确比较，保证严格弱序，供 std::sort 使用）
    bool operator<(const Point& o) const {
        if (x != o.x) return x < o.x;
        return y < o.y;
    }

    // eps 相等比较（用于近似去重等场景）
    bool operator==(const Point& o) const {
        return std::fabs(x - o.x) <= EPS && std::fabs(y - o.y) <= EPS;
    }
};

// 符号函数：x > EPS → 1，x < -EPS → -1，否则 → 0（浮点判零的统一入口）
inline int sgn(double x) {
    if (x > EPS) return 1;
    if (x < -EPS) return -1;
    return 0;
}

// 叉积：向量 a 与 b 的叉积 a × b = a.x * b.y - a.y * b.x
inline double cross(const Point& a, const Point& b) {
    return a.x * b.y - a.y * b.x;
}

// 点积：向量 a 与 b 的点积
inline double dot(const Point& a, const Point& b) {
    return a.x * b.x + a.y * b.y;
}

// 极角排序比较器：先按象限（逆时针 0→3），再按叉积；共线时距原点近者在前。
// 可用于以原点为极点的 std::sort，满足严格弱序。
inline bool polar_less(const Point& a, const Point& b) {
    // 象限划分（逆时针）：
    //   0：x >= 0 且 y >= 0（第一象限，含 +x、+y 轴）
    //   1：x <  0 且 y >= 0（第二象限，含 -x 轴）
    //   2：x <  0 且 y <  0（第三象限）
    //   3：x >= 0 且 y <  0（第四象限，含 -y 轴）
    auto quadrant = [](const Point& p) -> int {
        if (p.x >= 0.0) return (p.y >= 0.0) ? 0 : 3;
        else            return (p.y >= 0.0) ? 1 : 2;
    };
    int qa = quadrant(a), qb = quadrant(b);
    if (qa != qb) return qa < qb;
    double cr = cross(a, b);
    if (sgn(cr) != 0) return cr > 0;   // 同一象限内按逆时针排序
    return a.len() < b.len();          // 共线时距原点近者在前（保证严格弱序）
}

}  // namespace algo
