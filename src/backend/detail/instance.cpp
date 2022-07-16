#include "ph.h"
#include "instance.h"
#include "accountManager.h"
#include "accountManagerImpl.h"
#include "communicator.h"
#include "communicatorImpl.h"
#include "executor.h"
#include "executorImpl.h"
#include "climber.h"
#include "tradeManager.h"
#include "tradeManagerImpl.h"

namespace fx
{

KInstance::KInstance(
	std::ostream* cout,
	std::ostream* cerr)
	: m_cout(cout)
	, m_cerr(cerr)
	, m_accountManager(createAccountManager())
	, m_communicator(createCommunicator( m_accountManager ))
	, m_trademanager(createTradeManager( m_communicator ))
	, m_executor(createExecutor( cout, cerr, m_accountManager, m_trademanager ))
{
}

KInstance::~KInstance()
{
	delete m_executor;
	delete m_trademanager;
	delete m_communicator;
	delete m_accountManager;
}

void KInstance::run()
{
	ITradingStrategyFactory* climberFactory = fx::createClimberFactory();
	m_executor->registerStrategy(climberFactory);

	m_communicator->run();
}

// ---------------------------------------------------------------------------

std::ostream& KInstance::cout() const
{
	return *m_cout;
}

std::ostream& KInstance::cerr() const
{
	return *m_cerr;
}

IAccountManager& KInstance::accountManager() const
{
	return *m_accountManager;
}

ICommunicator& KInstance::communicator() const
{
	return *m_communicator;
}

IExecutor& KInstance::executor() const
{
	return *m_executor;
}

ITradeManager& KInstance::tradeManager() const
{
	return *m_trademanager;
}

} // namespace fx
