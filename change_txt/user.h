#pragma once
//开一个txt专门记录用户名称，确保填写时不用每次新开map
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
namespace Dyer
{
	class Name
	{
	private:
		std::vector<std::string>User;
		size_t size = 0;
	public:
		~Name() = default;
		bool User_name(std::string name);
		size_t User_num();
		void printf_user();
		void add_user(std::string name);
		std::string show_name(size_t num) { return User[num - 1]; }
	};

	inline void read_user_name(Name& user)//读入name
	{
		std::string name, y, m, d;
		std::ifstream in("User_name.txt");
		while (in >> name)
		{
			user.add_user(name);
		}
		in.close();
	}
}