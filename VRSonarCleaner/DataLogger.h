#pragma once

#include "BehaviorBase.h"
#include <map>
#include <fstream>
#include <filesystem>

class DataLogger
{
public:	
	typedef uint32_t LogHandle;

	static DataLogger& getInstance()
	{
		static DataLogger s_instance;
		return s_instance;
	}

	static std::string getTimeString();

	void setLogDirectory(std::string dir);
	LogHandle openLog(std::string logName, bool appendTimestampToLogname = true);
	void closeLog(LogHandle handle);

	void logMessage(LogHandle handle, std::string message);

private:
	DataLogger();
	~DataLogger();

	static LogHandle m_sCurrentHandle;

	std::map<LogHandle, std::ofstream> m_mapLogs;

	std::experimental::filesystem::v1::path m_LogDirectory;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	DataLogger(DataLogger const&) = delete;
	void operator=(DataLogger const&) = delete;
};

