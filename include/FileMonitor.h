#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <string>
#include <fstream>
#include <memory>
#include <boost/crc.hpp>
#include <boost/timer.hpp>

typedef std::shared_ptr<class FileMonitor> FileMonitorRef;
#define FILEMONITOR_CHECK_INTERVAL 3

class FileMonitor
{
public:
    static FileMonitorRef create(const boost::filesystem::path &fileName)
	{
		return FileMonitorRef(new FileMonitor(fileName));
	}

    bool hasChanged();

private:
	FileMonitor(const boost::filesystem::path &fileName);
	boost::filesystem::path m_fileName;
    int m_checksum;
    boost::timer m_time;
    const int getChecksum();
};

FileMonitor::FileMonitor(const boost::filesystem::path &fileName)
{
	m_fileName = fileName;
	m_checksum = getChecksum();
	m_time.restart();
}

bool FileMonitor::hasChanged()
{
	double elapsed = m_time.elapsed();

	if(elapsed > FILEMONITOR_CHECK_INTERVAL)
	{
		m_time.restart();
		int currentChecksum = getChecksum();

		if(m_checksum != currentChecksum)
		{
			m_checksum = currentChecksum;
			return true;
		}
	}

	return false;
}

const int FileMonitor::getChecksum()
{
	boost::crc_32_type result;
	std::streamsize const buffer_size = 1024;

	std::ifstream ifs(m_fileName.string(), std::ios_base::binary);

	if(ifs)
	{
		do
		{
			char buffer[buffer_size];
			ifs.read(buffer, buffer_size);
			size_t count = static_cast<size_t>(ifs.gcount());
			result.process_bytes(buffer, count);
		}
		while(ifs);
	}

	return result.checksum();
}

#endif

