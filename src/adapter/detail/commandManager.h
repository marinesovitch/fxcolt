#ifndef INC_ADAPTER_COMMANDMANAGER_H
#define INC_ADAPTER_COMMANDMANAGER_H

#include "common/baseTypes.h"

namespace fx
{

struct SAccountInfo;
struct SOrder;

struct ICommandManager
{
	public:
		virtual ~ICommandManager();

	public:
		virtual void run() = 0;

		virtual bool getCommand(wchar_t* wcmdName, int* argCount, MqlStr wargs[], int* ticketCount, int tickets[]) = 0;
		virtual void onCommandCompleted(const wchar_t* result) = 0;

};

ICommandManager* createCommandManager(const SAccountInfo& accountInfo);

} // namespace adapter

#endif
