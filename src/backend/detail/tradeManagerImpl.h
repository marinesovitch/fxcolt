#ifndef INC_BACKEND_TRADERMANAGERIMPL_H
#define INC_BACKEND_TRADERMANAGERIMPL_H

namespace fx
{

struct ITradeManager;
struct ICommunicator;
 
ITradeManager* createTradeManager(ICommunicator* communicator);

} // namespace fx

#endif
