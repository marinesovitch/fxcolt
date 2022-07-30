// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "climber.h"
#include "tradingStrategy.h"
#include "tradingStrategyFactory.h"
#include "common/types.h"

namespace fx
{

namespace
{

const std::string ClimberStrategyName = "climber";

struct SClimberParams
{
	SPrice m_openPrice;
	SPrice m_stopLoss;
	SVolume m_stepSize;
	SPrice m_profitMargin;
};

// ---------------------------------------------------------------------------

class KClimber : public ITradingStrategy
{
	public:
		KClimber();

	public:
		virtual const std::string& getName() const;

		virtual void onTick(const STick& tick);
		virtual void onOrder(const SOrder& order);

		virtual void executeCommand(std::istringstream& cmdLine);
};

// ---------------------------------------------------------------------------

KClimber::KClimber()
{
}

const std::string& KClimber::getName() const
{
	return ClimberStrategyName;
}

void KClimber::onTick(const STick& tick)
{
}

void KClimber::onOrder(const SOrder& order)
{
}

void KClimber::executeCommand(std::istringstream& cmdLine)
{
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class KClimberFactory : public ITradingStrategyFactory
{
	public:
		virtual void getNames(cpp::strings_t* strategies) const;
		virtual HTradingStrategy create(const std::string& name);

};

void KClimberFactory::getNames(cpp::strings_t* strategies) const
{
	*strategies = { ClimberStrategyName };
}

HTradingStrategy KClimberFactory::create(const std::string& name)
{
	HTradingStrategy result;
	if ( name == ClimberStrategyName )
	{
		result.reset( new KClimber() );
	}
	else
	{
		assert( !"unexpected strategy name" );
	}
	return result;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

ITradingStrategyFactory* createClimberFactory()
{
	ITradingStrategyFactory* factory = new KClimberFactory();
	return factory;
}

} // namespace fx
