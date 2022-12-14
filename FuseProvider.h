#pragma once
#include <string>

#include <thread>

#include "libInfinite/logger/logger.h"
#include "libInfinite/module/ModuleManager.h"

#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>

class FuseProvider{

public:
	FuseProvider(Logger* logger, ModuleManager* modMan);

	void setup(std::string path, std::string options);
	int mount();
	int unmount();

	bool mounted;

private:
	Logger* logger;
	std::string mountPath;
	std::string options;
	ModuleManager* modMan;

	std::thread fuseThread;

	struct fuse* fs;
	struct fuse_session* session;
};
