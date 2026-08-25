#pragma once
// ============================================================================
// algo/binary_search.hpp —— 函数式二分（在单调谓词上二分查找分界点）
// ----------------------------------------------------------------------------
// 功能：
//   提供整数域与实数域上的"二分答案"模板。相比直接对数组二分，这里把判定
//   条件抽象成一个谓词（pred(mid)），只要 pred 在区间上单调（false→true 或
//   true→false），就能求出分界点。常见用法：
//     - 求最后一个满足 a[i] <= x 的下标（升序数组）；
//     - 求第一个满足 a[i] >= x 的下标；
//     - 求最小的 x 使得 check(x) 成立（最大值最小化 / 最小值最大化）；
//     - 求单调函数 f 的零点（f(mid) >= 0 的分界点）。
// 时间复杂度：
//   整数二分：O(log(r - l + 1)) 次 pred 调用；
//   实数二分：O(iter) 次 pred 调用（iter 默认 100）。
// 空间复杂度：O(1)（不计谓词自身开销）。
// 使用示例：
//   // 例 1：求最后一个满足 a[i] <= x 的下标（a 升序，闭区间 [0, n-1]）
//   int pos = algo::upper_bound_func(0, n - 1,
//       [&](int mid) { return a[mid] <= x; });
//   // 找不到时返回 l - 1，即 -1。
//
//   // 例 2：求第一个满足 a[i] >= x 的下标
//   int pos = algo::lower_bound_func(0, n - 1,
//       [&](int mid) { return a[mid] >= x; });
//   // 找不到时返回 r + 1，即 n。
//
//   // 例 3：实数二分求单调递增函数 f 的零点 f(x)=0 的近似解
//   double x = algo::binary_search_real(-1e9, 1e9,
//       [&](double mid) { return f(mid) >= 0.0; });
// 注意事项：
//   - pred 必须在 [l, r] 上满足对应单调性，否则结果无意义；
//   - 整数版在空区间（l > r）时按"找不到"返回：lower 返回 r + 1，
//     upper 返回 l - 1；
//   - mid 采用 l + (r - l) / 2 与 l + (r - l + 1) / 2，避免整型溢出；
//   - 实数版固定迭代次数，区间每次减半，误差约 (r - l) / 2^iter。
// ============================================================================

namespace algo {

// ---------------------------------------------------------------------------
// 整数二分：找第一个使 pred(mid) == true 的位置（pred 单调 false→true）。
// 参数：闭区间 [l, r]；pred 单调，即存在分界点 k，mid < k 时为 false，
//       mid >= k 时为 true。
// 返回：分界点下标；若整个区间 pred 全为 false，返回 r + 1。
// ---------------------------------------------------------------------------
template<typename Pred>
int lower_bound_func(int l, int r, Pred pred) {
    if (l > r) return r + 1;  // 空区间：视为"找不到"
    int lo = l, hi = r;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;  // 向下取整，防溢出
        if (pred(mid)) {
            hi = mid;       // mid 已满足，向左收敛但保留 mid
        } else {
            lo = mid + 1;   // mid 不满足，答案必在其右侧
        }
    }
    // 循环结束时 lo == hi，指向第一个可能满足的位置
    return pred(lo) ? lo : r + 1;
}

// ---------------------------------------------------------------------------
// 整数二分：找最后一个使 pred(mid) == true 的位置（pred 单调 true→false）。
// 参数：闭区间 [l, r]；pred 单调，即存在分界点 k，mid <= k 时为 true，
//       mid > k 时为 false。
// 返回：分界点下标；若整个区间 pred 全为 false，返回 l - 1。
// ---------------------------------------------------------------------------
template<typename Pred>
int upper_bound_func(int l, int r, Pred pred) {
    if (l > r) return l - 1;  // 空区间：视为"找不到"
    int lo = l, hi = r;
    while (lo < hi) {
        int mid = lo + (hi - lo + 1) / 2;  // 向上取整，防止 lo 不动导致死循环
        if (pred(mid)) {
            lo = mid;       // mid 已满足，向右收敛但保留 mid
        } else {
            hi = mid - 1;   // mid 不满足，答案必在其左侧
        }
    }
    return pred(lo) ? lo : l - 1;
}

// ---------------------------------------------------------------------------
// 实数二分：固定迭代次数，求 pred(mid) == true 的分界点。
// 典型用法：pred(mid) = (f(mid) >= 0)，f 单调递增，求 f 的零点。
// 参数：[l, r] 为搜索区间（要求 l < r），iter 为迭代次数。
// 返回：分界点近似值（每次迭代区间减半，误差约 (r - l) / 2^iter）。
// ---------------------------------------------------------------------------
template<typename Pred>
double binary_search_real(double l, double r, Pred pred, int iter = 100) {
    for (int i = 0; i < iter; ++i) {
        double mid = l + (r - l) / 2.0;
        if (pred(mid)) {
            r = mid;   // 满足条件，分界点在左侧
        } else {
            l = mid;   // 不满足，分界点在右侧
        }
    }
    return l;  // 迭代结束后 l 与 r 已非常接近，取左端点作为答案
}

}  // namespace algo
