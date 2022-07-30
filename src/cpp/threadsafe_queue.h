// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#ifndef INC_CPP_THREADSAFE_QUEUE_H
#define INC_CPP_THREADSAFE_QUEUE_H

#include "types.h"

namespace cpp
{

template<typename TItem>
class threadsafe_queue
{
	public:
		threadsafe_queue()
		{
		}

		void clear()
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			m_items.clear();
		}

		void push(TItem item)
		{
			std::lock_guard<std::mutex> lck(m_mtx);
			m_items.push_back(item);
			m_onItemAdded.notify_one();
		}

		TItem pop()
		{
			std::unique_lock<std::mutex> lck(m_mtx);
			m_onItemAdded.wait(lck, [this]{return !m_items.empty();});
			TItem result = m_items.front();
			m_items.pop_front();
			return result;
		}

		TItem try_pop()
		{
			TItem result;
			if (!m_items.empty())
			{
				std::unique_lock<std::mutex> lck(m_mtx);
				if (!m_items.empty())
				{
					m_onItemAdded.wait(lck, [this]{return !m_items.empty();});
					result = m_items.front();
					m_items.pop_front();
				}
			}
			return result;
		}

	private:
		mutable std::mutex m_mtx;
		std::condition_variable m_onItemAdded;
		std::list<TItem> m_items;

};

} // namespace cpp

#endif
