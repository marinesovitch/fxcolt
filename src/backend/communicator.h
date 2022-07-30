// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_COMMUNICATOR_H
#define INC_BACKEND_COMMUNICATOR_H

#include "common/notes.h"
#include "common/types.h"
#include "connection.h"

namespace fx
{

struct IConnection;
struct ITraderSink;

struct ICommunicatorObserver
{
	virtual void onNewAccountDetected(const account_key_t& key) = 0;
	virtual void onSymbolNote(const account_key_t& key, const note::EKind noteKind, const std::string& symbol) = 0;
};

struct ICommunicator
{
	public:
		virtual ~ICommunicator();

	public:
		virtual void setObserver(ICommunicatorObserver* observer) = 0;

		virtual void run() = 0;

		virtual HConnection connect(const account_key_t& key, ITraderSink* sink) = 0;

};

} // namespace fx

#endif
