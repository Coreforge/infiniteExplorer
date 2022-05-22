#include <cstdio>
#include <gtkmm.h>
#include "ModuleManager.h"

void ModuleManager::openModuleDialog(){
	printf("open\n");
	Gtk::FileChooserDialog* fileChooser = new Gtk::FileChooserDialog("Load Module",Gtk::FILE_CHOOSER_ACTION_OPEN);
	fileChooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fileChooser->add_button("_Open", Gtk::RESPONSE_OK);
	fileChooser->set_select_multiple(true);
	int response = fileChooser->run();
	fileChooser->close();
	if(response == Gtk::RESPONSE_CANCEL){
		printf("canceld\n");
		return;
	}
	std::vector<std::string> files = fileChooser->get_filenames();
	for(int i = 0; i < files.size(); i++){
		printf("Opening file: %s\n",files[i].c_str());
		Module* mod = new Module();
		mod->loadModule(files[i].c_str());
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
	int response = fileChooser->run();
	fileChooser->close();
	if(response == Gtk::RESPONSE_CANCEL){
		printf("canceld\n");
		return;
	}

	loadPathRecursive(fileChooser->get_filename());

	// all modules are loaded now, build the tree and show the root node
	buildNodeTree();
	showNode(rootNode);
}

void ModuleManager::loadPathRecursive(std::string path){
	DIR* dir;
	dir = opendir(path.c_str());	// open the current directory
	if(!dir){
		// something went wrong opening the directory
		printf("Failed to open %s\n",path.c_str());
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
			printf("Failed to stat %s!\n",cPath.c_str());
			goto next;	// skip to the next entry
			continue;
		}
		if(S_ISDIR(buf.st_mode)){
			// another directory, check that one too
			if(strncmp(ent->d_name,".",2) && strncmp(ent->d_name,"..",3)){
				printf("Looking in %s\n",cPath.c_str());
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
				printf("Trying %s\n",cPath.c_str());
				Module* mod = new Module();
				if(mod->loadModule(cPath.c_str())){
					// the module couldn't be loaded
					mod->~Module();
					printf("Failed to load module %s, skipping!\n",cPath.c_str());
				} else {
					modules.emplace_back(mod);
				}
			}
		}
		next:
		ent = readdir(dir);	// next entry
	}
}

void ModuleManager::exportEntryDialog(){
	printf("Export\n");
	if(fileList->selectedEntries.size() == 0){
		// nothing selected, export everything currently visible
		printf("nothing selected\n");
		exportMultiple();
		return;
	}
	if(fileList->selectedEntries.size() == 1 && fileList->selectedEntries[0].first->type == FILE_TYPE_FILE){
		printf("Export 1 file entry\n");
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

void ModuleManager::exportEntry(std::string path){

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
	printf("Exporting to %s\n",newPath.c_str());
	if(node->type == NODE_TYPE_FILE){
		// finally a file, now export it!
		FILE* out = fopen(newPath.c_str(),"w");
		if(!node->item){
			printf("Error: Node %s with type file doesn't have an associated item! Node will be skipped!\n",newPath.c_str());
		}
		void* data = node->item->extractData();
		fwrite(data,1,node->item->decompressedSize,out);
		free(data);	// free the buffer again, as it's not needed anymore
		fclose(out);	// close the file again. It's done now
	} else if(node->type == NODE_TYPE_DIRECTORY){
		// still a directory, we need to go deeper!
		// create the directory, in case it doesn't exist yet. If it exists, nothing happens
		int mk = mkdir(newPath.c_str(),S_IRWXG | S_IRWXU | S_IRWXO);
		if(mk != 0){
			// some error occurred, but that doesn't have to be a problem. The directory might just already exist
			if(errno != EEXIST){
				// something else happened. Now it's probably a problem
				printf("Failed to create directory %s with error %d\n",newPath.c_str(),errno);
			}
		}
		for(auto const&[name,currentNode] : node->children){
			exportNode(currentNode, newPath);
		}
	} else {
		// this shouldn't happen. There should only be files or directories
		// if this ever gets reached, the node probably wasn't initialized correctly, memory got corrupted, or I forgot something
		printf("Error exporting node! Unknown node type!\n");
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
	for(int m = 0; m < modules.size(); m++){
		// each module
		printf("Module %d\n",m);
		for(auto const&[key,value] : modules[m]->items){
			//printf("Item!\n");
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

void showNodeID(int ID,void* data){
	ModuleManager* manager = (ModuleManager*)data;
	manager->showNode(manager->fileEntryNodes[ID]);
}

void ModuleManager::showNode(ModuleNode* node){
	currentNode = node;	// this is the current node now

	// delete the old list, we don't want it anymore
	for(int i = 0;i < fileEntries.size();i++){
		fileEntries[i]->~FileEntry();
	}
	fileEntries.clear();
	fileEntryNodes.clear();

	// add the parent entry
	FileEntry* parentEntry = new FileEntry();
	parentEntry->name = "..";
	parentEntry->ID = 0;
	parentEntry->onClickID = &showNodeID;
	parentEntry->data = this;
	parentEntry->type = FILE_TYPE_DIRECTORY;
	fileEntries.emplace_back(parentEntry);
	fileEntryNodes.emplace_back(node->parent);

	//now build the new list
	int c = 1;	// just counting up the ID
	for(auto const&[key,value] : currentNode->children){
		FileEntry* entry = new FileEntry();
		entry->name = (char*)value->name.c_str();
		entry->ID = c;
		entry->onClickID = &showNodeID;
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
		fileEntryNodes.emplace_back(value);
		c++;
	}
	fileList->updateFiles(fileEntries);
	if(currentNode == rootNode){
		fileList->currentPathLabel->set_text("/");
	}else{
		fileList->currentPathLabel->set_text(currentNode->path.c_str());
	}
}
