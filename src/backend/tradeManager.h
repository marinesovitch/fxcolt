#ifndef INC_BACKEND_TRADERMANAGER_H
#define INC_BACKEND_TRADERMANAGER_H

#include "common/baseTypes.h"
#include "common/smartTypes.h"

namespace fx
{

struct ITradeManager
{
	public: 
		virtual ~ITradeManager();

	public: 
		virtual void run() = 0;
		virtual HTrader getTrader(const account_key_t& key) const = 0;

};

} // namespace fx

#endif
