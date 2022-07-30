// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_BACKEND_COMMUNICATORIMPL_H
#define INC_BACKEND_COMMUNICATORIMPL_H

namespace fx
{

struct ICommunicator;
struct IAccountManager;

ICommunicator* createCommunicator(IAccountManager* accountManager);

} // namespace fx

#endif
