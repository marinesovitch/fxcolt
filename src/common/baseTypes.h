#ifndef INC_COMMON_BASETYPES_H
#define INC_COMMON_BASETYPES_H

#include <cpp/handle.h>

namespace fx
{

typedef int64_t datetime_t;
typedef double price_t;
typedef double volume_t;

fx_define_handle_type64(account_login);
fx_define_handle_type(account_key);
fx_define_handle_type(ticket);

typedef std::vector<account_key_t> account_keys_t;
typedef std::vector<ticket_t> tickets_t;

struct MqlStr
{
    int m_length;
    wchar_t* m_data;
    int reserved;
};

} // namespace fx

#endif
