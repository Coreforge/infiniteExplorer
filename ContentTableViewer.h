#pragma once

#include <gtkmm.h>
#include <vector>

#include "libInfinite/ContentTable.h"
#include "libInfinite/Item.h"

#include "ViewerUtils.h"

class ContentTableViewer : public Gtk::Box{
public:
	ContentTableViewer();
	void setItem(Item* item);
	void setShowDataCallback(std::function<void(int)> callback);

private:

	// start easily copyable code
	Gtk::Frame* viewFrame;
	Gtk::ScrolledWindow* scroller;

	Gtk::TreeView* view;
	Glib::RefPtr<Gtk::TreeStore> store;

	// Columns

	Gtk::TreeViewColumn* TypeViewColumn;
	Gtk::TreeViewColumn* RefViewColumn;
	Gtk::TreeViewColumn* RefSizeViewColumn;
	Gtk::TreeViewColumn* ParentViewColumn;

	// Columns for the TreeStore
	Gtk::TreeModelColumn<std::string> TypeColumn;
	Gtk::TreeModelColumn<std::string> RefColumn;
	Gtk::TreeModelColumn<std::string> RefSizeColumn;
	Gtk::TreeModelColumn<std::string> ParentColumn;
	Gtk::TreeModelColumn<int> IndexColumn;

	Item* item;

	enum{
		TYPE_COLUMN,
		REF_COLUMN,
		REF_SIZE_COLUMN,
		PARENT_COLUMN,
		N_COLUMNS
	};

	// end easily copyable code

	Gtk::Separator* settingsSeparator;
	Gtk::Box* settingsBox;	// contain the different settings for the viewer
	Gtk::Label* settingsLabel;

	Gtk::ComboBoxText* typeDisplayModeBox;	// how the 'type' field is displayed
	Gtk::Label* typeDisplayModeLabel;

	enum{
		MODE_GUID,
		MODE_HEXL,
		MODE_HEXB,
		MODE_PYTHON
	};

	int typeMode;

	std::function<void(int)> showDataCallback;

	// stuff for data storage/data handling

	class TreeEntry{
	public:
		std::vector<TreeEntry*> children;
		int idx;	// index of this entry in the Content Table
		bool hasParent;
	};

	std::vector<TreeEntry*> tree;

	std::string getTypeString(TypeGUID type);
	void generateTree(bool tree);
	void deleteTree();	// clean up the tree
	void deleteTreeRecursive(TreeEntry* entry);
	void fillStore();
	void fillStoreRecursive(TreeEntry* entry, Gtk::TreeStore::iterator iter);



};

