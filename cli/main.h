#pragma once

#include <string>
#include <libInfinite/module/ModuleManager.h>
#include <libInfinite/tags/handles/sbspHandle.h>

void loadPathRecursive(std::string path, Logger* logger, ModuleManager& modMan);
int exportBSP(sbspHandle* handle, std::string out, Logger* logger);
