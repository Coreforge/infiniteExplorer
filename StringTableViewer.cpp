#include "StringTableViewer.h"

#define IVALID "v"
#define IINVALID "i"
#define IBOTH "b"

StringTableViewer::StringTableViewer(){
	set_orientation(Gtk::ORIENTATION_VERTICAL);

	HBox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
	add(*HBox);
	HBox->show();

	Gtk::TreeModel::ColumnRecord record;

	record.add(IndexColumn);
	record.add(TypeColumn);
	record.add(StringColumn);
	record.add(IsValidColumn);

	store = Gtk::TreeStore::create(record);
	view = new Gtk::TreeView(store);

	IndexViewColumn = new Gtk::TreeViewColumn("Index",IndexColumn);
	TypeViewColumn = new Gtk::TreeViewColumn("Type",TypeColumn);
	StringViewColumn = new Gtk::TreeViewColumn("String",StringColumn);
	IsValidViewColumn = new Gtk::TreeViewColumn("Valid",IsValidColumn);

	IndexViewColumn->set_resizable(true);
	IndexViewColumn->set_min_width(10);
	TypeViewColumn->set_resizable(true);
	TypeViewColumn->set_min_width(10);
	StringViewColumn->set_resizable(true);
	StringViewColumn->set_min_width(10);
	IsValidViewColumn->set_resizable(true);
	IsValidViewColumn->set_min_width(10);

	view->append_column(*IndexViewColumn);
	view->append_column(*TypeViewColumn);
	view->append_column(*StringViewColumn);
	view->append_column(*IsValidViewColumn);
	view->set_hexpand(true);
	view->set_vexpand(true);
	view->show();
	view->set_show_expanders(true);

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

	regexEntry = new Gtk::SearchEntry();
	regexLabel = new Gtk::Label("Regex");
	regexLabel->show();
	regexEntry->show();
	cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("res/Entry.css");
	regexEntry->get_style_context()->add_provider(cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	regexEntry->signal_search_changed().connect([this]{
		try{
			stringRegex.assign(regexEntry->get_text().c_str());
		}catch (const std::regex_error& e){
			//printf("Regex error: %s\n",e.what());
			regexEntry->get_style_context()->add_class("error");
			return;
		}
		regexEntry->get_style_context()->remove_class("error");
		calculateCounts();
		updatePaging();
		updateRecord();
		return;
	});
	regexMode = new Gtk::CheckButton("Full match");
	regexMode->show();
	regexMode->signal_clicked().connect([this]{
		regexFullMatch = regexMode->get_active();
		calculateCounts();
		updatePaging();
		updateRecord();
		return;
	});
	regexFullMatch = false;
	regexExcludeButton = new Gtk::CheckButton("Exclude Regex");
	regexExcludeButton->show();
	regexExcludeButton->signal_clicked().connect([this]{
		regexExclude = regexExcludeButton->get_active();
		calculateCounts();
		updatePaging();
		updateRecord();
		return;
	});
	regexExclude = false;
	stringRegex.assign(".*");

	settingsBox->add(*regexLabel);
	settingsBox->add(*regexEntry);
	settingsBox->add(*regexMode);
	settingsBox->add(*regexExcludeButton);



	validIndiciesBox = new Gtk::ComboBoxText();
	validIndiciesLabel = new Gtk::Label("String Indices");
	validIndiciesBox->show();
	validIndiciesLabel->show();
	validIndiciesLabel->set_margin_top(8);
	validIndiciesBox->append(IVALID,"Valid");	// INDEX_VALID
	validIndiciesBox->append(IINVALID,"Invalid");	// INDEX_INVALID
	validIndiciesBox->append(IBOTH,"Both");	// INDEX_BOTH
	validIndiciesBox->set_active_id(IVALID);
	validIndiciesBox->signal_changed().connect([this] {
			std::string active = validIndiciesBox->get_active_id();
			if(active == IVALID){
				indexMode = INDEX_VALID;
			}
			if(active == IINVALID){
				indexMode = INDEX_INVALID;
			}
			if(active == IBOTH){
				indexMode = INDEX_BOTH;
			}
			calculateCounts();
			updatePaging();
			updateRecord();
	});
	indexMode = INDEX_VALID;
	settingsBox->add(*validIndiciesLabel);
	settingsBox->add(*validIndiciesBox);

	showDuplicatesButton = new Gtk::ToggleButton("Show Duplicates");
	showDuplicatesButton->show();
	settingsBox->add(*showDuplicatesButton);
	showDuplicatesButton->set_margin_top(8);
	showDuplicates = false;
	showDuplicatesButton->set_active(false);
	showDuplicatesButton->signal_clicked().connect([this] {
		showDuplicates = showDuplicatesButton->get_active();
		calculateCounts();
		updatePaging();
		updateRecord();
	});

	// Paging
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
		calculateCounts();
		updatePaging();
		updateRecord();
	});
	settingsBox->add(*pageSizeSpinner);
	pageNumberLabel = new Gtk::Label("Page");
	pageNumberLabel->show();
	pageNumberLabel->set_margin_top(8);
	settingsBox->add(*pageNumberLabel);
	pageNumberButton = new Gtk::SpinButton();
	pageNumberButton->show();
	pageNumberButton->get_adjustment()->set_lower(1);
	pageNumberButton->get_adjustment()->set_upper(1);
	pageNumberButton->set_value(1);
	pageNumberButton->get_adjustment()->set_step_increment(1);
	pageNumberButton->signal_changed().connect([this]{
		updatePaging();
		updateRecord();
	});
	settingsBox->add(*pageNumberButton);

	// Status Bar at the bottom

	statusBar = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
	statusBar->show();
	statusStringCountLabel = new Gtk::Label("Strings: ");
	statusStringCountLabel->show();
	statusStringCount = new Gtk::Label("");
	statusStringCount->show();
	add(*statusBar);
	statusBar->add(*statusStringCountLabel);
	statusBar->add(*statusStringCount);

	statusUniqueCountLabel = new Gtk::Label("Unique: ");
	statusUniqueCountLabel->show();
	statusUniqueCount = new Gtk::Label("");
	statusUniqueCount->show();
	statusSeparator1 = new Gtk::Separator(Gtk::ORIENTATION_VERTICAL);
	statusSeparator1->show();
	statusSeparator1->set_margin_left(5);
	statusSeparator1->set_margin_right(5);
	statusBar->add(*statusSeparator1);
	statusBar->add(*statusUniqueCountLabel);
	statusBar->add(*statusUniqueCount);

	item = nullptr;

}

