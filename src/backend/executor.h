#ifndef INC_BACKEND_EXECUTOR_H
#define INC_BACKEND_EXECUTOR_H

#include "common/types.h"
#include "common/smartTypes.h"

namespace fx
{

struct ITradingStrategyFactory;

struct IExecutor
{
	public:
		virtual ~IExecutor();

	public:
		virtual bool executeCommand(const account_key_t& key, HCommand command, std::string* error) = 0;
		virtual bool executeCommand(const std::string& cmdLine, std::string* error) = 0;

		virtual void registerStrategy( ITradingStrategyFactory* tradingStrategyFactory ) = 0;

};

} // namespace fx

#endif
