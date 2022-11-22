#include "DataTableViewer.h"

#include <string>


DataTableViewer::DataTableViewer(){
	set_orientation(Gtk::ORIENTATION_HORIZONTAL);

	// set up the ListStore for the TreeView
	Gtk::TreeModel::ColumnRecord record;


	record.add(OffsetColumn);
	record.add(SizeColumn);
	record.add(RegionColumn);
	record.add(indexColumn);

	store = Gtk::ListStore::create(record);

	view = new Gtk::TreeView(store);

	//Gtk::CellRendererText* OffsetRenderer = new Gtk::CellRendererText();
	OffsetViewColumn = new Gtk::TreeViewColumn("Offset",OffsetColumn);
	SizeViewColumn = new Gtk::TreeViewColumn("Size",SizeColumn);
	RegionViewColumn = new Gtk::TreeViewColumn("Region",RegionColumn);

	OffsetViewColumn->set_resizable(true);
	OffsetViewColumn->set_min_width(10);
	SizeViewColumn->set_resizable(true);
	SizeViewColumn->set_min_width(10);
	RegionViewColumn->set_resizable(true);
	RegionViewColumn->set_min_width(10);

	view->append_column(*OffsetViewColumn);
	view->append_column(*SizeViewColumn);
	view->append_column(*RegionViewColumn);
	view->set_hexpand(true);
	view->set_vexpand(true);
	view->show();
	view->set_show_expanders(true);

	// Tree view is done, but it needs a frame to look nicer and a ScrolledWindow to be scrollable

	scroller = new Gtk::ScrolledWindow();
	scroller->add(*view);
	scroller->show();
	scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	viewFrame = new Gtk::Frame();
	viewFrame->show();
	add(*viewFrame);
	viewFrame->add(*scroller);

	// the options on the side. Currently only export
	exportButton = new Gtk::Button("Export");
	exportButton->show();
	add(*exportButton);
	exportButton->signal_clicked().connect([this]{
		// lambda to export the currently selected Data Block
		if(this->item == nullptr){
			// no item loaded
			return;
		}
		int idx;
		std::vector selected =  this->view->get_selection()->get_selected_rows();
		if(selected.size() == 0){
			// nothing selected
			return;
		}

		// only single selection should be allowed, so index 0 should be the only valid index in the vector, and it should be valid.
		Gtk::TreeModel::Row row = *this->view->get_selection()->get_selected();
		idx = row.get_value(indexColumn);

		int response = exportDialog->run();
		exportDialog->close();
		if(response == Gtk::RESPONSE_CANCEL){
			return;
		}else if(response == Gtk::RESPONSE_OK){
			FILE* f = fopen(exportDialog->get_filename().c_str(),"w+b");
			DataTableEntry* entry = &this->item->dataTable.entries[idx];
			fwrite(this->item->getDataBlock(entry), 1, entry->size, f);
			fclose(f);
		}

	});

	exportDialog = new Gtk::FileChooserDialog("Export",Gtk::FILE_CHOOSER_ACTION_SAVE);
	exportDialog->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	exportDialog->add_button("_Save", Gtk::RESPONSE_OK);



	this->item = nullptr;
}

DataTableViewer::~DataTableViewer(){
	delete exportDialog;
	delete exportButton;
	delete viewFrame;
	delete scroller;
	delete OffsetViewColumn;
	delete SizeViewColumn;
	delete RegionViewColumn;
	delete view;
}

void DataTableViewer::setItem(Item* item){
	this->item = item;
	populateTable(&item->dataTable);
}

void DataTableViewer::selectEntry(int index){
	view->get_selection()->unselect_all();
	view->get_selection()->select(entries[index]);
}

void DataTableViewer::populateTable(DataTable* table){
	if(this->item == nullptr){
		store->clear();
		entries.clear();
		return;
	}
	store->clear();
	entries.clear();
	entries.reserve(table->entries.size());
	for(int i = 0; i < table->entries.size(); i++){
		// iterating over the Data Table
		Gtk::TreeIter iter = store->append();
		iter->set_value(OFFSET_COLUMN, uint32ToHexString(table->entries[i].offset));
		iter->set_value(SIZE_COLUMN, uint32ToHexString(table->entries[i].size));
		iter->set_value(REGION_COLUMN, uint16ToHexString(table->entries[i].region));
		iter->set_value(indexColumn, i);	// the index
		entries.emplace_back(iter);	// store the iter to be able to easily retrieve it later
	}
}
