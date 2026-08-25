#include"data.h"
namespace Dyer
{
	data::data(const std::string& y, const std::string& m, const std::string& d)
		:y(y), m(m), d(d){ }
	void data::add_Body(const double& h, const double& w)
	{
		body.high = h;
		body.weight = w;
	}
	void data::add_Body(const data& other)
	{
		body = other.body;
	}
	void data::add_HR(const int& RHR, const int& Avg_HR,
					  const int& SHR, const int& MAX_HR, const int& MINN_HR, const int& SpO2)
	{
		hr.RHR = RHR;
		hr.SpO2 = SpO2;
		hr.Avg_HR = Avg_HR;
		hr.SHR = SHR;
		hr.MAX_HR = MAX_HR;
		hr.MINN_HR = MINN_HR;
	}
	void data::printf_body() const
	{
		std::cout << "high: " << body.high << "m  " << "weight: " << body.weight << "kg\n";
	}
	void data::printf_hr() const
	{
		std::cout << "RHR: " << hr.RHR << ",SHR: " << hr.SHR << ",MAX_HR: " << hr.MAX_HR
				  << ",MINN_HR:" << hr.MINN_HR << ",Avg_HR:" << hr.Avg_HR 
			      << ",SpO2:" << hr.SpO2<<"\n";
	}
	void data::printf_all() const
	{
		printf_time();
		printf_body();
		printf_hr();
	}
}