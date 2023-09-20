#pragma once

#include <string>
#include <libInfinite/module/ModuleManager.h>
#include <libInfinite/tags/handles/sbspHandle.h>
#include <libInfinite/StringIDLUT.h>

void loadPathRecursive(std::string path, Logger* logger, ModuleManager& modMan);
int exportBSP(sbspHandle* handle, std::string out, Logger* logger, StringIDLUT& lut, ModuleManager* modman, std::string texout, int mipmap);
