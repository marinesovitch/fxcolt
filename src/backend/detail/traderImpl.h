// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_TRADERIMPL_H
#define INC_BACKEND_TRADERIMPL_H

#include "common/baseTypes.h"

namespace fx
{

struct ITrader;

ITrader* createTrader( const account_key_t& key );

} // namespace fx

#endif
