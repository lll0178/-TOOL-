#pragma once
#include<string>
#include<iostream>
namespace Dyer
{
	class data
	{
	private:
		std::string y, m, d;
		struct Body
		{
			double high = 0, weight = 0;
		}body;
		struct HR
		{
			int RHR = 0, Avg_HR = 0, SHR = 0, MAX_HR = 0, MINN_HR = 0, SpO2 = 0;
		}hr;
	public:
		~data() = default;
		data(const std::string& y, const std::string& m, const std::string& d);

		std::string show_time() const { return y + "/" + m + "/" + d; }
		std::string show_y() const { return y; }
		std::string show_m() const { return m; }
		std::string show_d() const { return d; }
		Body show_body() const { return body; }
		double show_high() const { return body.high; }
		double show_weight() const { return body.weight; }
		HR show_hr() const { return hr; }

		void printf_time() const { std::cout << this->show_time() << std::endl; }
		void printf_body() const;
		void printf_hr() const;
		void printf_all()const;

		void add_Body(const double& h, const double& w);
		void add_Body(const data& other);
		void add_HR(const int& RHR, const int& Avg_HR,
			const int& SHR, const int& MAX_HR, const int& MINN_HR, const int& SpO2);
	};
}