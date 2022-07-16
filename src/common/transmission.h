#ifndef INC_COMMON_TRANSMISSION_H
#define INC_COMMON_TRANSMISSION_H

#include "common/namedPipe.h"
#include "common/utils.h"
#include "cpp/threadsafe_queue.h"

namespace fx
{

template<typename TItem>
class KTransmitter
{
	public:
		KTransmitter(const SAccountInfo& accountInfo)
			: m_accountInfo(accountInfo)
		{
		}

		void run(const std::string& tag)
		{
			std::thread transmissionLoopThread(&KTransmitter::transmissionLoop, this, tag);
			transmissionLoopThread.detach();
		}

		void write(const TItem& item)
		{
			m_items.push(item);
		}

	private:
		void transmissionLoop(const std::string& tag)
		{
			int index = 0;
			while (true)
			{
				const std::string& pipePath = utils::preparePath(EMedium::NamedPipe, m_accountInfo, tag, index);
				++index;
				KNamedPipeServer dataPipe(pipePath);
				if (dataPipe.create())
				{
					while (dataPipe.isValid())
					{
						const TItem& item = m_items.pop();
						dataPipe.write(item);
					}
				}
				else
				{
					const int SleepDuration = 1000;
					utils::sleep(SleepDuration);
				}
			}
		}

	private:
		const SAccountInfo m_accountInfo;
		cpp::threadsafe_queue<TItem> m_items;

};

} // namespace fx

#endif
