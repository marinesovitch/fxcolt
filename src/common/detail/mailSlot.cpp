// author: Darek Slusarczyk alias marines marinesovitch 2012-2013, 2022
#include "ph.h"
#include "mailSlot.h"
#include "fileUtils.h"
#include "cpp/streams.h"

namespace fx
{

KMailSlotReceiver::KMailSlotReceiver(const std::string& slotName)
	: m_handle(INVALID_HANDLE_VALUE)
	, m_buffer(static_cast< char* >(::GlobalAlloc(GPTR, consts::MaxMailSlotMsgLen)))
{
	m_handle = ::CreateMailslot(slotName.c_str(), 0, MAILSLOT_WAIT_FOREVER, nullptr);
	if(m_handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("cannot alloc mail slot");
}

KMailSlotReceiver::~KMailSlotReceiver()
{
	if(m_handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_handle);
	}
	::GlobalFree(m_buffer);
}

std::size_t KMailSlotReceiver::read(const std::size_t dataSize)
{
	std::size_t msgSize = 0;

	unsigned long maxMsgSize;
	if (::GetMailslotInfo(
		m_handle,
		nullptr,
		&maxMsgSize,
		nullptr,
		nullptr))
	{
		if (maxMsgSize == MAILSLOT_NO_MESSAGE)
		{
			maxMsgSize = consts::MaxMailSlotMsgLen;
		}

		unsigned long rawMsgSize;
		if (::ReadFile(m_handle, m_buffer, maxMsgSize, &rawMsgSize, 0))
		{
			if ((dataSize != 0) && (dataSize != rawMsgSize))
			{
				throw std::runtime_error("incorrect length of mailslot message");
			}

			msgSize = static_cast<std::size_t>(rawMsgSize);
		}
	}

	return msgSize;
}

bool KMailSlotReceiver::read(std::string* msg)
{
	const std::size_t msgSize = read();
	const bool result = (0 < msgSize);
	if (result)
	{
		msg->assign(m_buffer, m_buffer + msgSize);
	}
	return result;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

KMailSlotTransmitter::KMailSlotTransmitter(const std::string& path)
	: m_handle(INVALID_HANDLE_VALUE)
	, m_path(path)
{
	cpp::cout << "KMailSlotTransmitter::KMailSlotTransmitter " << path << std::endl;
}

KMailSlotTransmitter::~KMailSlotTransmitter()
{
	close();
}

bool KMailSlotTransmitter::write(const char* buffer, const std::size_t dataSize)
{
	bool result = false;
	//cpp::cout << "KMailSlotTransmitter::write " << m_path << std::endl;
	if (m_handle == INVALID_HANDLE_VALUE)
	{
		cpp::cout << "KMailSlotTransmitter::write (m_handle == INVALID_HANDLE_VALUE)" << std::endl;
		if (open())
		{
			cpp::cout << "KMailSlotTransmitter::write before send" << std::endl;
			result = send(buffer, dataSize);
			cpp::cout << "KMailSlotTransmitter::write after send " << result << std::endl;
		}
	}
	else
	{
		result = send(buffer, dataSize);
	}
	//cpp::cout << "KMailSlotTransmitter::write " << m_path << std::endl;
	return result;
}

bool KMailSlotTransmitter::write(const std::string& msg)
{
	const bool result = write(msg.c_str(), msg.length());
	return result;
}

bool KMailSlotTransmitter::open()
{
	assert(m_handle == INVALID_HANDLE_VALUE);
	bool result = false;
	cpp::cout << "KMailSlotTransmitter::open " << m_path << std::endl;
	if (utils::pathExists(m_path))
	{
		cpp::cout << "KMailSlotTransmitter::open fs::pathExists true" << std::endl;
		m_handle = ::CreateFile(
			m_path.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);
		if (m_handle != INVALID_HANDLE_VALUE)
		{
			result = true;
		}
	}
	cpp::cout << "KMailSlotTransmitter::open result " << result << ' ' << ::GetLastError() << std::endl;
	return result;
}

void KMailSlotTransmitter::close()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

bool KMailSlotTransmitter::send(const char* buffer, const std::size_t dataSize)
{
	bool result = false;
	unsigned long writtenBytes;
	if (::WriteFile(
		m_handle,
		buffer,
		dataSize,
		&writtenBytes,
		nullptr))
	{
		result = true;
	}
	else
	{
		close();
	}
	return result;
}

} // namespace fx
