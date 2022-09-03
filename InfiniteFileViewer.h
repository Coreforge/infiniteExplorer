#pragma once

#include "libInfinite/Item.h"

#include "DataTableViewer.h"
#include "ContentTableViewer.h"

#include <gtkmm.h>


class InfiniteFileViewer : public Gtk::Frame{
public:
	InfiniteFileViewer();
	void setItem(Item* item);


private:
	Gtk::Notebook* notebook;

	DataTableViewer* dataTableViewer;
	ContentTableViewer* contentTableViewer;
};
