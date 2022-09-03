#pragma once

#include <gtkmm.h>
#include "InfiniteFileViewer.h"
#include "libInfinite/Item.h"
#include "ClosableTab.h"

#include <vector>
#include <map>

class FileViewerManager : public Gtk::Frame{
	// this class keeps track of opened files and allows switching between them
public:
	InfiniteFileViewer* viewer;	// the viewer which displays the files. This handles all the different stuff that can be displayed
	FileViewerManager();

	void addItem(Item* item);


	std::map<std::string,std::pair<Item*, Gtk::Label*>> items;
	Gtk::Notebook* itemNotebook;

private:
	Gtk::StackSwitcher* switcher;
	Gtk::Box* mainBox;
	Gtk::Label* noFileLabel;
	Gtk::Stack* stack;



	//std::vector<Item*> items;

};
