// ============================================================================
// examples/demo.cpp —— 综合使用示例
// ----------------------------------------------------------------------------
// 演示如何 include 模板库并调用几个常用板子。
// 构建：
//   g++ -std=c++17 -O2 -I include examples/demo.cpp -o build/demo && ./build/demo
//   cl /std:c++17 /EHsc /I include examples\demo.cpp /Fe:build\demo.exe
// ============================================================================
#include <cstdio>
#include <string>
#include <vector>

#include "algo/common.hpp"
#include "ds/union_find.hpp"
#include "ds/fenwick.hpp"
#include "graph/dijkstra.hpp"
#include "math/fastpow.hpp"
#include "math/bigint.hpp"
#include "string/kmp.hpp"

using namespace algo;

int main() {
    // 1. 并查集
    DSU dsu(5);
    dsu.unite(1, 2);
    dsu.unite(2, 3);
    std::printf("same(1,3) = %d\n", dsu.same(1, 3) ? 1 : 0);

    // 2. 树状数组：单点加 + 前缀和
    BIT<ll> bit(5);
    bit.add(1, 3);
    bit.add(3, 4);
    std::printf("bit.sum(3) = %lld\n", (long long)bit.sum(3));

    // 3. Dijkstra 最短路
    std::vector<std::vector<Edge>> g(4);
    g[1].push_back({2, 1});
    g[2].push_back({3, 2});
    g[1].push_back({3, 5});
    std::vector<ll> dist = dijkstra(3, g, 1);
    std::printf("dist[1->3] = %lld\n", (long long)dist[3]);

    // 4. 快速幂
    std::printf("qpow(2, 10, 1e9+7) = %lld\n", (long long)qpow(2, 10, 1000000007LL));

    // 5. KMP 多模式匹配
    std::vector<int> pos = kmp_match("ababab", "aba");
    std::printf("kmp matches at: ");
    for (int p : pos) std::printf("%d ", p);
    std::printf("\n");

    // 6. 高精度整数
    BigInt x("123456789123456789"), y("987654321987654321");
    std::printf("bigint sum = %s\n", (x + y).to_string().c_str());

    return 0;
}
