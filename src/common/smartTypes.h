// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_COMMON_SMARTTYPES_H
#define INC_COMMON_SMARTTYPES_H

namespace fx
{

struct STick;
typedef std::shared_ptr<STick> HTick;

struct SOrder;
typedef std::shared_ptr<SOrder> HOrder;

struct IConnection;
typedef std::shared_ptr<IConnection> HConnection;

struct ITrader;
typedef std::shared_ptr<ITrader> HTrader;

struct ITradingStrategy;
typedef std::shared_ptr<ITradingStrategy> HTradingStrategy;

class KCommand;
typedef std::shared_ptr<KCommand> HCommand;

} // namespace fx

#endif
