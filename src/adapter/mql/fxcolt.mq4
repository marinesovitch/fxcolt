//+------------------------------------------------------------------+
//|                                                       fxcolt.mq4 |
//|                    Copyright 2014-2015, 2022, Dariusz Slusarczyk |
//|                                 https://github.com/marinesovitch |
//+------------------------------------------------------------------+
#property copyright "Copyright 2014-2015, 2022, Dariusz Slusarczyk"
#property link      "https://github.com/marinesovitch"
#property version   "1.00"
#property strict

// ---------------------------------------------------------------------------

#import "mtfxcolt.dll"
	int RegisterSymbol(const string broker, const long accountLogin, const string symbol);
	int UnregisterSymbol(const string symbol);

	int DumpTick(const string symbol, const long time, const double bid, const double ask, const double last);

	int GetMaxCmdArgCount();
	int GetMaxCmdStringLen();
	int GetMaxCmdTicketCount();
	bool GetCommand(string& cmd, int& argCount, string& args[], int& ticketCount, int& tickets[]);
	void OnCommandCompleted(const string result);
	void DumpSymbol(const string name);
	void DumpOrder(
		const string symbol,
		const int ticket,
		const int type,
		const double lots,
		const double openPrice,
		const double closePrice,
		const double stopLoss,
		const double takeProfit,
		const long openTime,
		const long expirationTime,
		const long closeTime,
		const double commission,
		const double swap,
		const double profit);

	void LogWrite(string msg);
	void LogWriteln(string msg);
#import

int MaxCmdStringLen = 0;
string CmdArgs[];
int CmdArgCount;
int CmdTickets[];
int CmdTicketCount;
string CmdResult;

const int DefaultSlippage = 5;

// ---------------------------------------------------------------------------

int OnInit()
{
	if (!IsDemo())
	{
		MessageBox("It works on demo accounts only.");
		return(INIT_FAILED);
	}

	if (!IsDllsAllowed())
	{
		MessageBox("You need to turn on \'Allow DLL imports\'");
		return(INIT_FAILED);
	}

	int MaxCmdArgCount = GetMaxCmdArgCount();
	LogWriteln(StringConcatenate("MaxCmdArgCount ", MaxCmdArgCount));
	ArrayResize(CmdArgs, MaxCmdArgCount);

	MaxCmdStringLen = GetMaxCmdStringLen();
	LogWriteln(StringConcatenate("MaxCmdStringLen ", MaxCmdStringLen));
	for (int i = 0; i < MaxCmdArgCount; ++i)
	{
		//marines: there has to be used any char other than 0, e.g. ' '
		//DON'T change it, else strage effects appears while working on these strings in C++ layer
		StringInit(CmdArgs[i], MaxCmdStringLen, ' ');
	}

	int MaxCmdTicketCount = GetMaxCmdTicketCount();
	ArrayResize(CmdTickets, MaxCmdTicketCount);

	string sym = Symbol();
	LogWriteln(StringConcatenate("----------------- ", AccountInfoString(ACCOUNT_COMPANY), " ", AccountInfoInteger(ACCOUNT_LOGIN), " " , Symbol(), " -----------------"));
	LogWriteln(StringConcatenate("before ", AccountInfoString(ACCOUNT_COMPANY), AccountInfoInteger(ACCOUNT_LOGIN), " ", sym));
	RegisterSymbol(AccountInfoString(ACCOUNT_COMPANY), AccountInfoInteger(ACCOUNT_LOGIN), sym);

	LogWriteln(StringConcatenate("After RegisterSymbol"));

	EventSetTimer(1);

	return(INIT_SUCCEEDED);
}

void OnDeinit(const int reason)
{
	UnregisterSymbol(Symbol());
	LogWriteln(StringConcatenate("----------------- DEINIT ", AccountInfoString(ACCOUNT_COMPANY), " ", AccountInfoInteger(ACCOUNT_LOGIN), " " , Symbol(), " -----------------"));
}

// ---------------------------------------------------------------------------

void OnTick()
{
	MqlTick tick;
	if (SymbolInfoTick(Symbol(), tick))
	{
		DumpTick(Symbol(), tick.time, tick.bid, tick.ask, tick.last);
	}
}

void OnTimer()
{
	//LogWriteln(StringConcatenate("OnTimer"));
	string cmd;
	StringInit(cmd, MaxCmdStringLen);
	while(GetCommand(cmd, CmdArgCount, CmdArgs, CmdTicketCount, CmdTickets))
	{
		LogWriteln(StringConcatenate("ExecuteCommand '", cmd, "' ", CmdArgCount, " ", CmdTicketCount));
		for ( int i = 0; i < CmdArgCount; ++i)
		{
			LogWriteln(StringConcatenate("arg ", i, " ", CmdArgs[i]));
			Print("CmdArgs[i] ", i, "'", CmdArgs[i], "'");
		}
		for ( int j = 0; j < CmdTicketCount; ++j)
		{
			LogWriteln(StringConcatenate("ticket ", j, " ", CmdTickets[j]));
		}
		ExecuteCommand(cmd);
	}
	//LogWriteln(StringConcatenate("OnTimer end"));
}

