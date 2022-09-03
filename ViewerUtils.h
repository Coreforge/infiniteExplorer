#pragma once

#include <string>
#include <cstring>

static std::string uint32ToHexString(uint32_t value){
	int len = snprintf(NULL,0,"0x%x",value);
	char* c = (char*)malloc(len+1);
	snprintf(c,len+1,"0x%x",value);
	std::string ret(c);
	free(c);
	return ret;
}

static std::string uint16ToHexString(uint16_t value){
	int len = snprintf(NULL,0,"0x%x",value);
	char* c = (char*)malloc(len+1);
	snprintf(c,len+1,"0x%x",value);
	std::string ret(c);
	free(c);
	return ret;
}
