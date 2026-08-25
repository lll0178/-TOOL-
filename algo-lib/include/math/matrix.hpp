#pragma once
// ============================================================================
// algo/math/matrix.hpp —— 矩阵与矩阵快速幂
// ----------------------------------------------------------------------------
// 功能：动态矩阵（vector<vector<ll>>），带模数，支持乘法与快速幂。
//   接口：Matrix(int n, ll mod) / Matrix(int n, int m, ll mod)、unit、
//         at、operator*、pow、rows、cols、mod。
// 时间复杂度：乘法 O(n*m*p)（n x m 乘 m x p）；pow O(n^3 log e)（方阵）。
// 空间复杂度：O(n*m)。
// 使用示例（斐波那契 F(n)，F(0)=0, F(1)=1）：
//   #include "math/matrix.hpp"
//   algo::Matrix A(2, 2, mod);
//   A.at(0,0)=1; A.at(0,1)=1; A.at(1,0)=1; A.at(1,1)=0;
//   algo::Matrix An = A.pow(n);              // [[F(n+1), F(n)], [F(n), F(n-1)]]
//   ll fn = An.at(0, 1);                     // 第 n 项
// 注意事项：
//   1. pow 只对方阵有意义（n_ == m_），调用者须保证。
//   2. 元素请保持在 [0, mod) 内（at() 不做自动取模）。
//   3. 乘法内部假设 mod 不大（如 <= 2e9），否则 a*b 可能溢出 long long。
// ============================================================================
#include "algo/common.hpp"

#include <vector>

namespace algo {

// 模意义动态矩阵。
class Matrix {
public:
    Matrix(int n, int m, ll mod) : n_(n), m_(m), mod_(mod), a_(n, std::vector<ll>(m, 0)) {}
    Matrix(int n, ll mod) : Matrix(n, n, mod) {}  // 方阵

    // 单位矩阵（方阵）。
    static Matrix unit(int n, ll mod) {
        Matrix e(n, n, mod);
        for (int i = 0; i < n; ++i) e.a_[i][i] = 1 % mod;
        return e;
    }

    ll& at(int i, int j) { return a_[i][j]; }
    const ll& at(int i, int j) const { return a_[i][j]; }

    // 矩阵乘法：前提 this->cols() == o.rows()。
    Matrix operator*(const Matrix& o) const {
        Matrix r(n_, o.m_, mod_);
        for (int i = 0; i < n_; ++i) {
            for (int k = 0; k < m_; ++k) {
                if (a_[i][k] == 0) continue;
                for (int j = 0; j < o.m_; ++j) {
                    r.a_[i][j] = (r.a_[i][j] + a_[i][k] * o.a_[k][j]) % mod_;
                }
            }
        }
        return r;
    }

    // 矩阵快速幂：要求方阵。
    Matrix pow(ll e) const {
        Matrix res = unit(n_, mod_);
        Matrix base = *this;
        while (e > 0) {
            if (e & 1LL) res = res * base;
            base = base * base;
            e >>= 1;
        }
        return res;
    }

    int rows() const { return n_; }
    int cols() const { return m_; }
    ll mod() const { return mod_; }

private:
    int n_, m_;
    ll mod_;
    std::vector<std::vector<ll>> a_;
};

}  // namespace algo
