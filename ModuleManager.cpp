#include <cstdio>
#include <gtkmm.h>
#include "ModuleManager.h"
#include <algorithm>

#include "libInfinite/logger/logger.h"
#include "libInfinite/Item.h"

#ifdef _WIN64
#include <windows.h>
#include <fileapi.h>
//#include <direct.h>

//#warning WIN64 detected
#endif

//#define EXPORT_DRYRUN	// don't write any files, just check if all files can be extracted

// excludes files from the file list if their data isn't actually present in the modules
#define IGNORE_BROKEN_FILES 1

inline std::string cleanSearchString(std::string str){
	str.erase(std::remove(str.begin(), str.end(), ' '),str.end());	// remove " "
	str.erase(std::remove(str.begin(), str.end(), '_'),str.end());	// remove "_"
	str.erase(std::remove(str.begin(), str.end(), '.'),str.end());	// remove "."

	for(auto& c : str){
		c = std::tolower(c);
	}
	return str;
}

void ModuleManager::openModuleDialog(){
	//printf("open\n");
	Gtk::FileChooserDialog* fileChooser = new Gtk::FileChooserDialog("Load Module",Gtk::FILE_CHOOSER_ACTION_OPEN);
	fileChooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fileChooser->add_button("_Open", Gtk::RESPONSE_OK);
	fileChooser->set_select_multiple(true);
	// try to open the default location for steam
#ifdef _WIN64
	// for windows
	// this is just hard coded for now, but finding the steam library shouldn't be too hard either
	fileChooser->set_current_folder("C:/Program Files (x86)/Steam/steamapps/common/Halo Infinite/deploy");
#else
	// linux
	fileChooser->set_current_folder(Glib::getenv("HOME") + "/.steam/steam/steamapps/common/Halo Infinite/deploy");
#endif
	int response = fileChooser->run();
	fileChooser->close();
	if(response != Gtk::RESPONSE_OK){
		//printf("cancel\n");
		return;
	}
	std::vector<std::string> files = fileChooser->get_filenames();
	for(int i = 0; i < files.size(); i++){
		//printf("Opening file: %s\n",files[i].c_str());
		Module* mod = new Module();
		mod->logger = logger;
		mod->loadModule(files[i].c_str());
		logger->log(LOG_LEVEL_INFO,"Loaded module\n");
		modules.emplace_back(mod);
	}

	// modules have been loaded, but the tree needs to be built so that it can be displayed in the file list
	buildNodeTree();
	//currentNode = rootNode;
	showNode(rootNode);	// display the root node in the list
}

