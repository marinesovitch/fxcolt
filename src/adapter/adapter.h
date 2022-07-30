// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_ADAPTER_ADAPTER_H
#define INC_ADAPTER_ADAPTER_H

#ifdef ADAPTER_EXPORTS
#define ADAPTER_API extern "C" __declspec(dllexport)
#else
#define ADAPTER_API extern "C" __declspec(dllimport)
#endif

#include "common/baseTypes.h"
#include "common/order.h"
#include "common/symbolInfo.h"

#define stdcall __stdcall

ADAPTER_API void stdcall RegisterSymbol(const wchar_t* broker, const fx::account_login_t accountLogin, const wchar_t* symbol);
ADAPTER_API void stdcall UnregisterSymbol(const wchar_t* symbol);

ADAPTER_API void stdcall DumpTick(
	const wchar_t* symbol,
	const fx::datetime_t time,
	const fx::price_t bid,
	const fx::price_t ask,
	const fx::price_t last);

ADAPTER_API int stdcall GetMaxCmdArgCount();
ADAPTER_API int stdcall GetMaxCmdStringLen();
ADAPTER_API int stdcall GetMaxCmdTicketCount();
ADAPTER_API bool stdcall GetCommand(wchar_t* cmd, int* argCount, fx::MqlStr* args, int* ticketCount, int tickets[]);
ADAPTER_API void stdcall OnCommandCompleted(const wchar_t* result);
ADAPTER_API void stdcall DumpSymbol(
	const wchar_t* name);
ADAPTER_API void stdcall DumpOrder(
	const wchar_t* symbol,
	const fx::ticket_t ticket,
	const fx::SOrder::EType type,
	const fx::volume_t lots,
	const fx::price_t openPrice,
	const fx::price_t closePrice,
	const fx::price_t stopLoss,
	const fx::price_t takeProfit,
	const fx::datetime_t openTime,
	const fx::datetime_t expirationTime,
	const fx::datetime_t closeTime,
	const fx::price_t commission,
	const fx::price_t swap,
	const fx::price_t profit);

ADAPTER_API void stdcall LogWrite(const wchar_t* msg);
ADAPTER_API void stdcall LogWriteln(const wchar_t* msg);

#endif
