#pragma once
// ============================================================================
// algo/ds/monotonic.hpp —— 单调栈 / 单调队列
// ----------------------------------------------------------------------------
// 功能：
//   - 单调栈：求每个位置"左/右侧第一个更大/更小"的元素下标（无则 -1）。
//     next_greater / next_smaller / prev_greater / prev_smaller，共四个。
//   - 单调队列：滑动窗口最值。sliding_window_min / sliding_window_max。
// 时间复杂度：均为 O(n)（每个元素最多入栈/入队、出栈/出队一次）。
// 空间复杂度：O(n)。
// 使用示例：
//   std::vector<int> a = {1, 3, 2, 4};
//   auto ng = next_greater(a);        // {1, 3, 3, -1}
//   auto ps = prev_smaller(a);        // {-1, 0, 0, 2}
//   auto mn = sliding_window_min(a, 2);  // {1, 2, 2}
//   auto mx = sliding_window_max(a, 3);  // {3, 4}
// 注意事项：
//   - 所有函数下标均为 0-based（与 vector 下标一致），"无"返回 -1。
//   - "更大/更小"均为严格比较；相等不满足。
//   - 滑动窗口 k > n（或 k <= 0）时返回空 vector。
// ============================================================================

#include <deque>
#include <vector>

namespace algo {

// 每个位置右侧第一个更大的元素下标（严格更大，无则 -1）
template <typename T>
std::vector<int> next_greater(const std::vector<T>& a) {
    int n = static_cast<int>(a.size());
    std::vector<int> res(n, -1);
    std::vector<int> st;  // 单调递减栈（存下标）
    for (int i = 0; i < n; ++i) {
        while (!st.empty() && a[st.back()] < a[i]) {
            res[st.back()] = i;  // 栈顶元素第一次遇到右侧更大的
            st.pop_back();
        }
        st.push_back(i);
    }
    return res;
}

// 每个位置右侧第一个更小的元素下标（严格更小，无则 -1）
template <typename T>
std::vector<int> next_smaller(const std::vector<T>& a) {
    int n = static_cast<int>(a.size());
    std::vector<int> res(n, -1);
    std::vector<int> st;  // 单调递增栈
    for (int i = 0; i < n; ++i) {
        while (!st.empty() && a[st.back()] > a[i]) {
            res[st.back()] = i;
            st.pop_back();
        }
        st.push_back(i);
    }
    return res;
}

// 每个位置左侧第一个更大的元素下标（严格更大，无则 -1）
template <typename T>
std::vector<int> prev_greater(const std::vector<T>& a) {
    int n = static_cast<int>(a.size());
    std::vector<int> res(n, -1);
    std::vector<int> st;  // 严格递减栈
    for (int i = 0; i < n; ++i) {
        // 弹出所有 <= a[i] 的（它们不可能作为 i 或之后元素的"更大者"）
        while (!st.empty() && a[st.back()] <= a[i]) st.pop_back();
        if (!st.empty()) res[i] = st.back();
        st.push_back(i);
    }
    return res;
}

// 每个位置左侧第一个更小的元素下标（严格更小，无则 -1）
template <typename T>
std::vector<int> prev_smaller(const std::vector<T>& a) {
    int n = static_cast<int>(a.size());
    std::vector<int> res(n, -1);
    std::vector<int> st;  // 严格递增栈
    for (int i = 0; i < n; ++i) {
        while (!st.empty() && a[st.back()] >= a[i]) st.pop_back();
        if (!st.empty()) res[i] = st.back();
        st.push_back(i);
    }
    return res;
}

// 滑动窗口最小值：返回每个长度 k 窗口的最小值（共 n-k+1 个）
template <typename T>
std::vector<T> sliding_window_min(const std::vector<T>& a, int k) {
    int n = static_cast<int>(a.size());
    if (k > n || k <= 0) return {};
    std::vector<T> res;
    res.reserve(n - k + 1);
    std::deque<int> dq;  // 存下标，对应值单调递增（队首最小）
    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && a[dq.back()] >= a[i]) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();  // 移除滑出窗口的
        if (i >= k - 1) res.push_back(a[dq.front()]);
    }
    return res;
}

// 滑动窗口最大值：返回每个长度 k 窗口的最大值（共 n-k+1 个）
template <typename T>
std::vector<T> sliding_window_max(const std::vector<T>& a, int k) {
    int n = static_cast<int>(a.size());
    if (k > n || k <= 0) return {};
    std::vector<T> res;
    res.reserve(n - k + 1);
    std::deque<int> dq;  // 存下标，对应值单调递减（队首最大）
    for (int i = 0; i < n; ++i) {
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();
        if (i >= k - 1) res.push_back(a[dq.front()]);
    }
    return res;
}

}  // namespace algo