void ModuleManager::openPathDialog(){
	Gtk::FileChooserDialog* fileChooser = new Gtk::FileChooserDialog("Load Modules from",Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	fileChooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fileChooser->add_button("_Open", Gtk::RESPONSE_OK);

	// try to open the default location for steam
#ifdef _WIN64
	// for windows
	// this is just hard coded for now, but finding the steam library shouldn't be too hard either
	fileChooser->set_current_folder("C:/Program Files (x86)/Steam/steamapps/common/Halo Infinite");
#else
	// linux
	fileChooser->set_current_folder(Glib::getenv("HOME") + "/.steam/steam/steamapps/common/Halo Infinite");
#endif

	int response = fileChooser->run();
	fileChooser->close();
	if(response != Gtk::RESPONSE_OK){
		//printf("cancel\n");
		return;
	}

	loadPathRecursive(fileChooser->get_filename());

	// all modules are loaded now, build the tree and show the root node
	buildNodeTree();
	showNode(rootNode);
}

void ModuleManager::loadPathRecursive(std::string path){

	// this has to be done differently on POSIX-compliant systems and /windows/ ...sigh

#ifdef _WIN64
	// windows version
	WIN32_FIND_DATA find;
	HANDLE dirHandle;

	std::string searchPath = path + "/*";

	dirHandle = FindFirstFile(searchPath.c_str(), &find);

	if (dirHandle == INVALID_HANDLE_VALUE) {
		logger->log(LOG_LEVEL_WARNING,"Could not open %s\n", path.c_str());
	}

	do {
		std::string cPath = path + "\\" + find.cFileName;
		//printf("Found %s\n",cPath.c_str());
		if (!(strncmp(find.cFileName, ".", 2) && strncmp(find.cFileName, "..", 3))) {
			// . or .., skip those
			continue;
		}

		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// is a directory
			//printf("This is a directory\n");
			loadPathRecursive(cPath);

			// don't do the other stuff, which is only for files
			continue;
		}

		// this is only for files

		std::string name(find.cFileName);

		// all of this can just be copied from the posix version
		if (name.size() < 7) {
			// name is too short
			continue;
		}
		if (name.substr(name.size() - 7, 7) == ".module") {
			// extension should match
			//printf("Trying %s\n", cPath.c_str());
			Module* mod = new Module();
			mod->logger = logger;
			if (mod->loadModule(cPath.c_str())) {
				// the module couldn't be loaded
				mod->~Module();
				logger->log(LOG_LEVEL_ERROR,"Failed to load module %s, skipping!\n", cPath.c_str());
			}
			else {
				modules.emplace_back(mod);
			}
		}


	} while (FindNextFile(dirHandle,&find));
#else
	// the posix version
	DIR* dir;
	dir = opendir(path.c_str());	// open the current directory
	if(!dir){
		// something went wrong opening the directory
		logger->log(LOG_LEVEL_WARNING,"Failed to open %s\n",path.c_str());
	}
	struct dirent* ent;
	ent = readdir(dir);
	struct stat buf;
	while(ent){
		// for every entry
		std::string cPath = path + "/" + ent->d_name;
		if(stat(cPath.c_str(),&buf)){
			// something went wrong with stat
			// report the error and just skip this entry
			logger->log(LOG_LEVEL_WARNING,"Failed to stat %s!\n",cPath.c_str());	// warning because it might not be an actual issue
			goto next;	// skip to the next entry
			continue;
		}
		if(S_ISDIR(buf.st_mode)){
			// another directory, check that one too
			if(strncmp(ent->d_name,".",2) && strncmp(ent->d_name,"..",3)){
				//printf("Looking in %s\n",cPath.c_str());
				loadPathRecursive(cPath);
			}

		}
		if(S_ISREG(buf.st_mode)){
			// regular file, but that doesn't mean it's a module
			// first check the file extension (.module, the last 7 characters of the name)
			std::string name(ent->d_name);
			if(name.size() < 7){
				// name is too short
				goto next;
			}
			if(name.substr(name.size() - 7, 7) == ".module"){
				// extension should match
				//printf("Trying %s\n",cPath.c_str());
				Module* mod = new Module();
				mod->logger = logger;
				if(mod->loadModule(cPath.c_str())){
					// the module couldn't be loaded
					mod->~Module();
					logger->log(LOG_LEVEL_ERROR,"Failed to load module %s, skipping!\n",cPath.c_str());
				} else {
					modules.emplace_back(mod);
				}
			}
		}
		next:
		ent = readdir(dir);	// next entry
	}
#endif
}

void ModuleManager::exportEntryDialog(){
	//printf("Export\n");
	if(fileList->selectedEntries.size() == 0){
		// nothing selected, export everything currently visible
		//logger->log(LOG_LEVEL_ERROR,"nothing selected\n");
		exportMultiple();
		return;
	}
	if(fileList->selectedEntries.size() == 1 && fileList->selectedEntries[0].first->type == FILE_TYPE_FILE){
		//printf("Export 1 file entry\n");
		// export a single file, so we can use a save dialog and let the user choose the name
		FileEntry* entry = fileList->selectedEntries[0].first;
		Gtk::FileChooserDialog* fileChooser = new Gtk::FileChooserDialog("Export",Gtk::FILE_CHOOSER_ACTION_SAVE);
		fileChooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
		fileChooser->add_button("_Save", Gtk::RESPONSE_OK);

		fileChooser->set_current_name(entry->name);
		int response = fileChooser->run();
		fileChooser->close();
		exportNode((ModuleNode*)fileList->selectedEntries[0].first->nodeRef, fileChooser->get_filename(),true);
		return;
	}
	// multiple entries selected, only let the user choose a directory to export to
	exportMultiple();
}

