#pragma once
// ============================================================================
// algo/ds/fenwick.hpp —— 树状数组（Fenwick Tree / BIT）
// ----------------------------------------------------------------------------
// 功能：一维与二维的树状数组，支持以下四种形态：
//   1. BIT<T>        单点加、前缀和、区间和。
//   2. BITRangeAdd<T> 差分实现"区间加、单点查询"。
//   3. BITRange<T>    双 BIT 实现"区间加、区间和"。
//   4. BIT2D         二维 BIT，单点加、矩形和。
// 时间复杂度：一维单次操作 O(log n)；二维单次操作 O(log n · log m)。
// 空间复杂度：一维 O(n)；二维 O(n·m)。
// 使用示例：
//   BIT<ll> bit(100);
//   bit.add(3, 5);
//   ll s = bit.range_sum(1, 3);   // 5
//
//   BITRangeAdd<ll> bra(100);
//   bra.range_add(2, 7, 3);       // a[2..7] 各 +3
//   ll v = bra.point_query(5);    // 3
//
//   BITRange<ll> br(100);
//   br.range_add(1, 100, 2);      // 全区间 +2
//   ll t = br.range_sum(1, 100);  // 200
//
//   BIT2D b2(10, 10);
//   b2.add(1, 1, 3);
//   ll r = b2.range_sum(1, 1, 10, 10);  // 3
// 注意事项：
//   - 所有下标均为 1..n（1..m），越界访问行为未定义。
//   - T 需支持 += 与减法（BITRange 的区间和依赖差分回推，需支持负值）。
//   - BIT2D 固定使用 long long，累加可能溢出时请留意数据范围。
// ============================================================================

#include <vector>

#include "algo/common.hpp"

namespace algo {

// ============================================================================
// BIT —— 单点加、前缀和、区间和
// ============================================================================
template <typename T>
class BIT {
public:
    // 构造大小为 n（下标 1..n）的树状数组，初始全 0
    explicit BIT(int n) : n_(n), tree_(n + 1, T(0)) {}

    // 单点加：a[i] += v
    void add(int i, T v) {
        for (; i <= n_; i += i & -i) tree_[i] += v;
    }

    // 前缀和：a[1] + ... + a[i]
    T sum(int i) const {
        T s = T(0);
        for (; i > 0; i -= i & -i) s += tree_[i];
        return s;
    }

    // 区间和：a[l] + ... + a[r]
    T range_sum(int l, int r) const { return sum(r) - sum(l - 1); }

private:
    int n_;
    std::vector<T> tree_;
};

// ============================================================================
// BITRangeAdd —— 差分树状数组：区间加、单点查询
// ============================================================================
template <typename T>
class BITRangeAdd {
public:
    explicit BITRangeAdd(int n) : n_(n), bit_(n) {}

    // 区间加：a[l..r] 各 += v
    void range_add(int l, int r, T v) {
        bit_.add(l, v);
        if (r + 1 <= n_) bit_.add(r + 1, -v);  // 差分在 r+1 处减去
    }

    // 单点查询：返回 a[i]
    T point_query(int i) const { return bit_.sum(i); }

private:
    int n_;
    BIT<T> bit_;  // 维护差分数组
};

// ============================================================================
// BITRange —— 双 BIT：区间加、区间和
// ----------------------------------------------------------------------------
// 原理：维护差分 d[i] 与 i·d[i] 两个 BIT。区间 [l,r] 加 v 等价于
//   d[l] += v, d[r+1] -= v；i·d[i] 相应在 l 处 + l·v、在 r+1 处 -(r+1)·v。
// 前缀和 a[1..i] = (i+1)·Σd[k] - Σ(k·d[k])（k=1..i）。
// ============================================================================
template <typename T>
class BITRange {
public:
    explicit BITRange(int n) : n_(n), d_(n), id_(n) {}

    // 区间加：a[l..r] 各 += v
    void range_add(int l, int r, T v) {
        d_.add(l, v);
        id_.add(l, T(l) * v);
        if (r + 1 <= n_) {
            d_.add(r + 1, -v);
            id_.add(r + 1, -T(r + 1) * v);
        }
    }

    // 前缀和：a[1] + ... + a[i]
    T prefix_sum(int i) const { return T(i + 1) * d_.sum(i) - id_.sum(i); }

    // 区间和：a[l] + ... + a[r]
    T range_sum(int l, int r) const { return prefix_sum(r) - prefix_sum(l - 1); }

private:
    int n_;
    BIT<T> d_;   // 差分 d[i]
    BIT<T> id_;  // i · d[i]
};

// ============================================================================
// BIT2D —— 二维树状数组：单点加、矩形和
// ============================================================================
class BIT2D {
public:
    // 构造 n 行 m 列的二维 BIT，初始全 0
    BIT2D(int n, int m) : n_(n), m_(m), tree_(n + 1, std::vector<ll>(m + 1, 0)) {}

    // 单点加：a[x][y] += v
    void add(int x, int y, ll v) {
        for (int i = x; i <= n_; i += i & -i) {
            for (int j = y; j <= m_; j += j & -j) {
                tree_[i][j] += v;
            }
        }
    }

    // 前缀和：Σ_{i=1..x, j=1..y} a[i][j]
    ll sum(int x, int y) const {
        ll s = 0;
        for (int i = x; i > 0; i -= i & -i) {
            for (int j = y; j > 0; j -= j & -j) {
                s += tree_[i][j];
            }
        }
        return s;
    }

    // 矩形和：Σ_{i=x1..x2, j=y1..y2} a[i][j]
    ll range_sum(int x1, int y1, int x2, int y2) const {
        return sum(x2, y2) - sum(x1 - 1, y2) - sum(x2, y1 - 1) + sum(x1 - 1, y1 - 1);
    }

private:
    int n_, m_;
    std::vector<std::vector<ll>> tree_;
};

}  // namespace algo
