#pragma once

#include <stdint.h>
#include <map>

#include "libInfinite/module/ModuleItem.h"

// node types
#define NODE_TYPE_FILE 0
#define NODE_TYPE_DIRECTORY 1

class ModuleNode{
public:
	std::map<std::string,ModuleNode*> children;
	ModuleNode* parent;
	std::string path;
	std::string name;
	uint32_t type;
	ModuleItem* item;
};
