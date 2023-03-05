#include "InfiniteFileViewer.h"

#include "libInfinite/BitmapHandle.h"
#include "BitmapViewer.h"

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

	// check the tag class and maybe load additional stuff. This might turn into some spaghetti
	//
	// this also means that this object cannot be reused with a different item
	// as additional viewers would get added each time. If this is an issue, all additional
	// viewers could be deleted and the notebook could be rebuilt in this function
	// but this is currently not needed, so no point in doing so
	if(item->moduleItem->tagType == 'bitm'){
		// add a BitmapViewer
		BitmapViewer* bmv = new BitmapViewer();
		bmv->show();
		bmv->setItem(item);
		notebook->append_page(*bmv,bmv->getName());
		optionalViewers.emplace_back(bmv);
	}
}

InfiniteFileViewer::~InfiniteFileViewer(){
	// optional viewers first
	for(auto v = optionalViewers.begin(); v != optionalViewers.end(); v++){
		delete *v;
	}

	delete contentTableViewer;
	delete dataTableViewer;
	delete stringTableViewer;
	delete notebook;
}
