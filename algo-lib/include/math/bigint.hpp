#pragma once
// ============================================================================
// algo/math/bigint.hpp —— 高精度整数（非负）
// ----------------------------------------------------------------------------
// 功能：十进制 vector<int> 存储（低位在前），仅支持非负整数。
//   接口：构造（默认 0 / long long / string，自动去前导零）、比较运算符全套、
//         + - * / %（及 + - * % 的复合赋值）、to_string、is_zero、流 << >>。
// 时间复杂度：
//   - 加减 O(n)；乘法 O(n*m)（朴素，可换 FFT/分治优化）；除法/取模 O(n*m)（长除法）。
// 空间复杂度：O(n + m)。
// 使用示例：
//   #include "math/bigint.hpp"
//   algo::BigInt a("12345678901234567890"), b("9876543210");
//   algo::BigInt c = a * b;
//   std::cout << c.to_string() << '\n';
// 注意事项：
//   1. 仅支持非负整数；负数会被截断为 0（构造时）。
//   2. 减法要求 a >= b（内部用 assert 校验，调用者保证）。
//   3. 除零用 assert 校验；调用者须保证除数非零。
//   4. string 构造假设输入为合法的非负十进制串（空串视为 0），自动去前导零。
//   5. 乘法朴素 O(n*m)；n*m 很大时建议改用 FFT 优化。
// ============================================================================
#include "algo/common.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace algo {

// 高精度非负整数。
class BigInt {
public:
    BigInt() { d_.push_back(0); }

    BigInt(ll x) {
        if (x < 0) x = 0;  // 仅支持非负
        if (x == 0) {
            d_.push_back(0);
            return;
        }
        while (x > 0) {
            d_.push_back((int)(x % 10));
            x /= 10;
        }
    }

    BigInt(const std::string& s) {
        if (s.empty()) {
            d_.push_back(0);
            return;
        }
        int n = (int)s.size();
        int start = 0;
        while (start + 1 < n && s[start] == '0') ++start;  // 去前导零
        for (int j = n - 1; j >= start; --j) d_.push_back(s[j] - '0');
        trim();
    }

    // ---- 比较 ----
    bool operator<(const BigInt& o) const {
        if (d_.size() != o.d_.size()) return d_.size() < o.d_.size();
        for (int i = (int)d_.size() - 1; i >= 0; --i) {
            if (d_[i] != o.d_[i]) return d_[i] < o.d_[i];
        }
        return false;
    }
    bool operator==(const BigInt& o) const { return d_ == o.d_; }
    bool operator!=(const BigInt& o) const { return !(*this == o); }
    bool operator>(const BigInt& o) const { return o < *this; }
    bool operator<=(const BigInt& o) const { return !(o < *this); }
    bool operator>=(const BigInt& o) const { return !(*this < o); }

    // ---- 四则运算 ----
    BigInt operator+(const BigInt& o) const {
        BigInt r;
        r.d_.clear();
        int carry = 0;
        int n = (int)std::max(d_.size(), o.d_.size());
        for (int i = 0; i < n || carry; ++i) {
            int sum = carry;
            if (i < (int)d_.size()) sum += d_[i];
            if (i < (int)o.d_.size()) sum += o.d_[i];
            r.d_.push_back(sum % 10);
            carry = sum / 10;
        }
        return r;
    }

    BigInt operator-(const BigInt& o) const {
        assert(*this >= o);  // 调用者保证 a >= b
        BigInt r;
        r.d_.clear();
        int borrow = 0;
        for (int i = 0; i < (int)d_.size(); ++i) {
            int sub = d_[i] - borrow - (i < (int)o.d_.size() ? o.d_[i] : 0);
            if (sub < 0) {
                sub += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            r.d_.push_back(sub);
        }
        r.trim();
        return r;
    }

    BigInt operator*(const BigInt& o) const {
        BigInt r;
        r.d_.assign(d_.size() + o.d_.size(), 0);
        for (int i = 0; i < (int)d_.size(); ++i) {
            for (int j = 0; j < (int)o.d_.size(); ++j) {
                r.d_[i + j] += d_[i] * o.d_[j];
            }
        }
        // 统一进位（d_[i] <= 81 * 位数，通常不会溢出 int）。
        for (int i = 0; i + 1 < (int)r.d_.size(); ++i) {
            r.d_[i + 1] += r.d_[i] / 10;
            r.d_[i] %= 10;
        }
        r.trim();
        return r;
    }

    BigInt operator/(const BigInt& o) const {
        assert(!o.is_zero());  // 除零
        if (*this < o) return BigInt(0);
        BigInt q;
        q.d_.assign(d_.size(), 0);
        BigInt rem;
        for (int i = (int)d_.size() - 1; i >= 0; --i) {
            rem.d_.insert(rem.d_.begin(), d_[i]);  // rem = rem * 10 + d_[i]
            rem.trim();
            // 二分试商 0..9。
            int lo = 0, hi = 9, best = 0;
            while (lo <= hi) {
                int mid = (lo + hi) >> 1;
                BigInt t = o.mul_small(mid);
                if (t <= rem) {
                    best = mid;
                    lo = mid + 1;
                } else {
                    hi = mid - 1;
                }
            }
            q.d_[i] = best;
            rem = rem - o.mul_small(best);
        }
        q.trim();
        return q;
    }

    BigInt operator%(const BigInt& o) const {
        return *this - (*this / o) * o;
    }

    // ---- 复合赋值 ----
    BigInt& operator+=(const BigInt& o) {
        *this = *this + o;
        return *this;
    }
    BigInt& operator-=(const BigInt& o) {
        *this = *this - o;
        return *this;
    }
    BigInt& operator*=(const BigInt& o) {
        *this = *this * o;
        return *this;
    }
    BigInt& operator%=(const BigInt& o) {
        *this = *this % o;
        return *this;
    }

    // ---- 工具 ----
    std::string to_string() const {
        std::string s;
        for (int i = (int)d_.size() - 1; i >= 0; --i) s += (char)('0' + d_[i]);
        return s;
    }
    bool is_zero() const { return d_.size() == 1 && d_[0] == 0; }

    friend std::ostream& operator<<(std::ostream& os, const BigInt& x) {
        return os << x.to_string();
    }
    friend std::istream& operator>>(std::istream& is, BigInt& x) {
        std::string s;
        is >> s;
        x = BigInt(s);
        return is;
    }

private:
    std::vector<int> d_;  // 低位在前

    // 去掉高位多余的 0，但至少保留一位。
    void trim() {
        while (d_.size() > 1 && d_.back() == 0) d_.pop_back();
    }

    // 乘以单个数字（0..9），长除法内部使用。
    BigInt mul_small(int x) const {
        BigInt r;
        r.d_.assign(d_.size(), 0);
        int carry = 0;
        for (int i = 0; i < (int)d_.size(); ++i) {
            int t = d_[i] * x + carry;
            r.d_[i] = t % 10;
            carry = t / 10;
        }
        while (carry > 0) {
            r.d_.push_back(carry % 10);
            carry /= 10;
        }
        r.trim();
        return r;
    }
};

}  // namespace algo
