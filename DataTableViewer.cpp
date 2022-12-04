#include "DataTableViewer.h"

#include <string>


DataTableViewer::DataTableViewer(){
	set_orientation(Gtk::ORIENTATION_VERTICAL);
	HBox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
	HBox->show();
	add(*HBox);

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
	HBox->add(*viewFrame);
	viewFrame->add(*scroller);


	// Settings

	settingsBox = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
	settingsBox->show();
	HBox->add(*settingsBox);
	settingsLabel = new Gtk::Label("Viewer Settings");
	settingsLabel->show();
	settingsBox->add(*settingsLabel);
	settingsSeparator = new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL);
	settingsSeparator->show();
	settingsBox->add(*settingsSeparator);

	// the options on the side. Currently only export
	exportButton = new Gtk::Button("Export");
	exportButton->show();
	settingsBox->add(*exportButton);
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

	// Paging
	page = 0;
	pageSize = 0;
	pageCount = 0;
	pageSeparator = new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL);
	pageSeparator->show();
	pageSeparator->set_margin_top(8);
	pageSeparator->set_margin_bottom(8);
	settingsBox->add(*pageSeparator);
	pageCountLabel = new Gtk::Label("Strings per Page");
	pageCountLabel->show();
	settingsBox->add(*pageCountLabel);
	pageSizeSpinner = new Gtk::SpinButton();
	pageSizeSpinner->show();
	pageSizeSpinner->get_adjustment()->set_upper(20000);
	pageSizeSpinner->get_adjustment()->set_lower(1);
	pageSizeSpinner->get_adjustment()->set_step_increment(1);
	pageSizeSpinner->set_value(5000);
	pageSizeSpinner->signal_changed().connect([this]{
		updatePaging();
		populateTable(&item->dataTable);
	});
	settingsBox->add(*pageSizeSpinner);
	pageNumberLabel = new Gtk::Label("Page");
	pageNumberLabel->show();
	pageNumberLabel->set_margin_top(8);
	settingsBox->add(*pageNumberLabel);
	pageNumberButton = new Gtk::SpinButton();
	pageNumberButton->show();
	pageNumberButton->get_adjustment()->set_step_increment(1);
	pageNumberButton->get_adjustment()->set_lower(1);
	pageNumberButton->get_adjustment()->set_upper(1);
	pageNumberButton->set_value(1);
	pageNumberButton->signal_changed().connect([this]{
		updatePaging();
		populateTable(&item->dataTable);
	});
	settingsBox->add(*pageNumberButton);


	// Status Bar at the bottom

	statusBar = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
	statusBar->show();

	statusShownBlocksLabel = new Gtk::Label("/");
	statusShownBlocksLabel->show();
	statusShownBlocks = new Gtk::Label("");
	statusShownBlocks->show();
	statusBar->add(*statusShownBlocks);
	statusBar->add(*statusShownBlocksLabel);

	statusBlockCountLabel = new Gtk::Label(" Blocks");
	statusBlockCountLabel->show();
	statusBlockCount = new Gtk::Label("");
	statusBlockCount->show();
	add(*statusBar);
	statusBar->add(*statusBlockCount);
	statusBar->add(*statusBlockCountLabel);



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

	delete pageSeparator;
	delete pageCountLabel;
	delete pageSizeSpinner;
	delete pageNumberLabel;
	delete pageNumberButton;

	delete statusShownBlocksLabel;
	delete statusShownBlocks;
	delete statusBlockCountLabel;
	delete statusBlockCount;
	delete statusBar;

	delete HBox;

	delete view;
}

void DataTableViewer::updatePaging(){
	blockCount = item->dataTable.entries.size();
	page = pageNumberButton->get_value_as_int();
	pageSize = pageSizeSpinner->get_value_as_int();
	pageCount = blockCount / pageSize;
	if(blockCount % pageSize != 0){
		pageCount++;
	}
	pageNumberButton->get_adjustment()->set_upper(pageCount);
	if(page > pageCount){
		page = pageCount;
		pageNumberButton->set_value(page);
	}
}

void DataTableViewer::setItem(Item* item){
	this->item = item;
	updatePaging();
	populateTable(&item->dataTable);
}

void DataTableViewer::selectEntry(int index){
	// select the right page
	int pagenr = index / pageSize;
	pageNumberButton->set_value(pagenr+1);	// the page number internally starts at 0, but it shown starting at 1
	updatePaging();
	populateTable(&item->dataTable);

	view->get_selection()->unselect_all();
	view->get_selection()->select(entries[index % pageSize]);
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
	int start = (page-1) * pageSize;
	int count = 0;
	for(int i = start; i < table->entries.size(); i++){
		// iterating over the Data Table
		if(count >= pageSize) break;
		Gtk::TreeIter iter = store->append();
		iter->set_value(OFFSET_COLUMN, uint32ToHexString(table->entries[i].offset));
		iter->set_value(SIZE_COLUMN, uint32ToHexString(table->entries[i].size));
		iter->set_value(REGION_COLUMN, uint16ToHexString(table->entries[i].region));
		iter->set_value(indexColumn, i);	// the index
		entries.emplace_back(iter);	// store the iter to be able to easily retrieve it later
		count++;
	}
	statusBlockCount->set_label(std::to_string(item->dataTable.entries.size()));
	statusShownBlocks->set_label(std::to_string(start) + "-" + std::to_string(start + count));
}
