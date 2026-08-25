#pragma once
// ============================================================================
// algo/math/linear_basis.hpp —— 异或线性基
// ----------------------------------------------------------------------------
// 功能：维护一个集合的"异或线性基"，支持插入、判定、最大异或、第 k 小异或。
//   接口：insert / can / max_xor / kth / size。
// 时间复杂度：插入、can、max_xor 均 O(61)；kth O(61^2)（重构阶梯型）。
// 空间复杂度：O(61)。
// 使用示例：
//   #include "math/linear_basis.hpp"
//   algo::LinearBasis lb;
//   lb.insert(5); lb.insert(6); lb.insert(9);
//   ll mx = lb.max_xor();   // 最大异或值
//   ll k1 = lb.kth(1);      // 第 1 小的异或值
// 注意事项：
//   1. 仅处理 61 位整数（位编号 0..60），即值域 < 2^61。
//   2. kth 的 k 从 1 开始。若插入过程中出现过"消成 0"（集合线性相关，
//      即可异或出 0），则 kth(1) 返回 0，其余结果整体后移（内部 k 先减 1）。
//   3. 若集合线性无关，则 0 只能由空集得到、不计入答案，kth(1) 为最小非零值。
// ============================================================================
#include "algo/common.hpp"

#include <vector>

namespace algo {

// 异或线性基（61 位）。
class LinearBasis {
public:
    static const int MAX_BIT = 61;

    LinearBasis() : cnt_(0), has_zero_(false) {
        for (int i = 0; i < MAX_BIT; ++i) basis_[i] = 0;
    }

    // 插入一个数；返回后若该数被消成 0，说明集合可异或出 0。
    void insert(ll x) {
        for (int i = MAX_BIT - 1; i >= 0; --i) {
            if (((x >> i) & 1LL) == 0) continue;
            if (basis_[i]) {
                x ^= basis_[i];
            } else {
                basis_[i] = x;
                ++cnt_;
                return;
            }
        }
        has_zero_ = true;  // x 被消成 0 → 集合线性相关
    }

    // 判断 x 是否能由当前集合异或得到。
    bool can(ll x) const {
        for (int i = MAX_BIT - 1; i >= 0; --i) {
            if (((x >> i) & 1LL) == 0) continue;
            if (!basis_[i]) return false;
            x ^= basis_[i];
        }
        return true;
    }

    // 返回所有异或组合中的最大值。
    ll max_xor() const {
        ll res = 0;
        for (int i = MAX_BIT - 1; i >= 0; --i) {
            if ((res ^ basis_[i]) > res) res ^= basis_[i];
        }
        return res;
    }

    // 返回第 k 小的异或值，k 从 1 开始；越界返回 -1。
    ll kth(ll k) const {
        if (k <= 0) return -1;
        // 1. 把基化为阶梯型：每个非零基向量的最高位互异，且低位不在其它向量中出现。
        ll tmp[MAX_BIT];
        for (int i = 0; i < MAX_BIT; ++i) tmp[i] = basis_[i];
        for (int i = 0; i < MAX_BIT; ++i) {
            for (int j = i - 1; j >= 0; --j) {
                if ((tmp[i] >> j) & 1LL) tmp[i] ^= tmp[j];
            }
        }
        std::vector<ll> d;
        for (int i = 0; i < MAX_BIT; ++i) {
            if (tmp[i]) d.push_back(tmp[i]);
        }
        // 2. 若可异或出 0（插入失败过），0 是第 1 小，其余结果整体后移。
        if (has_zero_) {
            if (k == 1) return 0;
            --k;
        }
        // 此时有效区间为 [1, 2^|d| - 1]。
        if ((unsigned long long)k >= (1ULL << d.size())) return -1;
        ll res = 0;
        for (int i = 0; i < (int)d.size(); ++i) {
            if ((k >> i) & 1LL) res ^= d[i];
        }
        return res;
    }

    // 当前基的个数（秩）。
    int size() const { return cnt_; }

private:
    ll basis_[MAX_BIT];
    int cnt_;        // 基向量个数
    bool has_zero_;  // 是否出现过"消成 0"（集合可异或出 0）
};

}  // namespace algo
