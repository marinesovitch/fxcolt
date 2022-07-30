// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_CONSTS_H
#define INC_COMMON_CONSTS_H

#include "baseTypes.h"

namespace fx
{

namespace consts
{

extern const std::string AppTag;
extern const std::string NoteSlotName;
extern const std::string TickPipeName;
extern const std::string SymbolPipeName;
extern const std::string OrderPipeName;
extern const std::string CmdPipeName;

extern const std::string PipePrefix;
extern const std::string MailSlotPrefix;

const std::size_t MaxSymbolNameLen = 16;

const std::size_t MaxMailSlotMsgLen = 512;

const std::size_t NamedPipeBufferSize = 256 * 1024;

const int MaxCmdArgCount = 16;
const int MaxCmdArgLen = 128;
const int MaxCmdTicketCount = 32 * 1024;

extern const std::string CmdSendSuccess;

extern const std::string CmdExit;

} // namespace consts

} // namespace fx

#endif