void ModuleManager::exportNode(ModuleNode* node, std::string path, bool fullPath){

	std::string newPath;
	if(!fullPath){
		// we didn't get passed the full path to this node, add the node name to the end of the path
		newPath = path + "/" + node->name;
	} else {
		// the full path got passed, just use that (only used when exporting a single file, as the user can specify a different file name in that case)
		newPath = path;
	}
	//printf("Exporting to %s\n",newPath.c_str());
	if(node->type == NODE_TYPE_FILE){
		// finally a file, now export it!
#ifndef EXPORT_DRYRUN
		FILE* out = fopen(newPath.c_str(),"wb");
#endif
		if(!node->item){
			logger->log(LOG_LEVEL_ERROR,"Node %s with type file doesn't have an associated item! Node will be skipped!\n",newPath.c_str());
			return;
		}
		void* data = node->item->extractData();
		if(data == nullptr){
			// libInfinite should already log whatever error occured, so there's no need to do it here
			return;
		}
#ifndef EXPORT_DRYRUN
		fwrite(data,1,node->item->decompressedSize,out);
		fclose(out);	// close the file again. It's done now
#endif
		free(data);	// free the buffer again, as it's not needed anymore
	} else if(node->type == NODE_TYPE_DIRECTORY){
		// still a directory, we need to go deeper!
		// create the directory, in case it doesn't exist yet. If it exists, nothing happens
#ifndef EXPORT_DRYRUN
#if defined( _WIN64) && !defined(__MINGW32__)
		int mk = _mkdir(newPath.c_str());
#elif defined(__MINGW32__)
		int mk = mkdir(newPath.c_str());
#else
		int mk = mkdir(newPath.c_str(),S_IRWXG | S_IRWXU | S_IRWXO);
#endif
		if(mk != 0){
			// some error occurred, but that doesn't have to be a problem. The directory might just already exist
			if(errno != EEXIST){
				// something else happened. Now it's probably a problem
				logger->log(LOG_LEVEL_ERROR,"Failed to create directory %s with error %d\n",newPath.c_str(),errno);	// here having issues with directories is a problem, as we need to be able to write to them.
				// when searching for modules it doesn't matter as much, as we just ignore any that might be in the directory
			}
		}
#endif
		for(auto const&[name,currentNode] : node->children){
			exportNode(currentNode, newPath);
		}
	} else {
		// this shouldn't happen. There should only be files or directories
		// if this ever gets reached, the node probably wasn't initialized correctly, memory got corrupted, or I forgot something
		logger->log(LOG_LEVEL_CRITICAL,"Error exporting node! Unknown node type!\n");
	}
}

