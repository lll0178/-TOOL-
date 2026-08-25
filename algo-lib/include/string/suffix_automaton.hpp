#pragma once
// ============================================================================
// algo/string/suffix_automaton.hpp —— 后缀自动机（SAM）
// ----------------------------------------------------------------------------
// 功能：
//   class SAM：
//   - SAM() / build(s)：初始化并逐字符建机（小写字母）。
//   - extend(c)：在线加入一个字符 c（c ∈ [0, 26)），标准实现（含 clone）。
//   - distinct_substrings()：本质不同子串个数 = Σ(len[v] - len[link[v]])。
//   - endpos_size()：返回每个状态的 endpos 集合大小。初始时每个新建主链
//     状态 cnt=1（clone 状态 cnt=0），按 len 计数排序得到拓扑序，从 len 大到
//     小把 cnt 累加到 link 上，即得到每个状态的 endpos 大小。
// 时间复杂度：build / extend 均摊 O(1)（总 O(n)）；distinct_substrings O(状态数)；
//   endpos_size 首次调用 O(状态数 + 最大 len)（计数排序），之后缓存。
// 空间复杂度：O(状态数 * 26)，状态数不超过 2n。
// 使用示例：
//   SAM sam; sam.build("ababa");
//   sam.distinct_substrings(); // 9（本质不同子串个数）
//   const vector<int>& sz = sam.endpos_size(); // 每个状态的 endpos 大小
// 注意事项：
//   - next 用 array<int,26>，仅支持小写字母；需要更大字符集可改为 map 或
//     更大数组（注意空间）。
//   - distinct_substrings 返回 long long，避免溢出。
//   - endpos_size() 返回的是“状态”维度的数组，下标为状态编号（0 为初始状态）。
// ============================================================================
#include <array>
#include <string>
#include <vector>
#include "algo/common.hpp"

namespace algo {

class SAM {
public:
    SAM() { init(); }

    // 逐字符构建：先重置，再依次 extend
    void build(const std::string& s) {
        init();
        for (char ch : s) {
            extend(ch - 'a');
        }
    }

    // 在线加入一个字符 c（0 <= c < 26），标准实现（含 clone）
    void extend(int c) {
        int cur = (int)st_.size();
        st_.push_back(Node{});
        st_[cur].len = st_[last_].len + 1;
        st_[cur].next.fill(-1);
        cnt_.push_back(1);  // 新建的主链状态，endpos 初始大小为 1
        endpos_computed_ = false;

        // 从 last 沿 link 向上，为没有 c 转移的状态建立到 cur 的转移
        int p = last_;
        while (p != -1 && st_[p].next[c] == -1) {
            st_[p].next[c] = cur;
            p = st_[p].link;
        }
        if (p == -1) {
            st_[cur].link = 0;  // 一路都没找到 c 转移，直接连到初始状态
        } else {
            int q = st_[p].next[c];
            if (st_[p].len + 1 == st_[q].len) {
                st_[cur].link = q;  // q 恰好能作为 cur 的 link
            } else {
                // 需要拆出 clone 状态
                int clone = (int)st_.size();
                st_.push_back(st_[q]);  // 复制 q 的转移
                st_[clone].len = st_[p].len + 1;
                st_[clone].link = st_[q].link;
                cnt_.push_back(0);  // clone 状态自身不是新主链节点，endpos 初始 0
                // 把原本指向 q 的 c 转移改指向 clone
                while (p != -1 && st_[p].next[c] == q) {
                    st_[p].next[c] = clone;
                    p = st_[p].link;
                }
                st_[q].link = st_[cur].link = clone;
            }
        }
        last_ = cur;
    }

    // 本质不同子串个数 = Σ(len[v] - len[link[v]])
    ll distinct_substrings() const {
        ll sum = 0;
        for (int v = 1; v < (int)st_.size(); v++) {
            sum += (ll)(st_[v].len - st_[st_[v].link].len);
        }
        return sum;
    }

    // 每个状态的 endpos 集合大小（首次调用计算并缓存）
    const std::vector<int>& endpos_size() const {
        if (!endpos_computed_) {
            compute_endpos();
        }
        return endpos_;
    }

private:
    // 单个状态：link 后缀链接、len 最长子串长度、next 转移表
    struct Node {
        int link;
        int len;
        std::array<int, 26> next;
    };

    std::vector<Node> st_;  // 状态池，st_[0] 为初始状态
    int last_;              // 当前整串对应的状态
    std::vector<int> cnt_;  // 各状态 endpos 初始计数（主链节点为 1，clone 为 0）
    mutable std::vector<int> endpos_;   // 缓存：每个状态的 endpos 大小
    mutable bool endpos_computed_;

    void init() {
        st_.clear();
        cnt_.clear();
        endpos_.clear();
        st_.push_back(Node{});
        st_[0].link = -1;  // 初始状态的 link 为 -1
        st_[0].len = 0;
        st_[0].next.fill(-1);
        cnt_.push_back(0);
        last_ = 0;
        endpos_computed_ = false;
    }

    // 按 len 计数排序得到拓扑序，从大到小累加 endpos 到 link
    void compute_endpos() const {
        int sz = (int)st_.size();
        // 计数排序：按 len 升序得到 order
        int maxlen = st_[last_].len;
        std::vector<int> bucket(maxlen + 1, 0);
        for (int v = 0; v < sz; v++) {
            bucket[st_[v].len]++;
        }
        for (int i = 1; i <= maxlen; i++) {
            bucket[i] += bucket[i - 1];
        }
        std::vector<int> order(sz);
        for (int v = sz - 1; v >= 0; v--) {
            order[--bucket[st_[v].len]] = v;
        }
        // 从 len 大到小：link 的 len 一定更小，因此这是合法拓扑序
        endpos_ = cnt_;
        for (int i = sz - 1; i >= 1; i--) {
            int v = order[i];
            if (st_[v].link != -1) {
                endpos_[st_[v].link] += endpos_[v];
            }
        }
        endpos_computed_ = true;
    }
};

}  // namespace algo
