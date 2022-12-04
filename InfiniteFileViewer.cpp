#include "InfiniteFileViewer.h"

InfiniteFileViewer::InfiniteFileViewer(){
	notebook = new Gtk::Notebook();
	notebook->show();
	add(*notebook);

	// DataTable Viewer
	dataTableViewer = new DataTableViewer();
	dataTableViewer->show();
	notebook->append_page(*dataTableViewer, "Data Table");

	contentTableViewer = new ContentTableViewer();
	contentTableViewer->show();
	contentTableViewer->setShowDataCallback([this] (int idx){
		dataTableViewer->selectEntry(idx);
		notebook->set_current_page(0);
	});
	notebook->append_page(*contentTableViewer, "Content Table");

	// StringTable Viewer
	stringTableViewer = new StringTableViewer();
	stringTableViewer->show();
	notebook->append_page(*stringTableViewer, "String Table");
}

void InfiniteFileViewer::setItem(Item* item){
	dataTableViewer->setItem(item);
	contentTableViewer->setItem(item);
	stringTableViewer->setItem(item);
}

InfiniteFileViewer::~InfiniteFileViewer(){
	delete contentTableViewer;
	delete dataTableViewer;
	delete stringTableViewer;
	delete notebook;
}
