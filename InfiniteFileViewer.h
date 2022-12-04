#pragma once

#include "libInfinite/Item.h"

#include "DataTableViewer.h"
#include "ContentTableViewer.h"
#include "StringTableViewer.h"

#include <gtkmm.h>


class InfiniteFileViewer : public Gtk::Frame{
public:
	InfiniteFileViewer();
	void setItem(Item* item);
	~InfiniteFileViewer();


private:
	Gtk::Notebook* notebook;

	DataTableViewer* dataTableViewer;
	ContentTableViewer* contentTableViewer;
	StringTableViewer* stringTableViewer;
};
