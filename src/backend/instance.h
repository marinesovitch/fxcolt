// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_INSTANCE_H
#define INC_BACKEND_INSTANCE_H

#include "backend.h"

namespace fx
{

struct IAccountManager;
struct ICommunicator;
struct IExecutor;
struct ITradeManager;

class BACKEND_API KInstance
{
	public:
		KInstance(
			std::ostream* cout,
			std::ostream* cerr);
		~KInstance();

		void run();

	public:
		std::ostream& cout() const;
		std::ostream& cerr() const;
		IAccountManager& accountManager() const;
		ICommunicator& communicator() const;
		ITradeManager& tradeManager() const;
		IExecutor& executor() const;

	private:
		std::ostream* m_cout;
		std::ostream* m_cerr;
		IAccountManager* m_accountManager;
		ICommunicator* m_communicator;
		ITradeManager* m_trademanager;
		IExecutor* m_executor;

};

} // namespace fx

#endif
