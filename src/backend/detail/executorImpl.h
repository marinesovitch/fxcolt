// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_EXECUTORIMPL_H
#define INC_BACKEND_EXECUTORIMPL_H

namespace fx
{

struct IExecutor;
struct IAccountManager;
struct ITradeManager;

IExecutor* createExecutor( 
	std::ostream* cout,
	std::ostream* cerr,
	IAccountManager* accountManager, 
	ITradeManager* tradeManager );

} // namespace fx

#endif
