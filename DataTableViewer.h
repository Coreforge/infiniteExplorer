#pragma once

#include <gtkmm.h>
#include <vector>

#include "libInfinite/Item.h"
#include "ViewerUtils.h"

class DataTableViewer : public Gtk::Box{

public:
	DataTableViewer();
	~DataTableViewer();
	void setItem(Item* item);
	void selectEntry(int index);

private:
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

	Gtk::TreeModelColumn<std::string> OffsetColumn;
	Gtk::TreeModelColumn<std::string> SizeColumn;
	Gtk::TreeModelColumn<std::string> RegionColumn;
	Gtk::TreeModelColumn<int> indexColumn;

	Item* item;	// the item whose data table should be displayed

	std::vector<Gtk::TreeIter> entries;


	void populateTable(DataTable* table);

	enum{
		OFFSET_COLUMN,
		SIZE_COLUMN,
		REGION_COLUMN,
		N_COLUMNS
	};

};