StringTableViewer::~StringTableViewer(){

	delete pageSeparator;
	delete pageCountLabel;
	delete pageSizeSpinner;
	delete pageNumberLabel;
	delete pageNumberButton;

	delete regexLabel;
	delete regexEntry;
	delete regexMode;
	delete regexExcludeButton;

	delete settingsSeparator;
	delete settingsLabel;
	delete settingsBox;
	delete viewFrame;
	delete scroller;
	delete IndexViewColumn;
	delete TypeViewColumn;
	delete StringViewColumn;
	delete IsValidViewColumn;

	delete statusStringCountLabel;
	delete statusStringCount;
	delete statusUniqueCountLabel;
	delete statusUniqueCount;
	delete statusSeparator1;
	delete statusBar;

	delete HBox;

	delete view;
}

void StringTableViewer::setItem(Item* item){
	this->item = item;
	filterUniqueEntries();
	calculateCounts();
	updatePaging();
	updateRecord();
}

bool StringTableViewer::regexCheck(std::string str){
	bool match;
	if(regexFullMatch){
		match = std::regex_match(str, stringRegex);
	} else {
		match = std::regex_search(str, stringRegex);
	}
	return match ^ regexExclude;
}

void StringTableViewer::filterUniqueEntries(){
	// filtering for unique entries only makes sense for strings with valid indices
	uniqueEntries.clear();
	for(int i = 0; i < item->stringTable.strings.size(); i++){
		if(uniqueEntries.count(item->stringTable.strings[i].index) == 0){
			uniqueEntries.insert({item->stringTable.strings[i].index,&item->stringTable.strings[i]});
		}
	}
	statusUniqueCount->set_label(std::to_string(uniqueEntries.size()));
}

