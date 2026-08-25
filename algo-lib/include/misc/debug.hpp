#pragma once
// ============================================================================
// misc/debug.hpp —— 调试输出
// ----------------------------------------------------------------------------
// 功能：宏 ALGO_DBG(...) 在定义 ALGO_DEBUG 时向 stderr 打印"变量名 = 值"，
//       支持多参数（逗号分隔）；未定义 ALGO_DEBUG 时展开为空、零开销。
//       对 vector / map / set / pair / string 提供专门的重载打印。
// 时间复杂度：与打印内容规模成正比。
// 空间复杂度：O(1)（递归调用栈深度 = 参数个数）。
// 使用示例：
//   #define ALGO_DEBUG              // 开启调试输出（通常放编译命令或文件顶部）
//   #include "misc/debug.hpp"
//   int a = 3;
//   std::vector<int> v = {1, 2, 3};
//   ALGO_DBG(a, v);                 // stderr: [debug.cpp:7] a = 3, v = [1, 2, 3]
// 注意事项：
//   - 使用方式：编译时加 -DALGO_DEBUG，或在 include 本头文件之前 #define ALGO_DEBUG。
//   - 未定义 ALGO_DEBUG 时，ALGO_DBG(...) 展开为 ((void)0)，不引用、不求值参数，
//     保证零运行时开销；此时若某变量仅出现在 ALGO_DBG 中，编译器仍可能提示
//     "未使用变量"，这是调试宏的常见行为，可用 (void)变量 消除。
//   - ALGO_DBG 至少需要 1 个参数。
//   - 泛型类型需支持 operator<< 才能打印；容器元素同样需支持 operator<<。
// ============================================================================
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace algo {

// ============ 值打印（不带名字，供嵌套容器递归使用） ============

// 泛型：默认通过 std::ostream 输出，要求 T 支持 operator<<
template <typename T>
void dbg_print_value(const T& v) {
    std::cerr << v;
}

// std::string：加双引号
inline void dbg_print_value(const std::string& v) {
    std::cerr << "\"" << v << "\"";
}

// C 风格字符串（字符串字面量等）
inline void dbg_print_value(const char* v) {
    std::cerr << "\"" << (v == nullptr ? "(null)" : v) << "\"";
}

// std::pair
template <typename A, typename B>
void dbg_print_value(const std::pair<A, B>& v) {
    std::cerr << "(";
    dbg_print_value(v.first);
    std::cerr << ", ";
    dbg_print_value(v.second);
    std::cerr << ")";
}

// std::vector
template <typename T>
void dbg_print_value(const std::vector<T>& v) {
    std::cerr << "[";
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i != 0) std::cerr << ", ";
        dbg_print_value(v[i]);
    }
    std::cerr << "]";
}

// std::set
template <typename T>
void dbg_print_value(const std::set<T>& v) {
    std::cerr << "{";
    bool first = true;
    for (typename std::set<T>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (!first) std::cerr << ", ";
        first = false;
        dbg_print_value(*it);
    }
    std::cerr << "}";
}

// std::map
template <typename K, typename V>
void dbg_print_value(const std::map<K, V>& v) {
    std::cerr << "{";
    bool first = true;
    for (typename std::map<K, V>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (!first) std::cerr << ", ";
        first = false;
        dbg_print_value(it->first);
        std::cerr << ": ";
        dbg_print_value(it->second);
    }
    std::cerr << "}";
}

// ============ 名字 + 值打印 ============

// 打印"名字 = 值"；具体值的格式由 dbg_print_value 的重载决定。
template <typename T>
void dbg_print_one(const char* name, const T& v) {
    std::cerr << name << " = ";
    dbg_print_value(v);
}

namespace detail {

// 从逗号分隔的名字串中解析出下一个名字，并把指针推进到下一个名字开头
inline std::string next_name(const char*& p) {
    while (*p == ' ') ++p;                // 跳过前导空格
    const char* start = p;
    while (*p != '\0' && *p != ',') ++p;  // 读到逗号或串尾
    std::string s(start, p);
    if (*p == ',') ++p;                   // 跳过逗号
    return s;
}

// 递归终止：没有剩余参数
inline void dbg_print_all(const char* /*names*/, int /*idx*/) {}

// 递归：解析一个名字 + 打印一个值，再处理剩余参数；idx 用于控制逗号分隔
template <typename T, typename... Rest>
void dbg_print_all(const char* names, int idx, const T& v, Rest... rest) {
    std::string name = next_name(names);
    if (idx > 0) std::cerr << ", ";
    dbg_print_one(name.c_str(), v);
    dbg_print_all(names, idx + 1, rest...);
}

// 入口：打印 文件:行号，再打印所有"名字 = 值"
template <typename... Args>
void dbg(const char* file, int line, const char* names, Args... args) {
    std::cerr << "[" << file << ":" << line << "] ";
    dbg_print_all(names, 0, args...);
    std::cerr << std::endl;
}

}  // namespace detail
}  // namespace algo

// ---- 调试宏 ----
#ifdef ALGO_DEBUG
// 开启调试：#__VA_ARGS__ 把参数列表字符串化为 "a, b" 供名字解析
#define ALGO_DBG(...) ::algo::detail::dbg(__FILE__, __LINE__, #__VA_ARGS__, __VA_ARGS__)
#else
// 关闭调试：展开为空（不引用、不求值参数，零开销）
#define ALGO_DBG(...) ((void)0)
#endif
