// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_COMMAND_TYPES_H
#define INC_COMMON_COMMAND_TYPES_H

#include "types.h"
#include "cpp/types.h"

namespace fx
{

struct SNewOrder;
struct SModifyOrder;
struct ICommandVisitor;

class KCommand
{
	public:
		enum EOperation
		{
			ListSymbols,
			Get,
			Open,
			Close,
			CloseAll,
			Modify,
			SetStopLoss,
			SetTakeProfit,

			GetSymbols,
			ShowTicks,
			HideTicks,

			Unknown
		};

	protected:
		KCommand(
			const EOperation operation);
		KCommand(
			const EOperation operation,
			const tickets_t& tickets);
		KCommand(
			const EOperation operation,
			const cpp::strings_t& args);
		KCommand(
			const EOperation operation,
			const std::string& arg,
			const tickets_t& tickets);
		KCommand(
			const EOperation operation,
			const cpp::strings_t& args,
			const tickets_t& tickets);

	public:
		virtual ~KCommand();

		std::string toString() const;

		const std::string& name() const;
		const cpp::strings_t& args() const;
		const tickets_t& tickets() const;

	public:
		virtual bool isLocal() const;
		virtual void accept( ICommandVisitor* visitor );

	public:
		static EOperation operationFromStr(const std::string& operationStr);

	private:
		const EOperation m_operation;
		const cpp::strings_t m_args;
		const tickets_t m_tickets;
};

// ---------------------------------------------------------------------------

class KCmdListSymbols : public KCommand
{
	public:
		KCmdListSymbols();
};

class KCmdGet : public KCommand
{
	public:
		KCmdGet(const tickets_t& tickets);
};

class KCmdOpen : public KCommand
{
	public:
		KCmdOpen(const SNewOrder& newOrder);
};

class KCmdClose : public KCommand
{
	public:
		KCmdClose(const tickets_t& tickets);
};

class KCmdCloseAll : public KCommand
{
	public:
		KCmdCloseAll();
};

class KCmdModify : public KCommand
{
	public:
		KCmdModify(const SModifyOrder& modifyOrder, const tickets_t& tickets);
};

class KCmdSetStopLoss : public KCommand
{
	public:
		KCmdSetStopLoss(const SPrice& stopLoss, const tickets_t& tickets);
};

class KCmdSetTakeProfit : public KCommand
{
	public:
		KCmdSetTakeProfit(const SPrice& takeProfit, const tickets_t& tickets);
};

// ---------------------------------------------------------------------------

class KLocalCommand : public KCommand
{
	public:
		KLocalCommand(
			const EOperation operation);

	public:
		virtual bool isLocal() const;
};

// ---------------------------------------------------------------------------

class KCmdGetSymbols : public KLocalCommand
{
	public:
		KCmdGetSymbols();

	public:
		virtual void accept( ICommandVisitor* visitor );
};

class KCmdShowTicks : public KLocalCommand
{
	public:
		KCmdShowTicks();

	public:
		virtual void accept( ICommandVisitor* visitor );
};

class KCmdHideTicks : public KLocalCommand
{
	public:
		KCmdHideTicks();

	public:
		virtual void accept( ICommandVisitor* visitor );
};

// ---------------------------------------------------------------------------

struct ICommandVisitor
{
	virtual ~ICommandVisitor();
	virtual void visitGetSymbols( KCmdGetSymbols* cmd ) = 0;
	virtual void visitShowTicks( KCmdShowTicks* cmd ) = 0;
	virtual void visitHideTicks( KCmdHideTicks* cmd ) = 0;
	virtual void visitDefault( KCommand* cmd ) = 0;
};

// ---------------------------------------------------------------------------

void getTraderCommandsHelp(std::ostream& os);

} // namespace fx

#endif
