#ifndef INC_BACKEND_TRADERIMPL_H
#define INC_BACKEND_TRADERIMPL_H

#include "common/baseTypes.h"

namespace fx
{

struct ITrader;

ITrader* createTrader( const account_key_t& key );

} // namespace fx

#endif
