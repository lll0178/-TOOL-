// ============================================================================
// tests/smoke.cpp —— 冒烟测试
// ----------------------------------------------------------------------------
// 功能：include 全部头文件（全库可编译性检查），并对常用板子做正确性断言。
// 运行方式：
//   g++ -std=c++17 -O2 -Wall -Wextra -I include tests/smoke.cpp -o build/smoke && ./build/smoke
//   cl /std:c++17 /EHsc /I include tests\smoke.cpp /Fe:build\smoke.exe
// ============================================================================
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <string>
#include <vector>

// ---- 基础算法 ----
#include "algo/binary_search.hpp"
#include "algo/three_point.hpp"
#include "algo/sort.hpp"
#include "algo/discretize.hpp"
#include "algo/mo_algorithm.hpp"
#include "algo/meet_in_middle.hpp"
#include "algo/lis_lcs.hpp"
// ---- 数据结构 ----
#include "ds/union_find.hpp"
#include "ds/fenwick.hpp"
#include "ds/segment_tree.hpp"
#include "ds/persistent_segment_tree.hpp"
#include "ds/sparse_table.hpp"
#include "ds/treap.hpp"
#include "ds/splay.hpp"
#include "ds/hld.hpp"
#include "ds/cartesian_tree.hpp"
#include "ds/kdtree.hpp"
#include "ds/lct.hpp"
#include "ds/block.hpp"
#include "ds/monotonic.hpp"
// ---- 图论 ----
#include "graph/dijkstra.hpp"
#include "graph/spfa.hpp"
#include "graph/floyd.hpp"
#include "graph/mst.hpp"
#include "graph/topological_sort.hpp"
#include "graph/bipartite.hpp"
#include "graph/dinic.hpp"
#include "graph/mcmf.hpp"
#include "graph/tarjan.hpp"
#include "graph/two_sat.hpp"
#include "graph/bridge.hpp"
#include "graph/lca.hpp"
#include "graph/euler_path.hpp"
// ---- 数学 ----
#include "math/fastpow.hpp"
#include "math/gcd.hpp"
#include "math/prime.hpp"
#include "math/euler_phi.hpp"
#include "math/mobius.hpp"
#include "math/inverse.hpp"
#include "math/crt.hpp"
#include "math/linear_basis.hpp"
#include "math/gauss.hpp"
#include "math/matrix.hpp"
#include "math/combinatorics.hpp"
#include "math/modint.hpp"
#include "math/fft.hpp"
#include "math/ntt.hpp"
#include "math/bsgs.hpp"
#include "math/bigint.hpp"
// ---- 字符串 ----
#include "string/kmp.hpp"
#include "string/z_algorithm.hpp"
#include "string/manacher.hpp"
#include "string/trie.hpp"
#include "string/ac_automaton.hpp"
#include "string/suffix_array.hpp"
#include "string/suffix_automaton.hpp"
#include "string/rolling_hash.hpp"
// ---- 计算几何 ----
#include "geometry/point.hpp"
#include "geometry/line.hpp"
#include "geometry/polygon.hpp"
#include "geometry/convex_hull.hpp"
#include "geometry/rotating_calipers.hpp"
#include "geometry/circle.hpp"
// ---- 杂项 ----
#include "misc/fastio.hpp"
#include "misc/random.hpp"
#include "misc/timer.hpp"
#include "misc/debug.hpp"
#include "misc/bit.hpp"

using namespace algo;