void StringTableViewer::updatePaging(){
	page = pageNumberButton->get_value_as_int();
	pageSize = pageSizeSpinner->get_value_as_int();
	pageCount = displayableStrings / pageSize;
	if(displayableStrings % pageSize != 0){
		pageCount++;
	}
	pageNumberButton->get_adjustment()->set_upper(pageCount);
	if(page > pageCount){
		page = pageCount;
		pageNumberButton->set_value(page);
	}
}

void StringTableViewer::calculateCounts(){
	displayableStrings = 0;

	if(indexMode == INDEX_VALID || indexMode == INDEX_BOTH){
		if(showDuplicates){
			for(int i = 0; i < item->stringTable.strings.size(); i++){
				if(!regexCheck(item->stringTable.strings[i].string)) continue;
				displayableStrings++;
			}
		} else {
			for(auto const&[key,value] : uniqueEntries){
				if(!regexCheck(value->string)) continue;
				displayableStrings++;
			}
		}
	}
	if(indexMode == INDEX_INVALID || indexMode == INDEX_BOTH){
		for(int i = 0; i < item->stringTable.invalidStrings.size(); i++){
			if(!regexCheck(item->stringTable.invalidStrings[i].string)) continue;
			displayableStrings++;
		}
	}
}

void StringTableViewer::updateRecord(){
	store->clear();
	if(this->item == nullptr){
		return;
	}
	uint32_t count = 0;
	uint32_t visibleCount = 0;
	// count is the total number of string table entries in this item
	count += item->stringTable.invalidStrings.size();
	count += item->stringTable.strings.size();
	int start = (page-1) * pageSize;
	if(indexMode == INDEX_VALID || indexMode == INDEX_BOTH){
		if(showDuplicates){
			for(int i = start; i < item->stringTable.strings.size(); i++){
				if(visibleCount >= pageSize) break;
				if(!regexCheck(item->stringTable.strings[i].string)) continue;
				Gtk::TreeIter iter = store->append();
				iter->set_value(INDEX_COLUMN, item->stringTable.strings[i].index);
				iter->set_value(TYPE_COLUMN, item->stringTable.strings[i].type);
				iter->set_value(STRING_COLUMN, item->stringTable.strings[i].string);
				iter->set_value(ISVALID_COLUMN, true);
				visibleCount++;
				start++;
			}
		} else {
			std::map<uint32_t,StringTableEntry*>::iterator it = uniqueEntries.begin();
			// move the iterator to where the current page starts
			for(int i = 0; i < start && it != uniqueEntries.end(); i++){
				it++;
			}
			while(it != uniqueEntries.end() && visibleCount < pageSize){
			//for(auto const&[key,value] : uniqueEntries){
				//if(visibleCount >= pageSize) break;	// doesn't work properly yet
				if(!regexCheck(it->second->string)){
					it++;
					continue;
				}
				Gtk::TreeIter iter = store->append();
				iter->set_value(INDEX_COLUMN, it->second->index);
				iter->set_value(TYPE_COLUMN, it->second->type);
				iter->set_value(STRING_COLUMN, it->second->string);
				iter->set_value(ISVALID_COLUMN, true);
				visibleCount++;
				start++;
				it++;
			}
		}
	}
	if(indexMode == INDEX_INVALID || indexMode == INDEX_BOTH){
		//start = (page-1-(visibleCount/pageSize)) * pageSize;
		printf("%d\n",start);
		for(int i = start; i < item->stringTable.invalidStrings.size(); i++){
			if(visibleCount >= pageSize) break;
			if(!regexCheck(item->stringTable.invalidStrings[i].string)) continue;
			Gtk::TreeIter iter = store->append();
			iter->set_value(INDEX_COLUMN, item->stringTable.invalidStrings[i].index);
			iter->set_value(TYPE_COLUMN, item->stringTable.invalidStrings[i].type);
			iter->set_value(STRING_COLUMN, item->stringTable.invalidStrings[i].string);
			iter->set_value(ISVALID_COLUMN, false);
			visibleCount++;
			start++;
		}
	}

	statusStringCount->set_label(std::to_string(displayableStrings) + "/" + std::to_string(count));
}
