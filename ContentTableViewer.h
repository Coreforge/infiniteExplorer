#pragma once

#include <gtkmm.h>
#include <vector>

#include "libInfinite/ContentTable.h"
#include "libInfinite/Item.h"

#include "ViewerUtils.h"

class ContentTableViewer : public Gtk::Box{
public:
	ContentTableViewer();
	~ContentTableViewer();
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
	Gtk::TreeViewColumn* TypeIDViewColumn;
	Gtk::TreeViewColumn* RefViewColumn;
	Gtk::TreeViewColumn* RefSizeViewColumn;
	Gtk::TreeViewColumn* ParentViewColumn;
	Gtk::TreeViewColumn* FieldOffsetViewColumn;

	Gtk::CellRendererText typeRenderer;

	// Columns for the TreeStore
	Gtk::TreeModelColumn<std::string> TypeColumn;
	Gtk::TreeModelColumn<std::string> TypeIDColumn;
	Gtk::TreeModelColumn<std::string> RefColumn;
	Gtk::TreeModelColumn<std::string> RefSizeColumn;
	Gtk::TreeModelColumn<std::string> ParentColumn;
	Gtk::TreeModelColumn<std::string> FieldOffsetColumn;
	Gtk::TreeModelColumn<uint32_t> IndexColumn;

	Item* item;

	enum{
		TYPE_COLUMN,
		TYPE_ID_COLUMN,
		REF_COLUMN,
		REF_SIZE_COLUMN,
		PARENT_COLUMN,
		FIELD_OFFET_COLUMN,
		N_COLUMNS
	};

	// end easily copyable code

	Gtk::Separator* settingsSeparator;
	Gtk::Box* settingsBox;	// contain the different settings for the viewer
	Gtk::Label* settingsLabel;

	Gtk::ComboBoxText* typeDisplayModeBox;	// how the 'type' field is displayed
	Gtk::Label* typeDisplayModeLabel;

	Gtk::Box OffsetBox;
	Gtk::Label Label0x;
	Gtk::Entry OffsetEntry;
	Gtk::Button SearchOffsetButton;
	Gtk::RadioButton StartOffsetCheckButton;
	Gtk::RadioButton TagOffsetCheckButton;


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
	void fillStoreRecursive(ContentTableEntry* entry, Gtk::TreeStore::iterator iter);

	bool findOffset(uint32_t off,Gtk::TreeStore::iterator iter);



};

