#pragma once
// ============================================================================
// misc/random.hpp —— 随机数
// ----------------------------------------------------------------------------
// 功能：封装 std::mt19937_64 的随机数类，提供闭区间整数、闭区间实数随机与
//       数组洗牌（shuffle）。
// 时间复杂度：每次取随机数 O(1)；shuffle O(n)。
// 空间复杂度：O(1)（引擎内部状态）。
// 使用示例：
//   #include "misc/random.hpp"
//   algo::Random rng;                       // 默认随机种子
//   int a = rng.next_int(1, 100);           // [1, 100]
//   long long b = rng.next_ll(1, 1000000000000LL);
//   double d = rng.next_real(0.0, 1.0);     // [0.0, 1.0)
//   std::vector<int> v = {1,2,3,4,5};
//   rng.shuffle(v);
// 注意事项：
//   - 所有整数区间均为闭区间 [l, r]，要求 l <= r，否则行为未定义。
//   - next_real 返回 [l, r)（半开区间，标准 uniform_real_distribution 语义）。
//   - 默认构造使用 std::random_device 播种；个别平台 random_device 可能退化或
//     抛异常，此时退回时钟种子。
// ============================================================================
#include "algo/common.hpp"

#include <chrono>
#include <random>
#include <vector>

namespace algo {

// 随机数类：内部使用 std::mt19937_64（64 位梅森旋转）
class Random {
public:
    // 默认构造：用随机设备（必要时退回时钟）播种
    Random() : rng_(make_default_seed()) {}

    // 用指定种子构造（可复现随机序列）
    explicit Random(ull seed) : rng_(seed) {}

    // 闭区间 [l, r] 随机整数
    int next_int(int l, int r) {
        std::uniform_int_distribution<int> dist(l, r);
        return dist(rng_);
    }

    // 闭区间 [l, r] 随机 long long
    ll next_ll(ll l, ll r) {
        std::uniform_int_distribution<ll> dist(l, r);
        return dist(rng_);
    }

    // 半开区间 [l, r) 随机 double
    double next_real(double l, double r) {
        std::uniform_real_distribution<double> dist(l, r);
        return dist(rng_);
    }

    // 洗牌（Fisher-Yates，由 std::shuffle 实现）
    template <typename T>
    void shuffle(std::vector<T>& v) {
        std::shuffle(v.begin(), v.end(), rng_);
    }

private:
    std::mt19937_64 rng_;

    static ull make_default_seed() {
        try {
            std::random_device rd;
            // 组合两次 random_device 输出，得到 64 位种子
            return (static_cast<ull>(rd()) << 32) ^ static_cast<ull>(rd());
        } catch (...) {
            // random_device 不可用时退回时钟计数
            return static_cast<ull>(
                std::chrono::steady_clock::now().time_since_epoch().count());
        }
    }
};

}  // namespace algo
