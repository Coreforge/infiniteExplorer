#pragma once

#include "../libInfinite/tags/handles/baseClasses/GenericHandle.h"

class AssimpHandle : public GenericHandle{
public:
	int index;
	bool initialized = false;
};
