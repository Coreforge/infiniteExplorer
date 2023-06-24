#pragma once

#include <gtkmm.h>
#include <regex>

#include "libInfinite/ContentTable.h"
#include "libInfinite/Item.h"

class StringTableViewer : public Gtk::Box{
public:
	StringTableViewer();
	~StringTableViewer();
	void setItem(Item* item);

private:
	Gtk::Box* HBox;

	// start easily copyable code
	Gtk::Frame* viewFrame;
	Gtk::ScrolledWindow* scroller;

	Gtk::TreeView* view;
	Glib::RefPtr<Gtk::TreeStore> store;

	// Columns

	Gtk::TreeViewColumn* IndexViewColumn;
	Gtk::TreeViewColumn* TypeViewColumn;
	Gtk::TreeViewColumn* StringViewColumn;
	Gtk::TreeViewColumn* IsValidViewColumn;

	// Columns for the TreeStore
	Gtk::TreeModelColumn<int> IndexColumn;
	Gtk::TreeModelColumn<int> TypeColumn;
	Gtk::TreeModelColumn<std::string> StringColumn;
	Gtk::TreeModelColumn<bool> IsValidColumn;

	enum{
		INDEX_COLUMN,
		TYPE_COLUMN,
		STRING_COLUMN,
		ISVALID_COLUMN
	};


	Item* item;

	// settings

	Gtk::Separator* settingsSeparator;
	Gtk::Box* settingsBox;	// contain the different settings for the viewer
	Gtk::Label* settingsLabel;
	// regex filter
	Glib::RefPtr<Gtk::CssProvider> cssProvider;
	Gtk::SearchEntry* regexEntry;
	Gtk::Label* regexLabel;
	Gtk::CheckButton* regexMode;
	Gtk::CheckButton* regexExcludeButton;
	bool regexFullMatch;
	bool regexExclude;
	std::regex stringRegex;
	// valid/invalid/both indicies
	Gtk::ComboBoxText* validIndiciesBox;
	Gtk::Label* validIndiciesLabel;
	// remove duplicate Entries
	Gtk::ToggleButton* showDuplicatesButton;
	bool showDuplicates;
	enum{
		INDEX_VALID,
		INDEX_INVALID,
		INDEX_BOTH
	};

	int indexMode;
	int page;
	int pageSize;
	int pageCount;

	Gtk::Separator* pageSeparator;
	Gtk::Label* pageCountLabel;
	Gtk::SpinButton* pageSizeSpinner;
	Gtk::Label* pageNumberLabel;
	Gtk::SpinButton* pageNumberButton;

	// status bar

	Gtk::Box* statusBar;
	Gtk::Label* statusStringCountLabel;
	Gtk::Label* statusStringCount;
	Gtk::Separator* statusSeparator1;
	Gtk::Label* statusUniqueCountLabel;
	Gtk::Label* statusUniqueCount;

	std::map<uint32_t,TagRefFieldTableEntry*> uniqueEntries;

	uint32_t displayableStrings;

	bool regexCheck(std::string str);

	void updatePaging();
	void calculateCounts();
	void updateRecord();
	void filterUniqueEntries();
};
