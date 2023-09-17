#pragma once

#include "../libInfinite/tags/handles/baseClasses/GenericHandle.h"

#include <vector>

class AssimpHandle : public GenericHandle{
public:
	std::vector<int> indicies;
	bool initialized = false;
};