int main() {
    // ===== 并查集 =====
    {
        DSU d(5);
        d.unite(1, 2);
        d.unite(2, 3);
        assert(d.same(1, 3));
        assert(!d.same(1, 4));
        assert(d.size(1) == 3);
    }

    // ===== 树状数组 =====
    {
        BIT<ll> b(5);
        b.add(1, 2);
        b.add(3, 5);
        assert(b.sum(3) == 7);
        assert(b.range_sum(2, 3) == 5);
    }

    // ===== 线段树（区间加 + 区间和） =====
    {
        SegTreeSum st(std::vector<ll>{1, 2, 3, 4});
        st.range_add(1, 3, 10);
        assert(st.query_sum(1, 4) == 40);
        assert(st.query_max(1, 4) == 13);
        assert(st.query_min(1, 4) == 4);
    }

    // ===== Dijkstra =====
    {
        std::vector<std::vector<Edge>> g(4);
        g[1].push_back({2, 1});
        g[2].push_back({3, 2});
        g[1].push_back({3, 5});
        std::vector<ll> dist = dijkstra(3, g, 1);
        assert(dist[1] == 0 && dist[2] == 1 && dist[3] == 3);
    }

    // ===== KMP =====
    {
        std::vector<int> p = kmp_match("ababab", "aba");
        assert(p.size() == 2 && p[0] == 0 && p[1] == 2);
    }

    // ===== 快速幂 / 快速乘 =====
    {
        assert(qpow(2, 10, 1000000007LL) == 1024);
        assert(qmul(123456789LL, 987654321LL, 1000000007LL) ==
               (123456789LL * 987654321LL) % 1000000007LL);
    }

    // ===== gcd / exgcd =====
    {
        ll x, y;
        assert(exgcd(6, 15, x, y) == 3);
        assert(x * 6 + y * 15 == 3);  // 特解验证 ax + by = gcd
        assert(gcd(12, 18) == 6);
        assert(lcm(4, 6) == 12);
    }

    // ===== LIS =====
    {
        assert(lis(std::vector<int>{1, 3, 2, 4}) == 3);
        assert(lis(std::vector<int>{1, 1, 1}) == 1);
        assert(lis(std::vector<int>{1, 1, 1}, false) == 3);
    }

    // ===== 离散化 =====
    {
        Discrete<int> d;
        std::vector<int> a{10, 20, 10, 30};
        d.build(a);
        assert(d.size() == 3);
        assert(d.rank_of(10) == 0 && d.rank_of(20) == 1 && d.rank_of(30) == 2);
    }

    // ===== FHQ Treap =====
    {
        FHQTreap t;
        t.insert(3);
        t.insert(1);
        t.insert(2);
        assert(t.kth(2) == 2);
        assert(t.rank(2) == 2);
        t.erase(2);
        assert(t.kth(2) == 3);
        assert(t.prev(3) == 1);
    }

    // ===== ModInt =====
    {
        ModInt<1000000007> a = 3, b = 4;
        assert(a + b == ModInt<1000000007>(7));
        assert(a * b == ModInt<1000000007>(12));
        assert(b / a == ModInt<1000000007>(333333337LL));  // 4 * inv(3)
        assert(-a == ModInt<1000000007>(1000000004LL));
    }

    // ===== NTT 卷积 =====
    {
        std::vector<ll> c = NTT<998244353, 3>::convolution({1, 2}, {3, 4});
        assert(c.size() == 3 && c[0] == 3 && c[1] == 10 && c[2] == 8);
    }

    // ===== 高精度 =====
    {
        BigInt a("123456789"), b("987654321");
        assert((a + b).to_string() == "1111111110");
        assert((BigInt(100) / BigInt(7)).to_string() == "14");
        assert((BigInt(7) * BigInt(8)).to_string() == "56");
        assert(BigInt("999") < BigInt("1000"));
    }

    // ===== 拓扑排序 =====
    {
        std::vector<std::vector<int>> g(4);
        g[1].push_back(2);
        g[1].push_back(3);
        g[3].push_back(2);
        std::vector<int> t = topo_sort(3, g);
        assert(t.size() == 3 && t[0] == 1 && t[2] == 2);
    }

    // ===== 线性基 =====
    {
        LinearBasis lb;
        lb.insert(5);
        lb.insert(3);
        assert(lb.max_xor() == 6);
        assert(lb.can(6));
    }

    // ===== 矩阵快速幂（斐波那契） =====
    {
        Matrix m(2, 1000000007LL);
        m.at(0, 0) = 1; m.at(0, 1) = 1;
        m.at(1, 0) = 1; m.at(1, 1) = 0;
        Matrix p = m.pow(10);
        // [[F(n+1), F(n)], [F(n), F(n-1)]]，F(10) = 55
        assert(p.at(0, 1) == 55 || p.at(1, 0) == 55);
    }

    // ===== 主席树（静态区间第 k 小） =====
    {
        PersistentSegTree pst;
        pst.build(std::vector<ll>{1, 5, 2, 6, 3, 7, 4});
        // [2, 5] = {5,2,6,3}，第 3 小 = 5
        assert(pst.kth(2, 5, 3) == 5);
    }

    // ===== 逆序对 =====
    {
        assert(inversion_count(std::vector<int>{3, 1, 2}) == 2);
    }

    // ===== 莫队：区间不同数个数 =====
    {
        std::vector<int> a{1, 2, 1, 3, 2};
        std::vector<MoQuery> qs{{0, 1, 0}, {0, 4, 1}, {2, 4, 2}};
        std::vector<int> cnt(4, 0);
        int cur = 0;
        auto add = [&](int idx) { if (cnt[a[idx]]++ == 0) ++cur; };
        auto del = [&](int idx) { if (--cnt[a[idx]] == 0) --cur; };
        auto get = [&]() { return (ll)cur; };
        std::vector<ll> ans = mo_solve((int)a.size(), qs, add, del, get);
        assert(ans[0] == 2 && ans[1] == 3 && ans[2] == 3);
    }

    // ===== 三分 =====
    {
        ll x = ternary_min_int(0, 100, [](ll v) { return (v - 7) * (v - 7); });
        assert(x == 7);
    }

    // ===== 单调栈 / 单调队列 =====
    {
        std::vector<int> ng = next_greater(std::vector<int>{2, 1, 3});
        assert(ng.size() == 3 && ng[0] == 2 && ng[1] == 2 && ng[2] == -1);
        std::vector<int> wm = sliding_window_min(std::vector<int>{1, 3, -1, 2}, 2);
        assert(wm.size() == 3 && wm[0] == 1 && wm[1] == -1 && wm[2] == -1);
    }

    // ===== 快速选择 / 归并排序 =====
    {
        std::vector<int> a{5, 2, 9, 1};
        assert(quick_select(a, 1) == 2);
        std::vector<int> b{3, 1, 2};
        merge_sort(b);
        assert(b[0] == 1 && b[2] == 3);
    }

    // ===== 字符串哈希 =====
    {
        StringHash h("abcabc");
        assert(h.get(0, 2) == h.get(3, 5));
    }

    // ===== Manacher =====
    {
        std::vector<int> d = manacher("abba");
        assert(*std::max_element(d.begin(), d.end()) - 1 == 4);
    }

    // ===== Dinic 最大流 =====
    {
        Dinic din(4);
        din.add_edge(1, 2, 3);
        din.add_edge(1, 3, 2);
        din.add_edge(2, 4, 2);
        din.add_edge(3, 4, 3);
        assert(din.maxflow(1, 4) == 4);
    }

    // ===== 匈牙利最大匹配 =====
    {
        std::vector<std::vector<int>> adj(4);
        adj[1] = {1, 2};
        adj[2] = {2};
        adj[3] = {3};
        assert(hungarian(3, 3, adj) == 3);
    }

    // ===== 2-SAT =====
    {
        TwoSAT ts(2);
        ts.add_clause(1, true, 2, true);   // x1 OR x2
        ts.add_clause(1, false, 2, false); // !x1 OR !x2
        assert(ts.solve());
    }

    // ===== 中国剩余定理 =====
    {
        assert(crt(std::vector<ll>{2, 3}, std::vector<ll>{3, 5}) == 8);
    }

    // ===== 素数 =====
    {
        std::vector<int> ps = euler_sieve(20);
        assert(ps.size() == 8 && ps[0] == 2 && ps[7] == 19);
        assert(is_prime(1000000007LL));
        assert(!is_prime(1000000009LL * 1000000009LL));
    }

    // ===== BSGS =====
    {
        assert(bsgs(2, 3, 5) == 3);  // 2^3 = 8 ≡ 3 (mod 5)
    }

    // ===== 位运算 =====
    {
        std::vector<int> ss = enumerate_subsets(0b101);
        assert(ss.size() == 4);
        assert(popcount(7) == 3);
        assert(msb_index(8) == 3);
        assert(next_combination(0b011) == 0b101);
    }

    // ===== 折半搜索 =====
    {
        std::vector<int> a{1, 2, 3};
        assert(closest_subset_sum(a, 4) == 4);  // 1 + 3
    }

    // ===== 欧拉函数 =====
    {
        assert(phi(10) == 4);
    }

    std::printf("[smoke] all tests passed\n");
    return 0;
}
