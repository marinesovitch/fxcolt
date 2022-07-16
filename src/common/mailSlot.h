#ifndef INC_COMMON_MAILSLOT_H
#define INC_COMMON_MAILSLOT_H

#include "consts.h"

namespace fx
{

class KMailSlotReceiver
{
	public:
		KMailSlotReceiver(const std::string& path);
		~KMailSlotReceiver();

	public:
		bool read(std::string* msg);

		template<typename T>
		bool read(T* data)
		{
			const std::size_t dataSize = sizeof(T);
			assert(dataSize <= consts::MaxMailSlotMsgLen);
			char* buffer = reinterpret_cast<char*>(data);
			const std::size_t msgSize = read(dataSize);
			const bool result = (dataSize == msgSize);
			if (result)
			{
				memcpy(buffer, m_buffer, dataSize);
			}
			return result;
		}

	private:
		std::size_t read(const std::size_t dataSize = 0);

	private:
		HANDLE m_handle;
		char* m_buffer;

};

// ---------------------------------------------------------------------------

class KMailSlotTransmitter
{
	public:
		KMailSlotTransmitter(const std::string& path);
		~KMailSlotTransmitter();

	public:
		bool write(const char* buffer, const std::size_t dataSize);
		bool write(const std::string& msg);

		template<typename T>
		bool write(const T& data)
		{
			const std::size_t dataSize = sizeof(T);
			assert(dataSize);
			const char* buffer = reinterpret_cast<const char*>(&data);
			const bool result = write(buffer, dataSize);
			return result;
		}

	private:
		bool open();
		void close();
		bool send(const char* buffer, const std::size_t dataSize);

	private:
		const std::string m_path;
		HANDLE m_handle;

};

} // namespace fx

#endif
