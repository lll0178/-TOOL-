#pragma once
// ============================================================================
// misc/fastio.hpp —— 快读快写
// ----------------------------------------------------------------------------
// 功能：基于 fread / fwrite 缓冲的整数快读快写，比 scanf/printf 更快。
//       提供 read（整数）、write（整数）、write_char、write_str、println、flush。
// 时间复杂度：每字节 O(1)（缓冲读入/写出）。
// 空间复杂度：输入/输出缓冲各 1 << 20 字节。
// 使用示例：
//   #include "misc/fastio.hpp"
//   long long x;
//   while (algo::read(x)) {          // 读到 EOF 返回 false
//       algo::println(x * 2);        // 输出后换行
//   }
//   algo::flush();                   // 程序结束前刷新输出缓冲（也可省略，析构自动刷新）
// 注意事项：
//   - read 支持 int / long long / unsigned long long，可读负数（对无符号类型
//     按取模语义处理）；输入须为合法整数。
//   - 无符号类型读取接近 2^64-1 的极大值时，累加过程可能溢出，建议只读
//     < 2^63 的值。
//   - 不要与 cin/cout/scanf/printf 混用：本模块直接操作 stdin/stdout 缓冲，
//     混用会导致顺序错乱；若必须混用，请先调用 flush()，并注意 cin 需
//     先解除与 stdio 的同步。
// ============================================================================
#include <cstddef>
#include <cstdio>
#include <type_traits>

namespace algo {
namespace detail {

constexpr int FASTIO_BUF = 1 << 20;   // 输入/输出缓冲区大小
constexpr char EOF_CHAR = '\0';       // 内部 EOF 标记（fread 读到 0 字节时返回）

// 快读快写的全部状态集中于此，通过 Meyers 单例跨翻译单元共享，
// 且在析构时自动 flush 输出缓冲（避免程序结束时丢数据）。
struct FastIOState {
    char in_buf[FASTIO_BUF];
    int in_pos = 0;
    int in_len = 0;
    char pushed = EOF_CHAR;  // 回退的 1 个字符
    bool has_pushed = false;

    char out_buf[FASTIO_BUF];
    int out_pos = 0;

    // 析构时把剩余输出写入 stdout
    ~FastIOState() { flush_out(); }

    void flush_out() {
        if (out_pos > 0) {
            std::fwrite(out_buf, 1, static_cast<std::size_t>(out_pos), stdout);
            out_pos = 0;
        }
    }
};

inline FastIOState& io_state() {
    static FastIOState s;
    return s;
}

// 读取一个字符（带 1 字符回退），EOF 返回 EOF_CHAR
inline char get_char() {
    FastIOState& s = io_state();
    if (s.has_pushed) {
        s.has_pushed = false;
        return s.pushed;
    }
    if (s.in_pos >= s.in_len) {
        s.in_len = static_cast<int>(std::fread(s.in_buf, 1, FASTIO_BUF, stdin));
        s.in_pos = 0;
        if (s.in_len == 0) return EOF_CHAR;
    }
    return s.in_buf[s.in_pos++];
}

// 回退一个字符（供 read 读到数字后的分隔符时使用）
inline void unget_char(char c) {
    FastIOState& s = io_state();
    s.pushed = c;
    s.has_pushed = true;
}

// 输出一个字符到缓冲
inline void put_char(char c) {
    FastIOState& s = io_state();
    if (s.out_pos >= FASTIO_BUF) s.flush_out();
    s.out_buf[s.out_pos++] = c;
}

// 输出一个非负无符号整数（逆序生成数字再正序输出）
template <typename U>
inline void write_unsigned(U u) {
    char buf[24];  // 足够容纳 64 位无符号整数
    int len = 0;
    do {
        buf[len++] = static_cast<char>('0' + static_cast<int>(u % 10));
        u /= 10;
    } while (u != 0);
    while (len > 0) put_char(buf[--len]);
}

// 有符号整数：先处理负号，再转无符号输出（避免 LLONG_MIN 取负溢出）
template <typename T>
inline void write_impl(T x, std::true_type /* signed */) {
    using U = typename std::make_unsigned<T>::type;
    if (x < 0) {
        put_char('-');
        U u = U(0) - U(x);  // 无符号取负：得到 |x|，不溢出
        write_unsigned(u);
    } else {
        write_unsigned(static_cast<U>(x));
    }
}

// 无符号整数：直接输出
template <typename T>
inline void write_impl(T x, std::false_type /* unsigned */) {
    write_unsigned(x);
}

}  // namespace detail

// 快读整数（int / long long / unsigned long long，支持负数）。
// 读到 EOF 返回 false，否则返回 true。
template <typename T>
bool read(T& x) {
    char c = detail::get_char();
    if (c == detail::EOF_CHAR) return false;

    // 跳过空白
    while (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f') {
        c = detail::get_char();
        if (c == detail::EOF_CHAR) return false;
    }

    // 符号
    bool neg = false;
    if (c == '-') {
        neg = true;
        c = detail::get_char();
    } else if (c == '+') {
        c = detail::get_char();
    }

    // 数字累加（用无符号 long long 累加，兼容各整数类型）
    unsigned long long v = 0;
    while (c >= '0' && c <= '9') {
        v = v * 10ULL + static_cast<unsigned long long>(c - '0');
        c = detail::get_char();
    }

    // 数字后的分隔符已被读走，回退以便下次读取
    detail::unget_char(c);

    if (neg) {
        // 无符号取负得到模意义下的相反数；对有符号类型在二补码机器上
        // 即正确的负值（MSVC/GCC/Clang 均如此）
        x = static_cast<T>(0ULL - v);
    } else {
        x = static_cast<T>(v);
    }
    return true;
}

// 快写整数（含负数）
template <typename T>
void write(T x) {
    detail::write_impl(x, std::integral_constant<bool, std::is_signed<T>::value>());
}

// 输出单个字符
inline void write_char(char c) {
    detail::put_char(c);
}

// 输出 C 风格字符串（以 '\0' 结尾）
inline void write_str(const char* s) {
    while (*s != '\0') detail::put_char(*s++);
}

// 输出整数并换行
template <typename T>
void println(T x) {
    write(x);
    detail::put_char('\n');
}

// 手动刷新输出缓冲（程序结束前调用；也可依赖全局对象析构自动刷新）
inline void flush() {
    detail::io_state().flush_out();
}

}  // namespace algo
