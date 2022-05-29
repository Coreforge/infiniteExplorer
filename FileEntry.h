#pragma once
#include <string>

// file types, used for icons
#define FILE_TYPE_FILE 0
#define FILE_TYPE_DIRECTORY 1


class FileEntry{

public:
	// onClick functions. If both aren't nullptr, onClickID has higher priority. Of none are set, nothing happens
	// onClick with an ID passed to it to identify the associated file
	void (*onClick)(void*,void*) = nullptr;

	// this pointer gets passed to the onClick functions
	void* data;

	int ID;
	std::string path;
	char* name;


	void* nodeRef;
	// determines the icon used
	int type;

	// internal for FileList
	bool selected;
};
