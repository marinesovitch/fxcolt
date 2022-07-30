// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_TRADINGSTRATEGYFACTORY_H
#define INC_BACKEND_TRADINGSTRATEGYFACTORY_H

#include "common/smartTypes.h"
#include "cpp/types.h"

namespace fx
{

struct ITradingStrategyFactory
{
	virtual ~ITradingStrategyFactory();

	virtual void getNames(cpp::strings_t* strategies) const = 0;
	virtual HTradingStrategy create(const std::string& name) = 0;
};

} // namespace fx

#endif
