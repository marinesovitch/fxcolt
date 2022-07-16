#ifndef INC_BACKEND_CONNECTION_H
#define INC_BACKEND_CONNECTION_H

#include "common/smartTypes.h"

namespace fx
{

struct ITraderSink;

struct IConnection
{
	public:
		virtual ~IConnection();

	public:
		virtual bool isConnected() const = 0;
		virtual void sendCommand( HCommand command ) = 0;
		//virtual void disconnect() = 0;

};

typedef std::shared_ptr < IConnection > HConnection;

} // namespace fx

#endif
