#include "DataLogger.h"

#include <iostream>
#include <string>

using namespace std::experimental::filesystem::v1;

void DataLogger::setLogDirectory(std::string dir)
{
	m_LogDirectory = current_path().append(dir);
}

DataLogger::LogHandle DataLogger::openLog(std::string logName, bool appendTimestampToLogname)
{
	std::string filename = appendTimestampToLogname ? logName + getTimeString() : logName;
	std::ofstream ofs;
	ofs.open(std::string(m_LogDirectory.string() + filename));
	//LogHandle thisHandle = m_sCurrentHandle++;
	//m_mapLogs.emplace(std::make_pair(thisHandle, ofs));
	return 0;
}

void DataLogger::closeLog(LogHandle handle)
{
	m_mapLogs[handle].close();
	m_mapLogs.erase(handle);
}

void DataLogger::logMessage(LogHandle handle, std::string message)
{
	m_mapLogs[handle] << message << std::endl;
}

DataLogger::DataLogger()
{
}


DataLogger::~DataLogger()
{
}



std::string int2String(int i, unsigned int pad_to_magnitude)
{
	if (pad_to_magnitude < 1)
		return std::to_string(i);

	std::string ret;

	int mag = i == 0 ? 0 : (int)log10(i);

	for (int j = pad_to_magnitude - mag; j > 0; --j)
		ret += std::to_string(0);

	ret += std::to_string(i);

	return ret;
}


std::string DataLogger::getTimeString()
{
	time_t t = time(0);   // get time now
	struct tm *now = localtime(&t);

	return 	  /*** DATE ***/
		int2String(now->tm_year + 1900, 3) + // year
		"-" + int2String(now->tm_mon + 1, 1) +     // month
		"-" + int2String(now->tm_mday, 1) +        // day
													/*** TIME ***/
		"_" + int2String(now->tm_hour, 1) +        // hour
		"-" + int2String(now->tm_min, 1) +         // minute
		"-" + int2String(now->tm_sec, 1);          // second
}