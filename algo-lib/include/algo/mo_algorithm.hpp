#pragma once
// ============================================================================
// algo/mo_algorithm.hpp —— 莫队算法（离线区间查询）
// ----------------------------------------------------------------------------
// 功能：
//   离线回答多个区间查询。当"已知 [l, r] 的答案，可以 O(1) 地扩展到相邻区间
//   [l-1, r]、[l+1, r]、[l, r-1]、[l, r+1]"时，莫队通过把询问排序，用左右
//   指针在序列上滑动，把复杂度从朴素 O(n * m) 降到约 O((n + m) * sqrt(m))。
//   典型应用：区间不同数个数、区间众数、区间内某值出现次数等。
// 时间复杂度：
//   设块大小 B ≈ n / sqrt(m)，总复杂度 O(m log m + (n + m) * B)，
//   近似 O((n + m) * sqrt(m))。
// 空间复杂度：O(m)（答案数组 + 询问数组；add/del 的计数结构由调用方维护）。
// 使用示例（区间不同数个数）：
//   int n = 6;
//   vector<int> a = {1, 2, 1, 3, 2, 1};
//   vector<algo::MoQuery> qs = { {0, 3, 0}, {1, 4, 1}, {2, 5, 2} };
//   vector<int> cnt(4, 0);   // 值域 [0, 3]
//   long long cur = 0;       // 当前区间内不同数的个数
//   auto add = [&](int idx) { if (++cnt[a[idx]] == 1) ++cur; };
//   auto del = [&](int idx) { if (--cnt[a[idx]] == 0) --cur; };
//   auto get = [&]() { return cur; };
//   vector<long long> ans = algo::mo_solve(n, qs, add, del, get);
//   // ans = {3, 3, 3}
// 注意事项：
//   - MoQuery 的 l、r 为 0-based 闭区间，需保证 0 <= l <= r < n；
//   - add(idx) / del(idx) 接收的是元素下标 idx（闭区间端点包含在内）；
//   - 初始区间为空（cur_l = 0, cur_r = -1），调用方计数结构需与之对应；
//   - qs 为空时直接返回空答案数组，不会除零；
//   - 指针移动顺序固定为"先扩张后收缩"，保证任意时刻区间合法；
//   - 奇偶优化：l 所在块为奇数时 r 降序，减少指针来回移动的代价；
//   - Get 需返回 long long；若答案非整数，请改用其它返回方式。
// ============================================================================

#include <vector>
#include <algorithm>  // std::sort
#include <cmath>      // std::sqrt
#include "algo/common.hpp"  // algo::ll

namespace algo {

// 莫队询问：0-based 闭区间 [l, r]，id 为原始询问编号
struct MoQuery {
    int l, r, id;
};

// 莫队主流程。
// Add / Del：接收下标 idx 的可调用对象；Get：返回当前答案（long long）。
// 返回：长度与 qs 相同的答案数组，ans[qs[i].id] 为第 i 个询问的答案。
template<typename Add, typename Del, typename Get>
std::vector<ll> mo_solve(int n, std::vector<MoQuery> qs, Add add, Del del, Get get) {
    std::vector<ll> ans(qs.size());
    if (qs.empty()) return ans;  // 无询问直接返回，避免除零

    // 块大小：n / sqrt(m)，至少为 1（防止块过小或除零）
    int block = (int)(n / std::sqrt((double)qs.size()));
    if (block < 1) block = 1;

    // 排序：按 l 所在块分组，块内按 r 排序；奇数块 r 降序（奇偶优化）
    std::sort(qs.begin(), qs.end(), [&](const MoQuery& x, const MoQuery& y) {
        int bx = x.l / block;
        int by = y.l / block;
        if (bx != by) return bx < by;
        if (bx & 1) return x.r > y.r;   // 奇数块：r 降序
        return x.r < y.r;               // 偶数块：r 升序
    });

    int cur_l = 0, cur_r = -1;  // 当前维护的闭区间为空
    for (const MoQuery& q : qs) {
        // 先扩张后收缩：先把区间扩到覆盖 q，再收缩掉多余部分
        while (cur_l > q.l) add(--cur_l);
        while (cur_r < q.r) add(++cur_r);
        while (cur_l < q.l) del(cur_l++);
        while (cur_r > q.r) del(cur_r--);
        ans[q.id] = get();
    }
    return ans;
}

}  // namespace algo
