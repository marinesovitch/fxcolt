// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_COMMANDQUEUE_H
#define INC_COMMON_COMMANDQUEUE_H

#include "cpp/threadsafe_queue.h"

namespace fx
{

typedef cpp::threadsafe_queue<HCommand> command_queue;

}

#endif

