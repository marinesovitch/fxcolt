// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
# ifndef INC_CPP_DATETIME_H
# define INC_CPP_DATETIME_H

namespace cpp 
{

uint64_t miliCount();

// ---------------------------------------------------------------------------

struct STimeInfo
{
    STimeInfo(
        const unsigned int year = 0,
        const unsigned int month = 0, 
        const unsigned int day = 0,
        const unsigned int hour = 0,
        const unsigned int minute = 0,
        const unsigned int second = 0 );

    unsigned int m_year;
    unsigned int m_month; // 1-12
    unsigned int m_day; // 1-31
    unsigned int m_hour; //0-23
    unsigned int m_minute; //0-59
    unsigned int m_second; //0-59
};

bool getLocalTime(STimeInfo* timeInfo);

} // namespace cpp

#endif
