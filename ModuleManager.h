#pragma once
#include "libInfinite/module/Module.h"
#include "ModuleNode.h"
#include "FileList.h"
#include <string>

#define SEARCH_MAX_RESULTS 2000 // stop searching after finding this many results. If too many results are displayed, it may make the GUI unresponsive, which makes it difficult to type a different query

class ModuleManager{
public:
	void openModuleDialog();
	void openPathDialog();
	void exportEntryDialog();
	void exportEntry(std::string path);
	void buildNodeTree();
	void showNode(ModuleNode* node, bool outOfTree = false);	// if outOfTree is set, currentNode won't be updated (for displaying stuff like search results)
	void setupCallbacks();
	void searchNodes(ModuleNode* from, std::string query);
	std::pair<uint64_t,uint64_t> getSizes(ModuleNode* node);	// first is uncompressed, second is compressed
	std::vector<Module*> modules;
	ModuleNode* rootNode;
	ModuleNode* currentNode = nullptr;
	FileList* fileList;


	// the order in these vectors has to match, i.e. the FileEntry* and the associated ModuleNode* have to have the same index
	//std::vector<ModuleNode*> fileEntryNodes;
private:
	std::vector<FileEntry*> fileEntries;
	void exportMultiple();
	void exportNode(ModuleNode* node, std::string path, bool fullPath = false);	// recursive function to export a node

	void loadPathRecursive(std::string path);

	// expects a cleaned up query
	int searchNodesRecursive(ModuleNode* node, std::string query, int index);

	// this node is used to display search results.
	ModuleNode searchNode;

	//void showNodeID(int ID,void* data);

};
