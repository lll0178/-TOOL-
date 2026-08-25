#pragma once
// ============================================================================
// algo/math/gauss.hpp —— 高斯消元（浮点 / 模质数）与模意义行列式
// ----------------------------------------------------------------------------
// 功能：
//   - gauss(a)：浮点列主元消元解线性方程组，a 为 n x (n+1) 增广矩阵。
//   - gauss_mod(a, mod)：模质数消元解方程组（用逆元）。
//   - determinant_mod(a, mod)：模意义行列式。
// 时间复杂度：三者均为 O(n^3)。
// 空间复杂度：O(n^2)（按值传参，内部复制矩阵）。
// 使用示例：
//   #include "math/gauss.hpp"
//   std::vector<std::vector<double>> A = {{2,1,-1,8}, {-3,-1,2,-11}, {-2,1,2,-3}};
//   auto x = algo::gauss(A);          // 唯一解；无解/无穷解返回空
// 注意事项：
//   1. 返回值：唯一解返回 vector<double>；无解或无穷解返回空 vector。
//      判定规则：消元后若某行左端全 0 而右端非 0 → 无解；
//      否则若主元数 < n（存在自由变元）→ 无穷解。
//   2. gauss 用 EPS=1e-9 判断是否为零，浮点误差敏感问题请留意。
//   3. gauss_mod / determinant_mod 要求 mod 为质数；内部乘法假设 mod 不大
//      （如 <= 2e9，防止 a*b 溢出 long long）。
// ============================================================================
#include "algo/common.hpp"
#include "math/inverse.hpp"

#include <cmath>
#include <vector>

namespace algo {

// 浮点高斯消元（Gauss-Jordan 列主元）：解 n 元方程组。
inline std::vector<double> gauss(std::vector<std::vector<double>> a) {
    int n = (int)a.size();
    if (n == 0) return {};
    int m = (int)a[0].size();  // 应为 n+1
    int row = 0;
    for (int col = 0; col < n && row < n; ++col) {
        // 列主元：选该列绝对值最大的行。
        int pivot = row;
        for (int i = row + 1; i < n; ++i) {
            if (std::fabs(a[i][col]) > std::fabs(a[pivot][col])) pivot = i;
        }
        if (std::fabs(a[pivot][col]) < EPS) continue;  // 该列下方全 0
        std::swap(a[row], a[pivot]);
        double div = a[row][col];
        for (int j = col; j < m; ++j) a[row][j] /= div;
        for (int i = 0; i < n; ++i) {
            if (i == row || std::fabs(a[i][col]) < EPS) continue;
            double f = a[i][col];
            for (int j = col; j < m; ++j) a[i][j] -= f * a[row][j];
        }
        ++row;
    }
    // 检查矛盾：剩余行左端全 0 但右端非 0 → 无解。
    for (int i = row; i < n; ++i) {
        bool all_zero = true;
        for (int j = 0; j < n; ++j) {
            if (std::fabs(a[i][j]) > EPS) {
                all_zero = false;
                break;
            }
        }
        if (all_zero && std::fabs(a[i][n]) > EPS) return {};  // 0 = 非 0
    }
    if (row < n) return {};  // 存在自由变元 → 无穷解
    std::vector<double> x(n);
    for (int i = 0; i < n; ++i) x[i] = a[i][n];
    return x;
}

// 模质数高斯消元：解 n 元方程组 mod mod。
inline std::vector<ll> gauss_mod(std::vector<std::vector<ll>> a, ll mod) {
    int n = (int)a.size();
    if (n == 0) return {};
    int m = (int)a[0].size();
    int row = 0;
    for (int col = 0; col < n && row < n; ++col) {
        int pivot = -1;
        for (int i = row; i < n; ++i) {
            if (a[i][col] % mod != 0) {
                pivot = i;
                break;
            }
        }
        if (pivot == -1) continue;  // 该列下方全 0
        std::swap(a[row], a[pivot]);
        // 归一化主元行到 [0, mod)。
        for (int j = col; j < m; ++j) {
            a[row][j] %= mod;
            if (a[row][j] < 0) a[row][j] += mod;
        }
        ll inv = inv_exgcd(a[row][col], mod);  // mod 质数，主元非零必可逆
        for (int j = col; j < m; ++j) a[row][j] = a[row][j] * inv % mod;
        // 消去其它行的当前列。
        for (int i = 0; i < n; ++i) {
            if (i == row) continue;
            ll f = a[i][col] % mod;
            if (f < 0) f += mod;
            if (f == 0) continue;
            for (int j = col; j < m; ++j) {
                a[i][j] = (a[i][j] - f * a[row][j]) % mod;
                if (a[i][j] < 0) a[i][j] += mod;
            }
        }
        ++row;
    }
    for (int i = row; i < n; ++i) {
        bool all_zero = true;
        for (int j = 0; j < n; ++j) {
            if (a[i][j] % mod != 0) {
                all_zero = false;
                break;
            }
        }
        if (all_zero && a[i][n] % mod != 0) return {};  // 无解
    }
    if (row < n) return {};  // 无穷解（非唯一）
    std::vector<ll> x(n);
    for (int i = 0; i < n; ++i) {
        x[i] = a[i][n] % mod;
        if (x[i] < 0) x[i] += mod;
    }
    return x;
}

// 模质数行列式：消元求 det，行交换记录符号。
inline ll determinant_mod(std::vector<std::vector<ll>> a, ll mod) {
    int n = (int)a.size();
    if (n == 0) return 1 % mod;
    ll det = 1 % mod;
    for (int col = 0; col < n; ++col) {
        int pivot = -1;
        for (int i = col; i < n; ++i) {
            if (a[i][col] % mod != 0) {
                pivot = i;
                break;
            }
        }
        if (pivot == -1) return 0;  // 该列全 0 → 行列式为 0
        if (pivot != col) {
            std::swap(a[pivot], a[col]);
            det = (mod - det) % mod;  // 行交换变号
        }
        // 主元行归一化到 [0, mod)。
        for (int j = col; j < n; ++j) {
            a[col][j] %= mod;
            if (a[col][j] < 0) a[col][j] += mod;
        }
        ll piv = a[col][col];
        det = det * piv % mod;
        ll inv = inv_exgcd(piv, mod);
        for (int i = col + 1; i < n; ++i) {
            ll fc = a[i][col] % mod;
            if (fc < 0) fc += mod;
            ll f = fc * inv % mod;
            if (f == 0) continue;
            for (int j = col; j < n; ++j) {
                a[i][j] = (a[i][j] - f * a[col][j]) % mod;
                if (a[i][j] < 0) a[i][j] += mod;
            }
        }
    }
    return det;
}

}  // namespace algo
