#include "write_data.h"
#include <string>

namespace Dyer
{
    std::unordered_map<std::string, std::vector<data>> Map;

    // ========== 用户入口 ==========
    void work1(std::string name)
    {
        if (user.User_name(name))
        {
            work(name);
        }
        else
        {
            user.add_user(name);
            work(name);
        }
    }

    // ========== 添加每日数据 ==========
    void work_add(std::string name, std::vector<data>& arr)
    {
        std::string y, m, d;
        std::cout << "请输入日期（y m d）: ";
        std::cin >> y >> m >> d;
        data x(y, m, d);
        std::cout << "请输入body数据（身高 体重，0表示沿用上次）:\n";
        double h, w;
        std::cin >> h >> w;

        bool has_prev = !arr.empty();
        if (h <= 0 && w <= 0)
        {
            if (has_prev) x.add_Body(arr.back());
            else x.add_Body(0, 0);
        }
        else if (h > 0 && w > 0)
        {
            x.add_Body(h, w);
        }
        else if (h > 0)
        {
            if (has_prev) x.add_Body(h, arr.back().show_weight());
            else x.add_Body(h, 0);
        }
        else
        {
            if (has_prev) x.add_Body(arr.back().show_high(), w);
            else x.add_Body(0, w);
        }
        std::cout << "请输入HR数据（RHR Avg_HR SHR MAX_HR MINN_HR SpO2）:\n";
        int RHR, Avg_HR, SHR, MAX_HR, MINN_HR, SpO2;
        std::cin >> RHR >> Avg_HR >> SHR >> MAX_HR >> MINN_HR >> SpO2;
        x.add_HR(RHR, Avg_HR, SHR, MAX_HR, MINN_HR, SpO2);
        arr.push_back(std::move(x));
        append_data_to_file(name, arr.back());
        std::cout << "已添加。\n";
    }

    // ========== 打印数据 ==========
    void work_printf(const std::string& name, std::vector<data>& arr)
    {
        if (arr.empty())
        {
            std::cout << "暂无数据。\n";
            return;
        }
        std::cout << "===== " << name << " 的全部记录 =====\n";
        for (auto& d : arr)
        {
            d.printf_all();
            std::cout << "------------------------\n";
        }
    }

    // ========== 按时间段查找 ==========
    void work_time(const std::string& name, std::vector<data>& arr)
    {
        if (arr.empty())
        {
            std::cout << "暂无数据。\n";
            return;
        }
        std::cout << "请输入起始日期（y m d）: ";
        std::string y1, m1, d1;
        std::cin >> y1 >> m1 >> d1;
        std::cout << "请输入结束日期（y m d）: ";
        std::string y2, m2, d2;
        std::cin >> y2 >> m2 >> d2;

        // 转成整数比较（避免字符串比较时 "7" > "10" 的问题）
        /*这个后续可以修改一下，现在这个会出现一些不太好的问题（直接转成字符串检测合法会正常一点，前面的读入也改一下）7.15*/
        int iy1 = std::stoi(y1), im1 = std::stoi(m1), id1 = std::stoi(d1);
        int iy2 = std::stoi(y2), im2 = std::stoi(m2), id2 = std::stoi(d2);
        long long start = iy1 * 10000LL + im1 * 100 + id1;
        long long end = iy2 * 10000LL + im2 * 100 + id2;

        std::cout << "===== " << name << " " << y1 << "/" << m1 << "/" << d1
            << " ~ " << y2 << "/" << m2 << "/" << d2 << " =====\n";
        bool found = false;
        for (auto& d : arr)
        {
            int iy = std::stoi(d.show_y());
            int im = std::stoi(d.show_m());
            int id = std::stoi(d.show_d());
            long long date = iy * 10000LL + im * 100 + id;
            if (date >= start && date <= end)
            {
                d.printf_all();
                std::cout << "------------------------\n";
                found = true;
            }
        }
        if (!found) std::cout << "该时间段内没有记录。\n";
    }

    // ========== 重写用户数据文件 ==========
    void work_update(const std::string& name, std::vector<data>& arr)
    {
        rewrite_data_file(name, arr);
        std::cout << "已更新 " << name << "_data.txt\n";
    }

    // ========== 保存：写 Map + 写文件 ==========
    void save(const std::string& name, std::vector<data>& x)
    {
        Map[name] = x;                    // 更新内存缓存
        rewrite_data_file(name, x);       // 全量写入文件
    }

    // ========== 用户操作菜单 ==========
    void work(std::string name)
    {
        std::cout << "请选择操作：add(添加数据)，printf(打印数据)，"
            "update(更新文件)，time(按时间查找)，save(保存)，end(结束)\n";

        // 懒加载：如果 Map 里已有就用，否则从文件读
        std::vector<data> arr;
        if (Map.find(name) != Map.end())
        {
            arr = Map[name];
            std::cout << "（已从缓存加载 " << arr.size() << " 条记录）\n";
        }
        else
        {
            arr = read_data_from_file(name);
            if (!arr.empty())
            {
                Map[name] = arr;
                std::cout << "（已从文件加载 " << arr.size() << " 条记录）\n";
            }
        }

        std::string s;
        while (1)
        {
            std::cin >> s;
            if (s == "add") { work_add(name, arr); }
            else if (s == "printf") { work_printf(name, arr); }
            else if (s == "update") { work_update(name, arr); }
            else if (s == "time") { work_time(name, arr); }
            else if (s == "save") { save(name, arr); }
            else if (s == "end") { save(name, arr); break; }
            else { std::cout << "无效输入，请重试。\n"; }
        }
    }
}