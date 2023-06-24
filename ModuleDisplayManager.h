#pragma once
#include "libInfinite/module/Module.h"
#include "libInfinite/logger/logger.h"
#include "libInfinite/module/ModuleNode.h"
#include "FileList.h"
#include "FileViewerManager.h"
#include "libInfinite/module/ModuleManager.h"

#include "libInfinite/tags/TagManager.h"

#include <string>

#define SEARCH_MAX_RESULTS 2000 // stop searching after finding this many results. If too many results are displayed, it may make the GUI unresponsive, which makes it difficult to type a different query

class ModuleDisplayManager{
public:
	ModuleDisplayManager(Logger* logger);

	void openModuleDialog();
	void openPathDialog();
	void loadFileDialog();
	void exportEntryDialog();
	void exportEntry(std::string path);
	void showNode(ModuleNode* node, bool outOfTree = false);	// if outOfTree is set, currentNode won't be updated (for displaying stuff like search results)
	void setupCallbacks();
	void searchNodes(ModuleNode* from, std::string query);
	void batchExtractTextures();
	void displayItem(ModuleItem* item, std::string name, std::string path);

	ModuleNode* currentNode = nullptr;
	FileList* fileList;
	Logger* logger;

	FileViewerManager* fileViewerManager;

	TagManager tagManager;


	ModuleManager modMan;

	// the order in these vectors has to match, i.e. the FileEntry* and the associated ModuleNode* have to have the same index
	//std::vector<ModuleNode*> fileEntryNodes;
private:
	std::vector<FileEntry*> fileEntries;
	void exportMultiple();
	void exportNode(ModuleNode* node, std::string path, bool fullPath = false);	// recursive function to export a node

	void loadPathRecursive(std::string path);

	void batchTexturesRecursive(ModuleNode* node, std::string path);

	// expects a cleaned up query
	int searchNodesRecursive(ModuleNode* node, std::string query, int index);

	// this node is used to display search results.
	ModuleNode searchNode;

	//void showNodeID(int ID,void* data);

};