void ExecuteCommand(string cmd)
{
	CmdResult = "";
	LogWriteln(StringConcatenate("ExecuteCommand '", cmd, "'"));
	if (cmd == "list_symbols")
	{
		ExecuteListSymbolsCommand();
	}
	else if (cmd == "get")
	{
		ExecuteGetCommand();
	}
	else if (cmd == "open")
	{
		ExecuteOpenCommand();
	}
	else if (cmd == "modify")
	{
		ExecuteModifyCommand();
	}
	else if (cmd == "set_stop_loss")
	{
		ExecuteSetStopLossCommand();
	}
	else if (cmd == "set_take_profit")
	{
		ExecuteSetTakeProfitCommand();
	}
	else if (cmd == "close")
	{
		ExecuteCloseCommand();
	}
	else if (cmd == "close_all")
	{
		ExecuteCloseAllCommand();
	}

	if (CmdResult == "")
	{
		CmdResult = "success";
	}
	OnCommandCompleted(CmdResult);
}

// ---------------------------------------------------------------------------

void ExecuteListSymbolsCommand()
{
	LogWriteln(StringConcatenate("ExecuteListSymbolsCommand"));
	const string filter = GetStringArg(0);
	const bool onlySelectedInMarketWatch = (filter != "all");

	int symbolsCount = SymbolsTotal(onlySelectedInMarketWatch);
	for (int pos = 0; pos < symbolsCount; ++pos)
	{
		LogWriteln(StringConcatenate("before DumpSymbolByPos ", pos));
		string symbolName = SymbolName(pos, onlySelectedInMarketWatch);
		DumpSymbol(symbolName);
		LogWriteln(StringConcatenate("after DumpSymbolByPos ", pos));
	}
}

void ExecuteGetCommand()
{
	LogWriteln(StringConcatenate("ExecuteGetCommand"));
	if (CmdTicketCount == 0)
	{
		InternalGetAllOrders();
	}
	else
	{
		InternalGetOrdersByTicket();
	}
}

void ExecuteOpenCommand()
{
	const string symbol = GetStringArg(0);
	const int type = GetOperationArg(1);
	const double volume = GetDoubleArg(2);
	const double price = GetDoubleArg(3);
	const int slippage = DefaultSlippage;
	const double stopLoss = GetDoubleArg(4);
	const double takeProfit = GetDoubleArg(5);
	const datetime expiration = GetDateTimeArg(6, 0);
	const int ticket = OrderSend(
		symbol,
		type,
		volume,
		price,
		slippage,
		stopLoss,
		takeProfit,
		NULL,
		0,
		expiration);
	if (ticket != -1)
	{
		DumpOrderByTicket(ticket);
	}
	else
	{
		AddToCmdResult("OrderSend error");
	}
}

void ExecuteModifyCommand()
{
	for (int index = 0; index < CmdTicketCount; index++)
	{
		int ticket = CmdTickets[index];
		if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES))
		{
			const double openPrice = GetPriceArg(0, OrderOpenPrice());
			const double stopLoss = GetPriceArg(1, OrderStopLoss());
			const double takeProfit = GetPriceArg(2, OrderTakeProfit());
			const datetime expiration = GetDateTimeArg(3, OrderExpiration());
			if (!OrderModify(
					ticket,
					openPrice,
					stopLoss,
					takeProfit,
					expiration))
			{
				AddToCmdResult(StringConcatenate("OrderModify error ", ticket));
			}
		}
	}
}

void ExecuteSetStopLossCommand()
{
	if (CmdArgCount == 1)
	{
		const double stopLoss = GetDoubleArg(0);
		for (int index = 0; index < CmdTicketCount; ++index)
		{
			int ticket = CmdTickets[index];
			if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES))
			{
				if (!OrderModify(
					   ticket,
					   OrderOpenPrice(),
					   stopLoss,
					   OrderTakeProfit(),
					   OrderExpiration()))
			   {
					AddToCmdResult(StringConcatenate("ExecuteSetStopLossCommand OrderModify failure ", ticket));
			   }
			}
		}
	}
}

void ExecuteSetTakeProfitCommand()
{
	if (CmdArgCount == 1)
	{
		const double takeProfit = GetDoubleArg(0);

		for (int index = 0; index < CmdTicketCount; ++index)
		{
			int ticket = CmdTickets[index];
			if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES))
			{
				if (!OrderModify(
					   ticket,
					   OrderOpenPrice(),
					   OrderStopLoss(),
					   takeProfit,
					   OrderExpiration()))
			   {
					AddToCmdResult(StringConcatenate("ExecuteSetTakeProfitCommand OrderModify failure ", ticket));
			   }
			}
		}
	}
}

void ExecuteCloseCommand()
{
	for (int index = 0; index < CmdTicketCount; index++)
	{
		int ticket = CmdTickets[index];
		if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES))
		{
			InternalOrderClose(ticket);
		}
	}
	LogWriteln(StringConcatenate("ExecuteCloseCommand end"));
}

