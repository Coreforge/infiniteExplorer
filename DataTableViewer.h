#pragma once

#include <gtkmm.h>
#include <vector>

#include "libInfinite/Item.h"
#include "ViewerUtils.h"

#define HXD_ENABLE
#define HXD_COMMAND "wine \"/home/ich/.wine/drive_c/Program Files/HxD/HxD.exe\" Z:"
#define HXD_TMP_DIR "/tmp/ie/"

class DataTableViewer : public Gtk::Box{

public:
	DataTableViewer();
	~DataTableViewer();
	void setItem(Item* item);
	void selectEntry(int index);

private:
	Gtk::Box* HBox;
	Gtk::Frame* viewFrame;
	Gtk::ScrolledWindow* scroller;

	// since the colums should be resizable, I can't just use a Gtk::Grid, but instead have to use Gtk::Paned to separate two columns from each other
	// and then use a Gtk::Box to get the rows. Since it will just be filled with labels, height shouldn't be an issue

	Gtk::TreeView* view;
	Glib::RefPtr<Gtk::ListStore> store;

	Gtk::TreeViewColumn* OffsetViewColumn;
	Gtk::TreeViewColumn* SizeViewColumn;
	Gtk::TreeViewColumn* RegionViewColumn;

	Gtk::Button* exportButton;
	Gtk::FileChooserDialog* exportDialog;
	Gtk::Button hexButton;

	Gtk::TreeModelColumn<std::string> OffsetColumn;
	Gtk::TreeModelColumn<std::string> SizeColumn;
	Gtk::TreeModelColumn<std::string> RegionColumn;
	Gtk::TreeModelColumn<int> indexColumn;

	Item* item;	// the item whose data table should be displayed

	std::vector<Gtk::TreeIter> entries;

	// settings

	Gtk::Separator* settingsSeparator;
	Gtk::Box* settingsBox;	// contain the different settings for the viewer
	Gtk::Label* settingsLabel;

	// page controls
	int page;
	int pageSize;
	int pageCount;
	int blockCount;

	Gtk::Separator* pageSeparator;
	Gtk::Label* pageCountLabel;
	Gtk::SpinButton* pageSizeSpinner;
	Gtk::Label* pageNumberLabel;
	Gtk::SpinButton* pageNumberButton;


	// status bar

	Gtk::Box* statusBar;
	Gtk::Label* statusBlockCountLabel;
	Gtk::Label* statusBlockCount;
	Gtk::Label* statusShownBlocksLabel;
	Gtk::Label* statusShownBlocks;

	void populateTable(DataTable* table);
	void updatePaging();

	enum{
		OFFSET_COLUMN,
		SIZE_COLUMN,
		REGION_COLUMN,
		N_COLUMNS
	};

};
