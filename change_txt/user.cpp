#include "user.h"
#include "printf_txt.h"
namespace Dyer
{
	bool Name::User_name(std::string name)
	{
		for (auto& it : User)
		{
			if (name == it) { return true; }
		}
		return false;
	}
	size_t Name::User_num()
	{
		return size;
	}
	void Name::printf_user()
	{
		for (auto& it : User)
		{
			std::cout << it << "\n";
		}
	}
	void Name::add_user(std::string name)
	{
		if (User_name(name))
		{
			std::cerr << "name_exist\n";
			return;
		}
		size++;
		User.push_back(name);
		append_user(name);
	}
}