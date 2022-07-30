// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "datetime.h"
#include <chrono>

namespace cpp 
{

uint64_t miliCount()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	uint64_t result = currentTime.time_since_epoch().count();
	return result;
}

STimeInfo::STimeInfo(
    const unsigned int year,
    const unsigned int month, 
    const unsigned int day,
    const unsigned int hour,
    const unsigned int minute,
    const unsigned int second )
    : m_year( year )
    , m_month( month )
    , m_day( day )
    , m_hour( hour )
    , m_minute( minute ) 
    , m_second( second )
{
}

bool getLocalTime(STimeInfo* timeInfo)
{
	bool result = false;
	std::time_t currentTime = std::time(0);
	std::tm* localTime = std::localtime(&currentTime);
	if (localTime != nullptr)
	{
		timeInfo->m_year = localTime->tm_year + 1900;
		timeInfo->m_month = localTime->tm_mon + 1;
		timeInfo->m_day = localTime->tm_mday;
		timeInfo->m_hour = localTime->tm_hour;
		timeInfo->m_minute = localTime->tm_min;
		timeInfo->m_second = localTime->tm_sec;
		result = true;
	}
	return result;
}

} // namespace cpp
