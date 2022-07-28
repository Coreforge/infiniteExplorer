#pragma once

#include "libInfinite/logger/logger.h"
#include "LogManager.h"

class ManagedLogger : public Logger{
public:
	ManagedLogger(const char* prefix, LogManager* manager);

	void log(int level, const char* fmt, ...);

private:
	LogManager* manager;
	const char* prefix;
	int prefix_len;
};