void ExecuteCloseAllCommand()
{
	const int ordersCount = OrdersTotal();
	int tickets[];
	ArrayResize(tickets, ordersCount);
	for (int index = 0; index < ordersCount; index++)
	{
		if (OrderSelect(index, SELECT_BY_POS, MODE_TRADES))
		{
			tickets[index] = OrderTicket();
		}
	}

	for (int index = 0; index < ordersCount; index++)
	{
		const int ticket = tickets[index];
		InternalOrderClose(ticket);
	}
}

// ---------------------------------------------------------------------------

string GetStringArg(int argIndex)
{
	const string result = CmdArgs[argIndex];
	return result;
}

int GetIntArg(int argIndex)
{
	const string arg = CmdArgs[argIndex];
	const int result = (int)StringToInteger(arg);
	return result;
}

double GetDoubleArg(int argIndex)
{
	const string arg = CmdArgs[argIndex];
	const double result = StringToDouble(arg);
	return result;
}

double GetPriceArg(int argIndex, double defaultValue = 0.0)
{
	double result;
	const string arg = CmdArgs[argIndex];
	if (arg == "default")
	{
		result = defaultValue;
	}
	else if (arg == "bid")
	{
		result = defaultValue;
	}
	else if (arg == "ask")
	{
		result = defaultValue;
	}
	else
	{
		result = StringToDouble(arg);
	}
	return result;
}

datetime GetDateTimeArg(int argIndex, datetime defaultValue = 0)
{
	const string arg = CmdArgs[argIndex];
	const datetime result = (arg == "0") ? defaultValue : StringToTime(arg);
	return result;
}

int GetOperationArg(int argIndex)
{
	const string arg = CmdArgs[argIndex];
	if (arg == "Buy")
	{
		return OP_BUY;
	}
	else if (arg == "Sell")
	{
		return OP_SELL;
	}
	else if (arg == "BuyLimit")
	{
		return OP_BUYLIMIT;
	}
	else if (arg == "SellLimit")
	{
		return OP_SELLLIMIT;
	}
	else if (arg == "BuyStop")
	{
		return OP_BUYSTOP;
	}
	else if (arg == "SellStop")
	{
		return OP_SELLSTOP;
	}
	else
	{
		LogWriteln(StringConcatenate("incorrect operation: ", arg));
		return -1;
	}
}

// ---------------------------------------------------------------------------

void InternalGetAllOrders()
{
	int ordersCount = OrdersTotal();
	for (int pos = 0; pos < ordersCount; ++pos)
	{
		LogWriteln(StringConcatenate("before DumpOrderByPos ", pos));
		DumpOrderByPos(pos);
		LogWriteln(StringConcatenate("after DumpOrderByPos ", pos));
	}
}

void InternalGetOrdersByTicket()
{
	for (int i = 0; i < CmdTicketCount; ++i)
	{
		const int ticket = CmdTickets[i];
		LogWriteln(StringConcatenate("before DumpOrderByTicket ", ticket));
		DumpOrderByTicket(ticket);
		LogWriteln(StringConcatenate("after DumpOrderByTicket ", ticket));
	}
}

void InternalOrderClose(int ticket)
{
	LogWriteln(StringConcatenate("InternalOrderClose before ", ticket));
	bool success = false;
	if (IsOrderPending())
	{
		success = OrderDelete(ticket);
	}
	else
	{
		success = OrderClose(
			ticket,
			OrderLots(),
			OrderClosePrice(), //0,//TODO CurrentPrice,
			DefaultSlippage);
	}

	if (!success)
	{
		AddToCmdResult(StringConcatenate("InternalOrderClose error ", ticket, " pending=", IsOrderPending()));
	}
}

bool IsOrderPending()
{
	int type = OrderType();
	bool result = (type != OP_BUY) && (type != OP_SELL);
	return result;
}

// ---------------------------------------------------------------------------

void AddToCmdResult(string description)
{
	const int lastErrorCode = GetLastError();
	string msg = StringConcatenate(description, " (errorCode: ", lastErrorCode, ")");
	LogCmdResult(msg);
	CmdResult = StringConcatenate(CmdResult, msg, ";");
}

void LogCmdResult(string msg)
{
	LogWriteln(msg);
	// LogWriteln(ErrorDescription());
}

// ---------------------------------------------------------------------------

void DumpOrderByPos(int pos)
{
	if (OrderSelect(pos, SELECT_BY_POS, MODE_TRADES))
	{
		DumpSelectedOrder();
	}
}

void DumpOrderByTicket(int ticket)
{
	if (OrderSelect(ticket, SELECT_BY_TICKET, MODE_TRADES))
	{
		DumpSelectedOrder();
	}
}

void DumpSelectedOrder()
{
	DumpOrder(
		OrderSymbol(),
		OrderTicket(),
		OrderType(),
		OrderLots(),
		OrderOpenPrice(),
		OrderClosePrice(),
		OrderStopLoss(),
		OrderTakeProfit(),
		OrderOpenTime(),
		OrderExpiration(),
		OrderCloseTime(),
		OrderCommission(),
		OrderSwap(),
		OrderProfit());
//				OrderMagicNumber());
}
