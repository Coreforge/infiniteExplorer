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
		return;
	}
	// multiple entries selected, only let the user choose a directory to export to
	exportMultiple();
}

void ModuleManager::exportEntry(std::string path){

}

void ModuleManager::exportNode(ModuleNode* node, std::string path){
	if(node->type == NODE_TYPE_FILE){
		// finally a file, now export it!
	} else if(node->type == NODE_TYPE_DIRECTORY){
		// still a directory, we need to go deeper!
		for(auto const&[name,node] : node->children){

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
