#pragma once
#include "data.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

namespace Dyer
{
    // ========== 写：追加一条 data 到 "用户名_data.txt" ==========
    // y m d high weight RHR Avg_HR SHR MAX_HR MINN_HR SpO2
    inline void append_data_to_file(const std::string& username, const data& d)
    {
        std::string filename = username + "_data.txt";
        std::ofstream out(filename, std::ios::app);
        if (!out.is_open())
        {
            std::cerr << "无法打开文件: " << filename << "\n";
            return;
        }
        out << d.show_y() << " " << d.show_m() << " " << d.show_d() << " "
            << d.show_high() << " " << d.show_weight() << " "
            << d.show_hr().RHR << " " << d.show_hr().Avg_HR << " "
            << d.show_hr().SHR << " " << d.show_hr().MAX_HR << " "
            << d.show_hr().MINN_HR << " " << d.show_hr().SpO2 << "\n";
    }

    // ========== 读：从 "用户名_data.txt" 读回全部数据 ==========
    inline std::vector<data> read_data_from_file(const std::string& username)
    {
        std::vector<data> result;
        std::string filename = username + "_data.txt";
        std::ifstream in(filename);
        if (!in.is_open()) { return result; }   // 文件不存在就返回空

        std::string y, m, d;
        double high, weight;
        int RHR, Avg_HR, SHR, MAX_HR, MINN_HR, SpO2;

        while (in >> y >> m >> d >> high >> weight
            >> RHR >> Avg_HR >> SHR >> MAX_HR >> MINN_HR >> SpO2)
        {
            data item(y, m, d);
            item.add_Body(high, weight);
            item.add_HR(RHR, Avg_HR, SHR, MAX_HR, MINN_HR, SpO2);
            result.push_back(std::move(item));
        }
        return result;
    }

    // ========== 写：追加用户名到 User_name.txt ==========
    inline void append_user(const std::string& name)//这个输出逻辑得改一下，要去判断原先文档里面是否存在这个name
    {
        std::ofstream out("User_name.txt", std::ios::app);
        out << name << "\n";
    }

    // ========== 写：全量覆盖用户数据文件（update / save 用）==========
    inline void rewrite_data_file(const std::string& username, const std::vector<data>& arr)
    {
        std::string filename = username + "_data.txt";
        std::ofstream out(filename);
        if (!out.is_open())
        {
            std::cerr << "无法写入文件: " << filename << "\n";
            return;
        }
        for (const auto& d : arr)
        {
            out << d.show_y() << " " << d.show_m() << " " << d.show_d() << " "
                << d.show_high() << " " << d.show_weight() << " "
                << d.show_hr().RHR << " " << d.show_hr().Avg_HR << " "
                << d.show_hr().SHR << " " << d.show_hr().MAX_HR << " "
                << d.show_hr().MINN_HR << " " << d.show_hr().SpO2 << "\n";
        }
    }
}