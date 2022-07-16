#ifndef INC_COMMON_TRADERCOMMANDPARSER_H
#define INC_COMMON_TRADERCOMMANDPARSER_H

#include "smartTypes.h"
#include "command.h"

namespace fx
{

HCommand parseTraderCommand(const std::string& commandStr, std::string* error);
HCommand parseTraderCommand(const KCommand::EOperation operation, std::istringstream& cmdLine);

}

#endif
