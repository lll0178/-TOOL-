#pragma once
#include "data.h"
#include "user.h"
#include "printf_txt.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace Dyer
{
    extern std::unordered_map<std::string, std::vector<data>> Map;
    inline Name user;

    inline void User_download()
    {
        read_user_name(user);
    }

    inline void download_user_name(const std::string& name, std::vector<data>& x)
    {
        Map[name] = x;
    }

    void save(const std::string& name, std::vector<data>& x);

    void work1(std::string s);
    void work_add(std::string name, std::vector<data>& arr);
    void work_printf(const std::string& name, std::vector<data>& arr);
    void work_update(const std::string& name, std::vector<data>& arr);
    void work_time(const std::string& name, std::vector<data>& arr);
    void work(std::string name);
}