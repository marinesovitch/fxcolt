#ifndef INC_COMMON_NAMEDPIPE_H
#define INC_COMMON_NAMEDPIPE_H

namespace fx
{

class KNamedPipeBase
{
	protected:
		KNamedPipeBase(const std::string& path, const std::size_t bufferSize);

	public:
		virtual ~KNamedPipeBase();

		bool isValid() const;

	protected:
		virtual void close();

	public:
		bool read(std::string* msg);

		template<typename T>
		bool read(T* data)
		{
			const std::size_t dataSize = sizeof(T);
			assert(dataSize <= consts::NamedPipeBufferSize);
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


	public:
		bool write(const std::string& msg);
		bool write(const char* buffer, const std::size_t dataSize);

		template<typename T>
		bool write(const T& data)
		{
			const std::size_t dataSize = sizeof(T);
			assert(dataSize);
			const char* buffer = reinterpret_cast<const char*>(&data);
			const bool result = write(buffer, dataSize);
			return result;
		}

	protected:
		const std::string m_path;
		HANDLE m_handle;
		char* m_buffer;
		const std::size_t m_bufferSize;

};

// ---------------------------------------------------------------------------

class KNamedPipeServer : public KNamedPipeBase
{
	public:
		KNamedPipeServer(const std::string& path);

	public:
		bool create();

	protected:
		virtual void close();

};

// ---------------------------------------------------------------------------

class KNamedPipeClient : public KNamedPipeBase
{
	public:
		KNamedPipeClient(const std::string& path);

	public:
		bool open();

};

} // namespace fx

#endif
