# algo-lib

**C++ 算法与数据结构模板库（ACM / OI 板子库）**

一个 **header-only** 的 C++17 模板库，收录竞赛与工程中常用的算法与数据结构"板子"，
每个板子独立成一个头文件，**即拷即用、即插即用**。所有代码带中文注释，
标注时间/空间复杂度与注意事项。

## 特性

- 📦 **Header-only**：全部实现位于头文件中，无需编译链接，`#include` 即可使用
- 🧩 **独立自包含**：每个板子一个头文件，可单独拷贝进你的项目/代码模板
- ✅ **跨平台**：兼容 MSVC 2017+ / GCC 7+ / Clang 6+，仅依赖 C++ 标准库
- 🗒️ **中文注释**：每个文件顶部有功能说明、复杂度、使用示例、注意事项
- 🧪 **可验证**：自带冒烟测试（`tests/smoke.cpp`），一键全库编译 + 正确性断言
- 📚 **文档齐全**：按分类提供使用文档与编码约定

## 目录结构

```
algo-lib/
├── CMakeLists.txt          # CMake 构建（接口库 + 示例 + 冒烟测试）
├── README.md               # 本文档
├── docs/                   # 分类使用文档
│   ├── 约定.md             # 编码约定（新增板子必读）
│   ├── 基础算法.md
│   ├── 数据结构.md
│   ├── 图论.md
│   ├── 数学.md
│   ├── 字符串.md
│   ├── 计算几何.md
│   └── 杂项.md
├── include/
│   ├── algo_lib.h          # 总入口（一键包含全部板子）
│   ├── algo/               # 基础算法（含公共定义 common.hpp）
│   ├── ds/                 # 数据结构
│   ├── graph/              # 图论
│   ├── math/               # 数学
│   ├── string/             # 字符串
│   ├── geometry/           # 计算几何
│   └── misc/               # 杂项工具
├── examples/
│   └── demo.cpp            # 综合使用示例
└── tests/
    └── smoke.cpp           # 冒烟测试（全库编译 + 断言）
```

## 快速开始

### 方式一：直接拷贝头文件

把 `include/` 下的头文件（或单个板子）拷进你的工程，然后：

```cpp
#include "ds/union_find.hpp"     // 并查集
#include "graph/dijkstra.hpp"    // 最短路
#include "math/fastpow.hpp"      // 快速幂

using namespace algo;

int main() {
    DSU dsu(10);
    dsu.unite(1, 2);
    // ...
    long long x = qpow(2, 10, 1000000007LL);
}
```

> 库内头文件通过 `#include "algo/xxx.hpp"` 互相引用，
> 单独拷贝时请保持目录层级（`include/` 作为 include 根目录）。

### 方式二：CMake 集成

```cmake
add_subdirectory(algo-lib)
target_link_libraries(your_app PRIVATE algo_lib)
```

### 方式三：命令行编译示例

```bash
# GCC / Clang
g++ -std=c++17 -O2 -I include examples/demo.cpp -o build/demo && ./build/demo

# MSVC（VS 开发者命令行）
cl /std:c++17 /EHsc /I include examples\demo.cpp /Fe:build\demo.exe
```

## 板子清单（68 个）

| 分类 | 文档 | 板子 |
| ---- | ---- | ---- |
| 基础算法 | [docs/基础算法.md](docs/基础算法.md) | `binary_search` 函数式二分 · `three_point` 三分 · `sort` 归并/快排/第 k 小/逆序对 · `discretize` 离散化 · `mo_algorithm` 莫队 · `meet_in_middle` 折半搜索 · `lis_lcs` |
| 数据结构 | [docs/数据结构.md](docs/数据结构.md) | `union_find` 并查集三件套 · `fenwick` 树状数组四件套 · `segment_tree` 线段树/最大子段和/扫描线 · `persistent_segment_tree` 主席树 · `sparse_table` ST 表 · `treap` FHQ Treap · `splay` 文艺平衡树 · `hld` 树链剖分 · `cartesian_tree` 笛卡尔树 · `kdtree` KD-Tree · `lct` Link-Cut Tree · `block` 分块 · `monotonic` 单调栈/队列 |
| 图论 | [docs/图论.md](docs/图论.md) | `dijkstra` · `spfa` + Bellman-Ford · `floyd` + 传递闭包 · `mst` Kruskal/Prim · `topological_sort` · `bipartite` 染色/匈牙利 · `dinic` 最大流/最小割 · `mcmf` 费用流 · `tarjan` SCC 缩点 · `two_sat` · `bridge` 割点/桥/边双 · `lca` 倍增 + 树上差分 · `euler_path` 欧拉回路 |
| 数学 | [docs/数学.md](docs/数学.md) | `fastpow` 快速幂/乘 · `gcd`/exgcd · `prime` 筛/Miller-Rabin/Pollard-Rho · `euler_phi` · `mobius` · `inverse` 逆元 · `crt`/exCRT · `linear_basis` 线性基 · `gauss` 高斯消元 · `matrix` 矩阵快速幂 · `combinatorics` 组合数学 · `modint` 模数类 · `fft` · `ntt` + 多项式求逆 · `bsgs` · `bigint` 高精度 |
| 字符串 | [docs/字符串.md](docs/字符串.md) | `kmp` · `z_algorithm` · `manacher` · `trie`/01 字典树 · `ac_automaton` · `suffix_array` · `suffix_automaton` · `rolling_hash` 单/双哈希 |
| 计算几何 | [docs/计算几何.md](docs/计算几何.md) | `point` 点/向量 · `line` 直线/线段 · `polygon` 多边形 · `convex_hull` 凸包 · `rotating_calipers` 旋转卡壳 · `circle` 圆/最小圆覆盖 |
| 杂项 | [docs/杂项.md](docs/杂项.md) | `fastio` 快读快写 · `random` 随机数 · `timer` 计时/内存 · `debug` 调试输出 · `bit` 位运算 |

> 另有 `include/algo_lib.h` 总入口：`#include "algo_lib.h"` 一键包含全部板子。

## 运行冒烟测试

```bash
# 方式一：直接编译（GCC/Clang）
g++ -std=c++17 -O2 -Wall -Wextra -I include tests/smoke.cpp -o build/smoke && ./build/smoke

# 方式二：CMake + CTest
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build
```

全部通过时输出 `[smoke] all tests passed`。

## 编码约定

新增板子前请阅读 [docs/约定.md](docs/约定.md)，内容包括：

- C++17、仅标准库、MSVC/GCC/Clang 三端兼容
- 头文件规范：`#pragma once`、`namespace algo`、自包含、中文注释块
- 命名规范与公共依赖（`algo/common.hpp` 中的 `ll` / `INF` / `EPS` / `Edge`）

## 许可证

[MIT](LICENSE)
