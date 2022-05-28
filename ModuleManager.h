#pragma once
#include "libInfinite/module/Module.h"
#include "ModuleNode.h"
#include "FileList.h"
#include <string>

class ModuleManager{
public:
	void openModuleDialog();
	void openPathDialog();
	void exportEntryDialog();
	void exportEntry(std::string path);
	void buildNodeTree();
	void showNode(ModuleNode* node);
	std::vector<Module*> modules;
	ModuleNode* rootNode;
	ModuleNode* currentNode;
	FileList* fileList;


	// the order in these vectors has to match, i.e. the FileEntry* and the associated ModuleNode* have to have the same index
	//std::vector<ModuleNode*> fileEntryNodes;
private:
	std::vector<FileEntry*> fileEntries;
	void exportMultiple();
	void exportNode(ModuleNode* node, std::string path, bool fullPath = false);	// recursive function to export a node

	void loadPathRecursive(std::string path);


	//void showNodeID(int ID,void* data);
};
