#pragma once
// ============================================================================
// algo/discretize.hpp —— 离散化（值 → 0-based 排名）
// ----------------------------------------------------------------------------
// 功能：
//   把一组可能数值范围很大（或含负数、浮点、大整数）的值压缩成紧凑的
//   0-based 排名，常用于：线段树/树状数组在值域上建树、坐标压缩、桶计数等
//   场景，把 O(值域) 的空间/时间压到 O(n)。
//   接口：
//   - build(a)：对数组 a 去重排序，建立"值 → 排名"映射；
//   - rank_of(v)：返回 v 的 0-based 排名，不存在返回 -1；
//   - values()：返回去重后的有序值数组（排名 i 对应的原始值）；
//   - size()：返回去重后的元素个数。
// 时间复杂度：
//   build：O(n log n)（排序 + 去重）；
//   rank_of：O(log n) 每次（二分查找）。
// 空间复杂度：O(n)。
// 使用示例：
//   // 把原数组每个元素替换成它的排名
//   vector<long long> a = {10, -3, 10, 5, -3, 0};
//   algo::Discrete<long long> disc;
//   disc.build(a);
//   for (long long& x : a) x = disc.rank_of(x);  // 得到 {3, 0, 3, 2, 0, 1}
//   // disc.values()[i] 即为排名 i 对应的原始值，可用于还原
// 注意事项：
//   - rank_of 内部用二分，要求 T 支持 < 与 == 比较（内置数值/字符串均可）；
//   - build 会丢弃重复值，排名是去重后的下标；
//   - 对空数组 build 后 size() 为 0，rank_of 恒返回 -1；
//   - 模板类型 T 需可复制、可排序；返回值 0-based（最小值为排名 0）。
// ============================================================================

#include <vector>
#include <algorithm>  // std::sort / std::unique / std::lower_bound

namespace algo {

// 离散化辅助类：维护"去重有序值"并支持值到排名的映射
template<typename T>
class Discrete {
public:
    Discrete() {}

    // 根据数组 a 建立离散化映射（去重 + 排序）
    void build(const std::vector<T>& a) {
        vals_ = a;
        std::sort(vals_.begin(), vals_.end());
        vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    }

    // 返回 v 的 0-based 排名；若 v 不在集合中返回 -1
    int rank_of(const T& v) const {
        // 找到第一个 >= v 的位置
        typename std::vector<T>::const_iterator it =
            std::lower_bound(vals_.begin(), vals_.end(), v);
        // 用 !(*it == v) 而非 *it != v，兼容只重载 == 的自定义类型
        if (it == vals_.end() || !(*it == v)) return -1;
        return (int)(it - vals_.begin());
    }

    // 去重后的有序值数组（排名 i 对应的原始值）
    const std::vector<T>& values() const { return vals_; }

    // 去重后的元素个数
    int size() const { return (int)vals_.size(); }

private:
    std::vector<T> vals_;  // 去重排序后的值
};

}  // namespace algo
