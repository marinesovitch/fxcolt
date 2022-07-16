#ifndef INC_BACKEND_COMMUNICATORIMPL_H
#define INC_BACKEND_COMMUNICATORIMPL_H

namespace fx
{

struct ICommunicator;
struct IAccountManager;

ICommunicator* createCommunicator(IAccountManager* accountManager);

} // namespace fx

#endif
