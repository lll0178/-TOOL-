#pragma once
// ============================================================================
// algo/sort.hpp —— 排序与选择（归并排序 / 逆序对 / 快速排序 / 第 k 小）
// ----------------------------------------------------------------------------
// 功能：
//   - merge_sort(a)：稳定归并排序，原地排序，O(n log n)；
//   - inversion_count(a)：归并排序同时统计逆序对数量（不修改原数组）；
//   - quick_sort(a, l, r)：随机基准快速排序（闭区间 [l, r]）；
//   - quick_select(a, k)：求第 k 小元素（k 从 0 开始），期望 O(n)，会打乱数组。
// 时间复杂度：
//   merge_sort / inversion_count：O(n log n)；
//   quick_sort：O(n log n) 期望，O(n^2) 最坏（随机基准下概率极低）；
//   quick_select：O(n) 期望，O(n^2) 最坏。
// 空间复杂度：
//   merge_sort / inversion_count：O(n) 辅助数组；
//   quick_sort / quick_select：O(log n) 递归栈（原地划分）。
// 使用示例：
//   vector<int> a = {3, 1, 4, 1, 5};
//   algo::merge_sort(a);                       // a 变为升序
//   long long inv = algo::inversion_count(a);  // 求逆序对（原数组不变）
//   algo::quick_sort(a, 0, (int)a.size() - 1); // 对 a 快速排序
//   int kth = algo::quick_select(a, 2);        // 第 3 小（k=2），a 被改动
// 注意事项：
//   - inversion_count 内部拷贝数组，入参 a 不被修改；
//   - quick_sort 的区间 [l, r] 为闭区间，调用前保证 0 <= l <= r < n；
//   - quick_select 会破坏原数组元素顺序；k 必须满足 0 <= k < a.size()，
//     且 a 非空，否则行为未定义；
//   - 随机基准使用 std::rand（必要时请先自行 std::srand 设置种子）；
//   - 逆序对数量最大为 n(n-1)/2，可能超出 int，故返回 long long；
//   - 所有操作要求元素类型 T 支持 <、<=、> 比较与拷贝。
// ============================================================================

#include <vector>
#include <cstdlib>    // std::rand / std::srand
#include <utility>    // std::swap
#include "algo/common.hpp"  // algo::ll

namespace algo {

namespace sort_detail {

// 归并排序递归实现：对 a 的闭区间 [l, r] 排序，tmp 为辅助数组
template<typename T>
void merge_sort_impl(std::vector<T>& a, std::vector<T>& tmp, int l, int r) {
    if (l >= r) return;
    int mid = l + (r - l) / 2;
    merge_sort_impl(a, tmp, l, mid);
    merge_sort_impl(a, tmp, mid + 1, r);
    // 合并两个已排序区间 [l, mid] 与 [mid+1, r]
    int i = l, j = mid + 1, k = l;
    while (i <= mid && j <= r) {
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];  // <= 保证稳定性
    }
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (int p = l; p <= r; ++p) a[p] = tmp[p];
}

// 归并排序并统计逆序对数量
template<typename T>
ll inversion_impl(std::vector<T>& a, std::vector<T>& tmp, int l, int r) {
    if (l >= r) return 0;
    int mid = l + (r - l) / 2;
    ll cnt = inversion_impl(a, tmp, l, mid) + inversion_impl(a, tmp, mid + 1, r);
    int i = l, j = mid + 1, k = l;
    while (i <= mid && j <= r) {
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            // a[j] 小于左侧剩余的所有元素，构成 (mid - i + 1) 个逆序对
            tmp[k++] = a[j++];
            cnt += (ll)(mid - i + 1);
        }
    }
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= r) tmp[k++] = a[j++];
    for (int p = l; p <= r; ++p) a[p] = tmp[p];
    return cnt;
}

}  // namespace sort_detail

// 稳定归并排序（原地排序，O(n) 辅助空间）
template<typename T>
void merge_sort(std::vector<T>& a) {
    int n = (int)a.size();
    if (n <= 1) return;
    std::vector<T> tmp(n);
    sort_detail::merge_sort_impl(a, tmp, 0, n - 1);
}

// 统计逆序对数量：返回 long long；不修改原数组
template<typename T>
ll inversion_count(const std::vector<T>& a) {
    int n = (int)a.size();
    if (n <= 1) return 0;
    std::vector<T> b = a;          // 拷贝一份，避免修改入参
    std::vector<T> tmp(n);
    return sort_detail::inversion_impl(b, tmp, 0, n - 1);
}

// 随机基准快速排序：对闭区间 [l, r] 排序
template<typename T>
void quick_sort(std::vector<T>& a, int l, int r) {
    if (l >= r) return;
    // 随机选取基准并与最左元素交换（降低有序/逆序输入下退化到 O(n^2) 的概率）
    int pivot_pos = l + std::rand() % (r - l + 1);
    std::swap(a[pivot_pos], a[l]);
    T pivot = a[l];
    int i = l + 1, j = r;
    // 划分：使 [l, j-1] <= pivot，[j+1, r] > pivot
    while (i <= j) {
        while (i <= j && a[i] <= pivot) ++i;
        while (i <= j && a[j] > pivot) --j;
        if (i < j) std::swap(a[i], a[j]);
    }
    std::swap(a[l], a[j]);  // j 为 pivot 最终位置
    quick_sort(a, l, j - 1);
    quick_sort(a, j + 1, r);
}

// 求第 k 小（k 从 0 开始）：期望 O(n)，会打乱数组
template<typename T>
T quick_select(std::vector<T>& a, int k) {
    int n = (int)a.size();
    // 前置条件：a 非空且 0 <= k < n
    int l = 0, r = n - 1;
    while (true) {
        if (l == r) return a[l];
        // 随机基准划分（与 quick_sort 相同的划分过程）
        int pivot_pos = l + std::rand() % (r - l + 1);
        std::swap(a[pivot_pos], a[l]);
        T pivot = a[l];
        int i = l + 1, j = r;
        while (i <= j) {
            while (i <= j && a[i] <= pivot) ++i;
            while (i <= j && a[j] > pivot) --j;
            if (i < j) std::swap(a[i], a[j]);
        }
        std::swap(a[l], a[j]);
        if (j == k) return a[j];
        if (k < j) r = j - 1;   // 目标在左半区
        else l = j + 1;         // 目标在右半区
    }
}

}  // namespace algo
