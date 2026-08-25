#pragma once
// ============================================================================
// algo/string/trie.hpp —— 字典树（Trie）与 01 字典树（Binary Trie）
// ----------------------------------------------------------------------------
// 功能：
//   1. class Trie：26 个小写字母的字典树。用 vector<array<int,26>> 节点池
//      动态开点，根节点编号为 0。每个节点记录 cnt（经过该节点的字符串数，
//      即前缀计数）与 end_cnt（以该节点结尾的字符串数）。
//      - insert(s)：插入一个字符串。
//      - find(s)：判断 s 是否被完整插入过（end_cnt > 0）。
//      - count_prefix(s)：统计以 s 为前缀的已插入字符串数量。
//      - count_exact(s)：统计与 s 完全相等的已插入字符串数量。
//   2. class BinaryTrie：01 字典树，支持插入、删除、查询与给定 x 异或的
//      最大值。固定 31 位（从高位到低位），适用于 0 <= x < 2^31；若需更大
//      范围，可把 BITS 改为 63（适用于 0 <= x < 2^63，此时循环变量 b 最大
//      为 62，1LL << 62 仍安全）。
// 时间复杂度：
//   - Trie 各操作：O(|s|)。
//   - BinaryTrie 各操作：O(BITS)。
// 空间复杂度：O(总字符数 * 26)（Trie）；O(操作数 * BITS)（BinaryTrie）。
// 使用示例：
//   Trie t; t.insert("abc"); t.insert("abd");
//   t.find("abc");        // true
//   t.count_prefix("ab"); // 2
//   t.count_exact("abc"); // 1
//   BinaryTrie bt; bt.insert(3); bt.insert(5);
//   bt.max_xor(6);        // 与 3 异或 = 5，与 5 异或 = 3，取最大 5
// 注意事项：
//   - Trie 仅支持小写字母 'a'..'z'，其它字符会越界（数组下标为 ch - 'a'）。
//   - BinaryTrie 假定 x 为非负整数；erase 前须保证 x 已被插入，否则计数变负。
//   - cnt / end_cnt 为 int，若插入次数极大（>2^31）可能溢出，可改为 ll。
// ============================================================================
#include <array>
#include <string>
#include <vector>
#include "algo/common.hpp"

namespace algo {

// 26 小写字母字典树
class Trie {
public:
    Trie() { new_node(); }  // 根节点编号固定为 0

    void insert(const std::string& s) {
        int u = 0;
        cnt_[u]++;  // 空串也计入前缀计数
        for (char ch : s) {
            int c = ch - 'a';
            if (child_[u][c] == -1) child_[u][c] = new_node();
            u = child_[u][c];
            cnt_[u]++;  // 沿路经过次数 +1（前缀计数）
        }
        end_[u]++;  // 结尾次数 +1
    }

    bool find(const std::string& s) const {
        int u = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (child_[u][c] == -1) return false;
            u = child_[u][c];
        }
        return end_[u] > 0;  // 走到末尾且结尾次数 > 0 才算存在
    }

    int count_prefix(const std::string& s) const {
        int u = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (child_[u][c] == -1) return 0;  // 前缀都不存在
            u = child_[u][c];
        }
        return cnt_[u];  // 经过该节点的字符串数 = 以 s 为前缀的数量
    }

    int count_exact(const std::string& s) const {
        int u = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (child_[u][c] == -1) return 0;
            u = child_[u][c];
        }
        return end_[u];  // 以该节点结尾的字符串数
    }

private:
    std::vector<std::array<int, 26>> child_;  // 子节点，-1 表示不存在
    std::vector<int> cnt_;                     // 经过该节点的字符串数（前缀计数）
    std::vector<int> end_;                     // 以该节点结尾的字符串数

    int new_node() {
        child_.push_back(std::array<int, 26>{});
        child_.back().fill(-1);
        cnt_.push_back(0);
        end_.push_back(0);
        return (int)child_.size() - 1;
    }
};

// 01 字典树（固定位数，支持插入 / 删除 / 查询最大异或）
class BinaryTrie {
public:
    // 固定位数：默认 31 位（0 <= x < 2^31）；更大范围可改为 63
    static const int BITS = 31;

    BinaryTrie() { new_node(); }  // 根节点编号为 0

    void insert(ll x) {
        int u = 0;
        cnt_[u]++;
        for (int b = BITS - 1; b >= 0; b--) {
            int bit = (int)((x >> b) & 1LL);
            if (child_[u][bit] == -1) child_[u][bit] = new_node();
            u = child_[u][bit];
            cnt_[u]++;  // 经过计数 +1
        }
    }

    void erase(ll x) {
        // 注意：调用前须保证 x 已插入，否则计数会变负
        int u = 0;
        cnt_[u]--;
        for (int b = BITS - 1; b >= 0; b--) {
            int bit = (int)((x >> b) & 1LL);
            u = child_[u][bit];
            cnt_[u]--;  // 沿路经过计数 -1（懒删除，不真正释放节点）
        }
    }

    ll max_xor(ll x) const {
        int u = 0;
        ll res = 0;
        for (int b = BITS - 1; b >= 0; b--) {
            int bit = (int)((x >> b) & 1LL);
            int want = bit ^ 1;  // 贪心：优先走与当前位相反的子树
            if (child_[u][want] != -1 && cnt_[child_[u][want]] > 0) {
                res |= (1LL << b);  // 该位异或结果为 1
                u = child_[u][want];
            } else {
                u = child_[u][bit];  // 只能走相同位
            }
        }
        return res;  // 返回与 x 异或的最大值（不含具体配对元素）
    }

private:
    std::vector<std::array<int, 2>> child_;  // 子节点 {0,1}，-1 表示不存在
    std::vector<int> cnt_;                    // 经过该节点的数值个数

    int new_node() {
        child_.push_back(std::array<int, 2>{});
        child_.back().fill(-1);
        cnt_.push_back(0);
        return (int)child_.size() - 1;
    }
};

}  // namespace algo
