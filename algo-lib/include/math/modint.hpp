#pragma once
// ============================================================================
// algo/math/modint.hpp —— 编译期模数类
// ----------------------------------------------------------------------------
// 功能：模板类 ModInt<MOD>，所有运算即时取模并修正负数，使用编译期模数 MOD。
//   支持 + - * / 及复合赋值、取负、前置/后置 ++ --、== !=、与 long long 的
//   隐式转换、流输入输出、友元 pow(ModInt, ll)。
// 时间复杂度：+ - O(1)；* O(1)；/ O(log MOD)（费马小定理）。
// 空间复杂度：O(1)。
// 使用示例：
//   #include "math/modint.hpp"
//   using Mint = algo::ModInt<1000000007>;
//   Mint a = 5, b = 2;
//   Mint c = a / b;                 // 5 * inv(2)
//   Mint d = pow(a, 10LL);          // pow 为友元，经 ADL 查找
// 注意事项：
//   1. 除法用费马小定理（要求 MOD 为质数）；对 0 取逆未定义，勿对 0 做除法。
//   2. 内部乘法 v * v 在 MOD <= 2e9 时不会溢出 64 位（MOD^2 <= 4e18）。
//   3. 提供了到 long long 的隐式转换（双向）；混用 ModInt 与原生整数时，
//      个别表达式可能产生重载歧义，建议保持操作数类型一致或用 ModInt(...) 显式转换。
//   4. pow 是友元函数（经 ADL 查找，无法用 algo::pow(...) 限定名调用）：
//      请用 pow(m, n) 且指数用 long long；若同时引入 <cmath>，pow(m, 整型字面量)
//      可能与 std::pow 产生重载歧义。
// ============================================================================
#include "algo/common.hpp"

#include <istream>
#include <ostream>

namespace algo {

template <int MOD>
struct ModInt {
    ModInt(ll x = 0) : v_(x % MOD) {
        if (v_ < 0) v_ += MOD;  // 负数修正
    }

    ModInt& operator+=(const ModInt& o) {
        v_ += o.v_;
        if (v_ >= MOD) v_ -= MOD;
        return *this;
    }
    ModInt& operator-=(const ModInt& o) {
        v_ -= o.v_;
        if (v_ < 0) v_ += MOD;
        return *this;
    }
    ModInt& operator*=(const ModInt& o) {
        v_ = v_ * o.v_ % MOD;
        return *this;
    }
    ModInt& operator/=(const ModInt& o) { return *this *= o.inv(); }

    ModInt operator+(const ModInt& o) const {
        ModInt r = *this;
        return r += o;
    }
    ModInt operator-(const ModInt& o) const {
        ModInt r = *this;
        return r -= o;
    }
    ModInt operator*(const ModInt& o) const {
        ModInt r = *this;
        return r *= o;
    }
    ModInt operator/(const ModInt& o) const {
        ModInt r = *this;
        return r /= o;
    }
    ModInt operator-() const { return ModInt(-v_); }

    ModInt& operator++() {
        v_ += 1;
        if (v_ >= MOD) v_ -= MOD;
        return *this;
    }
    ModInt operator++(int) {
        ModInt t = *this;
        ++(*this);
        return t;
    }
    ModInt& operator--() {
        v_ -= 1;
        if (v_ < 0) v_ += MOD;
        return *this;
    }
    ModInt operator--(int) {
        ModInt t = *this;
        --(*this);
        return t;
    }

    bool operator==(const ModInt& o) const { return v_ == o.v_; }
    bool operator!=(const ModInt& o) const { return v_ != o.v_; }

    // 到 long long 的隐式转换（构造函数已提供反向隐式转换）。
    operator ll() const { return v_; }

    // 逆元：费马小定理，要求 MOD 为质数且 *this 非零。
    ModInt inv() const { return pow(*this, MOD - 2); }

    // 幂：指数 b >= 0。
    friend ModInt pow(ModInt a, ll b) {
        ModInt res(1);
        while (b > 0) {
            if (b & 1LL) res *= a;
            a *= a;
            b >>= 1;
        }
        return res;
    }

    friend std::istream& operator>>(std::istream& is, ModInt& x) {
        ll t;
        is >> t;
        x = ModInt(t);
        return is;
    }
    friend std::ostream& operator<<(std::ostream& os, const ModInt& x) {
        return os << x.v_;
    }

private:
    ll v_;  // 恒保持 [0, MOD)
};

}  // namespace algo
