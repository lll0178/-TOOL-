#pragma once
// ============================================================================
// algo/meet_in_middle.hpp —— 折半搜索（Meet-in-the-Middle）
// ----------------------------------------------------------------------------
// 功能：
//   当朴素枚举 2^n 个状态不可接受时，把原集合分成两半分别枚举，再合并，
//   把复杂度从 2^n 降到约 2^(n/2) 级别。
//   - subset_sums(a)：生成数组 a 的所有子集和（含空集，值为 0）；
//   - closest_subset_sum(a, target)：在 a 的所有子集和中，找"最接近 target"
//     的值（允许负数），返回该最接近的子集和。
// 时间复杂度：
//   subset_sums：O(2^n) 时间、O(2^n) 空间；
//   closest_subset_sum：O(2^(n/2) log 2^(n/2)) 时间、O(2^(n/2)) 空间。
// 空间复杂度：同上。
// 使用示例：
//   // 求数组 {5, -7, 3, 5} 的子集和中，最接近 1 的那个（答案 1 = 5-7+3）
//   vector<long long> a = {5, -7, 3, 5};
//   long long ans = algo::closest_subset_sum(a, 1LL);
//
//   // 生成所有子集和
//   vector<long long> sums = algo::subset_sums(a);  // 共 16 个，含 0
// 注意事项：
//   - n 较大时 2^n 会爆炸：subset_sums 仅适用于 n <= 20 左右；
//     closest_subset_sum 适用于 n <= 40 左右（两半各 2^20）；
//   - 支持负数：合并时对右半部分排序，用 lower_bound 找 target - s 的
//     最接近位置，并同时检查其前驱，保证负数场景同样正确；
//   - 元素类型 T 需支持加、减、比较（内置整数 / long long / double 均可）；
//   - 空数组的子集和集合为 {0}，closest_subset_sum 对空数组返回 0。
// ============================================================================

#include <vector>
#include <algorithm>  // std::sort / std::lower_bound
#include <cstddef>    // std::size_t

namespace algo {

// 生成所有子集和（含空集 0）
template<typename T>
std::vector<T> subset_sums(const std::vector<T>& a) {
    std::vector<T> res;
    // 共 2^n 个子集和；仅在 n 较小（< 63）时预分配，避免移位溢出
    if (a.size() < 63) res.reserve(std::size_t(1) << a.size());
    res.push_back(T(0));  // 空集
    // 逐个加入元素：每加入一个元素，现有集合翻倍（选/不选该元素）
    for (std::size_t i = 0; i < a.size(); ++i) {
        std::size_t sz = res.size();
        for (std::size_t j = 0; j < sz; ++j) {
            res.push_back(res[j] + a[i]);
        }
    }
    return res;
}

namespace meet_middle_detail {

// 判断 cand 是否比 best 更接近 target（比较绝对距离，兼容负数）
template<typename T>
bool closer(T cand, T best, T target) {
    T d_cand = cand < target ? target - cand : cand - target;
    T d_best = best < target ? target - best : best - target;
    return d_cand < d_best;
}

}  // namespace meet_middle_detail

// 折半搜索求最接近 target 的子集和（含负数同样正确）
template<typename T>
T closest_subset_sum(const std::vector<T>& a, T target) {
    int n = (int)a.size();
    if (n == 0) return T(0);
    int mid = n / 2;
    // 分成左右两半
    std::vector<T> left(a.begin(), a.begin() + mid);
    std::vector<T> right(a.begin() + mid, a.end());
    std::vector<T> sum_l = subset_sums(left);
    std::vector<T> sum_r = subset_sums(right);
    std::sort(sum_r.begin(), sum_r.end());

    T best = sum_l[0] + sum_r[0];  // 初始候选（两侧都含空集 0，一定非空）
    for (std::size_t i = 0; i < sum_l.size(); ++i) {
        T need = target - sum_l[i];
        // 在右半部分找第一个 >= need 的位置
        typename std::vector<T>::const_iterator it =
            std::lower_bound(sum_r.begin(), sum_r.end(), need);
        // 检查 it 本身（>= need 中最小的）
        if (it != sum_r.end()) {
            T cand = sum_l[i] + *it;
            if (meet_middle_detail::closer(cand, best, target)) best = cand;
        }
        // 检查 it 的前驱（< need 中最大的）
        if (it != sum_r.begin()) {
            typename std::vector<T>::const_iterator prev = it;
            --prev;
            T cand = sum_l[i] + *prev;
            if (meet_middle_detail::closer(cand, best, target)) best = cand;
        }
    }
    return best;
}

}  // namespace algo
