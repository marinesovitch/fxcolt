#ifndef INC_COMMON_COMMANDPARSERBASE_H
#define INC_COMMON_COMMANDPARSERBASE_H

#include "order.h"

namespace fx
{

class KCommandParserBase
{
	protected:
		KCommandParserBase(const std::string& cmdLine);
		KCommandParserBase(std::istringstream& cmdLine);

	public:
		virtual ~KCommandParserBase();

	protected:
		SNewOrder parseNewOrder();
		SModifyOrder parseModifyOrder();
		void parseTickets(const bool obligatory = true);
		SOrder::EType parseOrderType();
		SVolume parseVolume();
		SPrice parsePrice(const bool obligatory = true);
		SDateTime parseDateTime(const bool obligatory = false);

		std::string getNextToken(const bool obligatory = true, const std::string& defaultValue = "");
		bool getNextToken(std::string* token);

		void parseError( const std::string& reason ) const;

	protected:
		std::istringstream m_cmdLine;
		std::string m_token;
		tickets_t m_tickets;

};

} // namespace fx

#endif
