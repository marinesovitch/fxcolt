#ifndef INC_COMMON_UTILS_H
#define INC_COMMON_UTILS_H

#include "types.h"

namespace fx
{

namespace utils
{

std::string normalizeBrokerName(const std::string& rawBrokerName);
bool isNormalizedBrokerName(const std::string& brokerName);

std::string preparePath(const EMedium kind, const std::string& dir);
std::string preparePath(const EMedium kind, const SAccountInfo& accountInfo, const std::string& tag, const int index = -1);

void sleep(const int durationMilliseconds);

} // namespace utils

} // namespace fx

#endif
