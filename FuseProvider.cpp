#include "FuseProvider.h"

#ifndef _WIN64
#include <fuse_lowlevel.h>
#endif
#include <signal.h>

#include <cstring>
#include <map>

#ifdef _WIN64
#define S_IFDIR 0040000
#define S_IFREG 0100000
#endif

// to allow the callbacks to talk to the manager too

struct inf_loaded_file {
	void* data;
	uint64_t size;
	uint32_t refCount;
}inf_loaded_file;

ModuleManager* cModMan;

std::map<std::string,struct inf_loaded_file> loadedFiles;
Logger* fuseLogger;

FuseProvider::FuseProvider(Logger* logger, ModuleManager* modMan){
	this->logger = logger;
	mounted = false;
	this->modMan = modMan;
	fs = nullptr;
	session = nullptr;
}

void FuseProvider::setup(std::string path, std::string options){
	mountPath = path;
	this->options = options;
}

static void* inf_init(struct fuse_conn_info* conn, struct fuse_config* cfg){
	cfg->kernel_cache = 1;
	return NULL;
}

#ifdef _WIN64
static int inf_getattr(const char* path, struct fuse_stat* stbuf, struct fuse_file_info* fi){
#else
static int inf_getattr(const char* path, struct stat* stbuf, struct fuse_file_info* fi){
#endif
	cModMan->mutex.lock_shared();
	ModuleNode* node = cModMan->getNode(std::string(path));
	if(node == nullptr){
		cModMan->mutex.unlock_shared();
		return -ENOENT;
	}
	if(node->type == NODE_TYPE_DIRECTORY){
		stbuf->st_mode = S_IFDIR | 0555;
		stbuf->st_nlink = 2;
		stbuf->st_size = 0;	// directories get no size
	}
	if(node->type == NODE_TYPE_FILE){
		stbuf->st_mode = S_IFREG | 0555;
		stbuf->st_nlink = 1;
		stbuf->st_size = cModMan->getSizes(node).first;
	}
	/*if(strcmp(path,"/")){
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		return -ENOENT;
	}*/
	cModMan->mutex.unlock_shared();
	return 0;
}

#ifdef _WIN64
static int inf_readdir(const char* path, void* buf, fuse_fill_dir_t filler, fuse_off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags){
#else
static int inf_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags){
#endif
	cModMan->mutex.lock_shared();
	ModuleNode* node = cModMan->getNode(std::string(path));
	if(node->type != NODE_TYPE_DIRECTORY){
		return -ENOENT;
	}
	filler(buf,".",NULL,0,(fuse_fill_dir_flags)0);
	filler(buf,"..",NULL,0,(fuse_fill_dir_flags)0);

	for(auto it = node->children.begin(); it != node->children.end(); it++){
		filler(buf,it->first.c_str(),NULL,0,(fuse_fill_dir_flags)0);
	}

	cModMan->mutex.unlock_shared();
	return 0;
}

static int inf_open(const char *path, struct fuse_file_info *fi){
	printf("File %s opened\n", path);
	if(loadedFiles.count(std::string(path)) == 0){
		// file hasn't been loaded yet
		ModuleNode* node = cModMan->getNode(std::string(path));
		if(node == nullptr){
			// file doesn't exist
			return -ENOENT;
		}
		if(node->item == nullptr || node->type != NODE_TYPE_FILE){
			// no item, something is probably broken, but just act like the file doesn't exist for now
			return -ENOENT;
		}
		void* data = node->item->extractData();
		uint64_t size = cModMan->getSizes(node).first;
		loadedFiles.insert({std::string(path),{.data = data, .size = size, .refCount = 0}});
	}
	// the file should definitely be loaded now, increment refcount
	loadedFiles[std::string(path)].refCount++;
	return 0;
}

#ifdef _WIN64
static int inf_read(const char *path, char *buf, size_t size, fuse_off_t offset, struct fuse_file_info *fi){
#else
static int inf_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
#endif
	if(loadedFiles.count(std::string(path)) == 0){
		// file not open?
		return -ENOENT;
	}
	struct inf_loaded_file* lf = &loadedFiles[std::string(path)];
	if(offset > lf->size || offset < 0){
		// beyond the end of the file
		return -EINVAL;
	}

	if(offset + size > lf->size){
		// read would go beyond eof, but is partially still inside the file
		size = lf->size - offset;
	}
	memcpy(buf,lf->data + offset,size);
	return size;
}

static int inf_release(const char* path, struct fuse_file_info *fi){
	struct inf_loaded_file* lf = &loadedFiles[std::string(path)];
	lf->refCount--;
	if(lf->refCount == 0){
		// there are no more references, so we can unload this file and free the memory
		free(lf->data);
		loadedFiles.erase(std::string(path));
	}
	return 0;
}

static void inf_destroy(void* privateData){
	for(auto it = loadedFiles.begin(); it != loadedFiles.end(); it++){
		// there's no need to delete the entries from the map here because that can just be done later after all buffers have been freed
		free(it->second.data);
	}
	loadedFiles.clear();
}

static const struct fuse_operations inf_ops = {
		.getattr = inf_getattr,
		.open = inf_open,
		.read = inf_read,
		//.statfs = inf_statfs,
		.release = inf_release,
		.readdir = inf_readdir,
		.init = inf_init,
		.destroy = inf_destroy
};





int FuseProvider::mount(){
	// just make it available to the callbacks too
	cModMan = modMan;
	fuseLogger = logger;

	if(mounted){
		logger->log(LOG_LEVEL_ERROR,"Already mounted.");
		return -1;
	}

	int r;
	char* argv[2];
	argv[0] = "owo";
	//argv[0] = (char*)mountPath.c_str();
	//argv[1] = (char*) options.c_str();
	int argc = options.length() == 0 ? 0 : 2;
	struct fuse_args args = FUSE_ARGS_INIT(1,argv);
	if(fuse_opt_parse(&args, NULL, NULL, NULL)){
		return -1;
	}
	if(options.length() != 0){
		r = fuse_opt_add_arg(&args, "-o");
		r = fuse_opt_add_arg(&args, options.c_str());
	}
	//r = fuse_opt_add_arg(&args, "-f");
	//r = fuse_opt_add_arg(&args, mountPath.c_str());
	// setup is done, start fuse in a separate thread
	if(fuseThread.joinable()){
		// this can happen for a few reasons, but shouldn't be a big deal
		//logger->log(LOG_LEVEL_WARNING,"Looks like fuse didn't shut down cleanly. Maybe you had some wrong options, or the folder was unmounted externally. Joining Thread.\n");
		fuseThread.join();
	}
	logger->log(LOG_LEVEL_INFO, "starting fuse\n");
	fuseThread = std::thread([this,args]{
		//int r = fuse_main(args.argc,args.argv,&inf_ops,modMan);

		struct fuse_args args2 = FUSE_ARGS_INIT(args.argc,args.argv);
		if(fuse_opt_parse(&args2, NULL, NULL, NULL)){
			return -1;
		}
		/*struct fuse_opt opts;
		if(fuse_opt_parse((fuse_args*)&args, NULL, NULL, NULL)){
			return 1;
		}*/

#ifndef _WIN64
		if(realpath(mountPath.c_str(),NULL) == 0){
			logger->log(LOG_LEVEL_ERROR, "Invalid mounting path!\n");
			return -1;
		}
#endif
		//struct fuse_fs* fs = fuse_fs_new(&inf_ops, sizeof(inf_ops), modMan);
		fs = fuse_new(&args2, &inf_ops, sizeof(inf_ops), modMan);
		if(fs == NULL){
			return -1;
		}
		if(fuse_mount(fs, mountPath.c_str())){
			fuse_destroy(fs);
			return -2;
		}

		session = fuse_get_session(fs);
		if(fuse_set_signal_handlers(session)){
			fuse_unmount(fs);
			fuse_destroy(fs);
			return -3;
		}
		mounted = true;
		logger->log(LOG_LEVEL_INFO,"Started fuse with mountpoint %s\n",mountPath.c_str());
		int r = fuse_loop(fs);

		fuse_remove_signal_handlers(session);

		fuse_unmount(fs);
		fuse_destroy(fs);
		/*struct fuse_args args2 = FUSE_ARGS_INIT(args.argc,args.argv);

		struct fuse_session* se;

		struct fuse_loop_config config;

		if(fuse_parse_cmdline(&args2, &opts)){
			return 1;
		}
		se = fuse_session_new(&args2, &inf_ops, sizeof(inf_ops),modMan);*/
		fuse_opt_free_args((fuse_args*)&args2);
		logger->log(LOG_LEVEL_INFO, "stopped fuse\n");
		mounted = false;
		return 0;
	});
	//fuse_opt_free_args(&args);
	return r;
}

int FuseProvider::unmount(){
	//struct fuse* f  =
	//fuse_unmount(f);
	//fuse_exit(f);
	//pthread_kill(fuseThread.get_id(), 2);


#ifdef _WIN64
	fuse_exit(fs);
#else
	fuse_session_exit(fuse_get_session(fs));
	fuse_unmount(fs);
#endif
	logger->log(LOG_LEVEL_INFO, "stopping fuse\n");
	if(fuseThread.joinable())
		fuseThread.join();
	//fuse_remove_signal_handlers(session);

	//fuse_destroy(fs);

	return 0;
}
