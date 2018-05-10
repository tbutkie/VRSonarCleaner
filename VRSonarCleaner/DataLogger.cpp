#include "DataLogger.h"

#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std::experimental::filesystem::v1;

void DataLogger::setLogDirectory(std::string dir)
{
	m_LogDirectory = current_path().append(dir);
}

bool DataLogger::openLog(std::string logName, bool appendTimestampToLogname)
{
	std::string filename = appendTimestampToLogname ? logName + "_" + getTimeString() : logName;
	m_fsLog.open(std::string(m_LogDirectory.string() + filename));
	return m_fsLog.is_open();
}

void DataLogger::closeLog()
{
	if (m_fsLog.is_open())
		m_fsLog.close();

	stop();
}

void DataLogger::start()
{
	m_bLogging = m_fsLog.is_open();

	if (m_bLogging)
		m_tpLogStart = std::chrono::high_resolution_clock::now();
}

void DataLogger::stop()
{
	m_bLogging = false;
}

bool DataLogger::logging()
{
	return m_bLogging;
}

void DataLogger::setID(std::string id)
{
	m_strID = id;
}

void DataLogger::logMessage(std::string message)
{
	if (m_bLogging)
		m_fsLog << m_strID << '\t' << message << "\n";
}

std::string DataLogger::getTimeSinceLogStartString()
{
	if (!m_bLogging)
		return "00:00:00.000";

	std::chrono::duration<double> elapsedTime(std::chrono::high_resolution_clock::now() - m_tpLogStart);

	int hours, minutes, seconds, milliseconds;

	hours = std::chrono::duration_cast<std::chrono::hours>(elapsedTime).count() % 24;
	minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsedTime).count() % 60;
	seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count() % 60;
	milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count() % 1000;

	std::stringstream ss;
	ss << std::setw(2) << std::setfill('0') << hours;
	ss << ":";
	ss << std::setw(2) << std::setfill('0') << minutes;
	ss << ":";
	ss << std::setw(2) << std::setfill('0') << seconds;
	ss << ".";
	ss << std::setw(3) << std::setfill('0') << milliseconds;

	return ss.str();
}

DataLogger::DataLogger() : m_bLogging(false)
{
}


DataLogger::~DataLogger()
{
	if (m_fsLog.is_open())
		m_fsLog.close();
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