void ModuleManager::exportMultiple(){
	// exporting multiple files, so just select a directory to export the files into
	Gtk::FileChooserDialog* fileChooser = new Gtk::FileChooserDialog("Export to",Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	fileChooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fileChooser->add_button("_Save", Gtk::RESPONSE_OK);

	int response = fileChooser->run();
	fileChooser->close();

	if(response == Gtk::RESPONSE_OK){
		// export
		std::string path = fileChooser->get_filename();
		if(fileList->selectedEntries.size() == 0){
			// export all shown entries
			// start with index 1 to skip the parent directory, as that would eventually result in infinite recursion, which would then crash due to a StackOverflow
			for(int i = 1; i < fileList->shownEntries.size(); i++){
				exportNode((ModuleNode*)fileList->shownEntries[i].first.nodeRef, path);
			}
		}else{
			// export all selected entries
			for(int i = 0; i < fileList->selectedEntries.size(); i++){
				exportNode((ModuleNode*)fileList->selectedEntries[i].first->nodeRef, path);
			}
		}

	}

}

void ModuleManager::buildNodeTree(){
	rootNode = new ModuleNode();
	rootNode->path = "";
	rootNode->name = "/";
	rootNode->parent = rootNode;	// the roots parent is the root
	rootNode->type = NODE_TYPE_DIRECTORY;
	for(int m = 0; m < modules.size(); m++){
		// each module
		//printf("Module %d\n",m);
		for(auto const&[key,value] : modules[m]->items){
			//printf("Item!\n");
			if(value->dataOffset > modules[m]->data_size && IGNORE_BROKEN_FILES){
				// the actual data for this file is missing, so it's not getting added to the tree, as there is no point in doing so
				continue;
			}
			std::stringstream stream(key);
			std::string part;
			ModuleNode* currentParent = rootNode;
			while(std::getline(stream,part,'/')){
				if(currentParent->children.count(part) == 0){
					//printf("adding node %s\n",part.c_str());
					ModuleNode* newNode = new ModuleNode();
					newNode->parent = currentParent;
					newNode->name = part;
					newNode->path = currentParent->path + "/" + part;
					newNode->type = NODE_TYPE_DIRECTORY;
					currentParent->children.insert({part,newNode});
				}
				currentParent =  currentParent->children[part];
				//printf("Part: %s\n",part.c_str());
			}
			currentParent->type = NODE_TYPE_FILE;
			currentParent->item = value;
		}
	}
}

void showNodeCallback(void* node,void* data){
	ModuleManager* manager = (ModuleManager*)data;
	//manager->currentNode = (ModuleNode*)node;	// this needs to be set here now because setting it in showNode causes issues with the search
	ModuleNode* nodeptr = (ModuleNode*)node;
	if(nodeptr->type == NODE_TYPE_FILE){
		// it's not a directory, so there's no point in displaying it. Try to load it instead
		//printf("Trying to load %s\n",nodeptr->path.c_str());

		uint8_t* itmData = nodeptr->item->extractData();
		if(itmData == nullptr){
			// libInfinite already logged the error, no need to do it again
			return;
		}
		Item* itm = new Item(itmData, nodeptr->item->decompressedSize, manager->logger, nodeptr->name, nodeptr->path);
		free(itmData);
		manager->fileViewerManager->addItem(itm);
		// we still don't want to display it in the file list
		return;
	}
	manager->showNode((ModuleNode*)node);
}

void search(void* manager, std::string query){
	// start by cleaning the query, then decide if we're actually going to search

	query = cleanSearchString(query);
	ModuleManager* man = (ModuleManager*)manager;

	if(man->currentNode == nullptr)return;	// we can't search if there is nothing loaded
	if(query == ""){
		//printf("displaying node\n");
		man->showNode(man->currentNode);
	}else{
		// we have to actually search
		//printf("searching\n");

		man->searchNodes(man->currentNode, query);
	}

}

void ModuleManager::searchNodes(ModuleNode* from, std::string query){
	// clean up the previous search, if there was one
	searchNode.children.clear();
	searchNode.parent = NULL;
	searchNode.item = NULL;

	// to search, just call the recursive search function, as there's nothing special from here on
	int index = 0;
	searchNodesRecursive(from, query, index);

	// the search either completed, or stopped after too many results. Just display the results now
	showNode(&searchNode, true);
}

int ModuleManager::searchNodesRecursive(ModuleNode* node, std::string query, int index){
	for(auto const&[key,cNode] : node->children){
		// clean up the name of the node, just like the query
		if(searchNode.children.size() > SEARCH_MAX_RESULTS){
			// we have too many results already, just return
			logger->log(LOG_LEVEL_DEBUG,"Reached search maximum!\n");
			return index;
		}
		std::string cName(key);
		cName = cleanSearchString(cName);

		// check if the query is in the name of the node
		if(cName.find(query, 0) != cName.npos){
			// the name contains the query, insert the node into the results list
			std::string newKey;

			newKey  = key + "_" + std::to_string(index);	// to allow for multiple results with the same name. It shouldn't affect sorting otherwise
			searchNode.children.insert({newKey, cNode});
			index++;
		}

		if(cNode->type == NODE_TYPE_DIRECTORY){
			// search this directory too
			index = searchNodesRecursive(cNode, query, index);
		}
	}
	return index;
}

void ModuleManager::setupCallbacks(){
	fileList->setSearchCallback(&search,this);
}

// first is uncompressed, second is compressed
std::pair<uint64_t,uint64_t> ModuleManager::getSizes(ModuleNode* node){
	uint64_t compressed = 0;
	uint64_t uncompressed = 0;
	if(node->type == NODE_TYPE_DIRECTORY){
		for(auto const&[key,cNode] : node->children){
			auto res = getSizes(cNode);
			compressed += res.second;
			uncompressed += res.first;
		}
	} else {
		if(node->item){
			compressed = node->item->compressedSize;
			uncompressed = node->item->decompressedSize;
		}
	}
	return {uncompressed, compressed};
}

void ModuleManager::showNode(ModuleNode* node, bool outOfTree){
	if(!outOfTree){
		// this node is part of the regular tree, so we should update the current Node
		currentNode = node;	// this is the current node now
	}

	// delete the old list, we don't want it anymore
	for(int i = 0;i < fileEntries.size();i++){
		fileEntries[i]->~FileEntry();
	}
	fileEntries.clear();

	int c = 0;	// just counting up the ID

	if(node->parent != NULL){
		// add the parent entry
		FileEntry* parentEntry = new FileEntry();
		parentEntry->name = "..";
		parentEntry->ID = 0;
		parentEntry->onClick = &showNodeCallback;
		parentEntry->data = this;
		parentEntry->type = FILE_TYPE_DIRECTORY;
		parentEntry->nodeRef = node->parent;
		fileEntries.emplace_back(parentEntry);
		c++;
	}

	//now build the new list
	// iterating through the keys like this seems to already automatically alphabetically sort them, which is nice
	for(auto const&[key,value] : node->children){
		FileEntry* entry = new FileEntry();
		entry->name = (char*)value->name.c_str();
		entry->path = value->path;
		entry->ID = c;
		entry->onClick = &showNodeCallback;
		entry->data = this;
		entry->nodeRef = value;
		switch(value->type){
		case NODE_TYPE_FILE:
			entry->type = FILE_TYPE_FILE;
			break;
		case NODE_TYPE_DIRECTORY:
			entry->type = FILE_TYPE_DIRECTORY;
			break;
		}
		fileEntries.emplace_back(entry);
		c++;
	}
	fileList->updateFiles(fileEntries);
	if(node == rootNode){
		fileList->currentPathLabel->set_text("/");
	}else{
		fileList->currentPathLabel->set_text(node->path.c_str());
	}
}
