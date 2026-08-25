#pragma once
// ============================================================================
// algo/string/ac_automaton.hpp —— AC 自动机（多模式匹配）
// ----------------------------------------------------------------------------
// 功能：
//   - insert(s)：插入一个模式串，模式串按插入顺序编号 0..m-1。
//   - build()：BFS 建 fail 指针，并把缺失转移补全成完整转移表（查询 O(1)）。
//   - query(text)：返回长度 m 的 vector<int>，第 i 项为模式串 i 在 text 中
//     出现的次数。支持“模式串互为前缀”的计数：一个位置匹配到节点 v，等价于
//     fail 链上的所有祖先节点也各出现一次，故按 BFS 逆序（拓扑序）把每个
//     节点的计数一次性累加到 fail 上。
// 时间复杂度：
//   - insert：O(|s|)。
//   - build：O(节点数 * 26)。
//   - query：O(|text| + 节点数)（转移已补全 + 拓扑累加）。
// 空间复杂度：O(节点数 * 26)。
// 使用示例：
//   ACAutomaton ac;
//   ac.insert("he"); ac.insert("she"); ac.insert("her");
//   ac.build();
//   vector<int> ans = ac.query("ushers"); // {"he":1, "she":1, "her":1}
// 注意事项：
//   - 仅支持小写字母 'a'..'z'；query 中遇到的非小写字符会被忽略。
//   - 必须先调用 build() 再调用 query()，否则转移表未补全。
//   - 模式串可为空串（end 挂在根节点上）；若出现次数极大，int 可能溢出，
//     可将返回类型改为 long long。
// ============================================================================
#include <array>
#include <queue>
#include <string>
#include <vector>
#include "algo/common.hpp"

namespace algo {

class ACAutomaton {
public:
    ACAutomaton() { new_node(); }  // 根节点编号为 0

    // 插入一个模式串，编号按插入顺序 0,1,2,...
    void insert(const std::string& s) {
        int u = 0;
        for (char ch : s) {
            int c = ch - 'a';
            if (child_[u][c] == -1) child_[u][c] = new_node();
            u = child_[u][c];
        }
        out_[u].push_back(pat_cnt_++);  // 记录以该节点结尾的模式串编号
    }

    // BFS 构建 fail 指针，并补全缺失转移
    void build() {
        std::queue<int> q;
        // 初始化：根节点的子节点 fail = 根，缺失转移指向根
        for (int c = 0; c < 26; c++) {
            if (child_[0][c] == -1) {
                child_[0][c] = 0;  // 补全转移
            } else {
                fail_[child_[0][c]] = 0;
                q.push(child_[0][c]);
            }
        }
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order_.push_back(u);  // 记录 BFS 顺序（fail 深度递增）
            for (int c = 0; c < 26; c++) {
                int v = child_[u][c];
                if (v == -1) {
                    // 缺失转移补全：指向 fail 的对应转移，使查询无需回退
                    child_[u][c] = child_[fail_[u]][c];
                } else {
                    fail_[v] = child_[fail_[u]][c];  // fail[v] = fail(u)+c
                    q.push(v);
                }
            }
        }
    }

    // 多模式匹配：返回每个模式串的出现次数（含互为前缀的情况）
    std::vector<int> query(const std::string& text) const {
        int sz = (int)child_.size();
        std::vector<ll> freq(sz, 0);  // 每个节点在文本扫描中被访问的次数
        int u = 0;
        for (char ch : text) {
            if (ch < 'a' || ch > 'z') continue;  // 忽略非小写字母
            int c = ch - 'a';
            u = child_[u][c];  // 转移已补全，直接跳转
            freq[u]++;
        }
        // 按 BFS 逆序（即 fail 深度从大到小）累加：
        // 走到 v 一次，等价于 fail[v] 对应的模式也出现一次
        for (int i = (int)order_.size() - 1; i >= 0; i--) {
            int v = order_[i];
            if (v != 0) {
                freq[fail_[v]] += freq[v];
            }
        }
        // 汇总到各模式串
        std::vector<int> ans(pat_cnt_, 0);
        for (int v = 0; v < sz; v++) {
            for (int pid : out_[v]) {
                ans[pid] = (int)freq[v];
            }
        }
        return ans;
    }

private:
    std::vector<std::array<int, 26>> child_;  // 转移表（build 后补全）
    std::vector<int> fail_;                    // fail 指针
    std::vector<std::vector<int>> out_;        // 以该节点结尾的模式串编号列表
    std::vector<int> order_;                   // BFS 顺序（用于拓扑累加）
    int pat_cnt_ = 0;                          // 已插入模式串数量

    int new_node() {
        child_.push_back(std::array<int, 26>{});
        child_.back().fill(-1);
        fail_.push_back(0);
        out_.push_back(std::vector<int>());
        return (int)child_.size() - 1;
    }
};

}  // namespace algo
