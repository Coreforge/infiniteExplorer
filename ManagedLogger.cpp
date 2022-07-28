#include "ManagedLogger.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <stdint.h>

ManagedLogger::ManagedLogger(const char* prefix, LogManager* manager){
	this->prefix = prefix;
	this->manager = manager;
	prefix_len = strlen(prefix);
}

void ManagedLogger::log(int level, const char* fmt, ...){
	// first get how much space we need for the log message
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(NULL,0,fmt,args);
	va_end(args);
	// calculate the size we actually need for the whole log entry
	// the first byte tells the level of the message (+1 byte)
	// between the prefix and the message, a space gets added (+1 byte)
	// we also need a terminating null byte (+1 byte)
	unsigned int size = prefix_len + n + 3;
	char* msg = (char*)malloc(size);
	*msg = (uint8_t)level;	// the first byte tells the level of the message
	memcpy(msg + 1, prefix,prefix_len);	// copy the log prefix
	*(msg+prefix_len+1) = ' ';	// put in the space between the prefix and the message
	// write the actual message to the buffer
	va_start(args, fmt);
	vsnprintf(msg+prefix_len+2,n + 1,fmt,args);
	va_end(args);
	// submit the full entry to the log manager, which then copies it into its ring buffer
	manager->submitLog((const char*)msg, size);
	free(msg);	// we don't need this buffer anymore, as the data got copied
}
