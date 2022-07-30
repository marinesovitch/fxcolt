// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_TRADINGSTRATEGY_H
#define INC_BACKEND_TRADINGSTRATEGY_H

namespace fx
{

struct STick;
struct SOrder;

struct ITradingStrategy
{
	virtual ~ITradingStrategy();

	virtual const std::string& getName() const = 0;

	virtual void onTick(const STick& tick) = 0;
	virtual void onOrder(const SOrder& order) = 0;

	virtual void executeCommand(std::istringstream& cmdLine) = 0;
};

} // namespace fx

#endif
