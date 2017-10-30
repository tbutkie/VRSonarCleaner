#pragma once

#include "BehaviorBase.h"
#include <map>
#include <fstream>
#include <filesystem>

class DataLogger
{
public:

	static DataLogger& getInstance()
	{
		static DataLogger s_instance;
		return s_instance;
	}

	static std::string getTimeString();

	void setLogDirectory(std::string dir);
	bool openLog(std::string logName, bool appendTimestampToLogname = true);
	void closeLog();

	void setID(int id);

	void logMessage(std::string message);

private:
	DataLogger();
	~DataLogger();

	std::ofstream m_fsLog;

	int m_nID;

	std::experimental::filesystem::v1::path m_LogDirectory;

public:
	// DELETE THE FOLLOWING FUNCTIONS TO AVOID NON-SINGLETON USE
	DataLogger(DataLogger const&) = delete;
	void operator=(DataLogger const&) = delete;
};

