#include "ContentTableViewer.h"

#include <stdint.h>

// display modes for the 16-byte type field
#define TYPE_GUID "guid"
#define TYPE_HEXL "hexl"
#define TYPE_HEXB "hexb"
#define TYPE_PYTHON "py"

ContentTableViewer::ContentTableViewer() {

	// this code is pretty much the same for all table/list viewers. Only the columns have to be changed

	set_orientation(Gtk::ORIENTATION_HORIZONTAL);

	Gtk::TreeModel::ColumnRecord record;

	record.add(TypeColumn);
	record.add(TypeIDColumn);
	record.add(RefColumn);
	record.add(RefSizeColumn);
	record.add(ParentColumn);
	record.add(FieldOffsetColumn);
	record.add(IndexColumn);

	store = Gtk::TreeStore::create(record);
	view = new Gtk::TreeView(store);

	TypeViewColumn = new Gtk::TreeViewColumn("Type",TypeColumn);
	TypeIDViewColumn = new Gtk::TreeViewColumn("Type ID",TypeIDColumn);
	RefViewColumn = new Gtk::TreeViewColumn("Reference Index",RefColumn);
	RefSizeViewColumn = new Gtk::TreeViewColumn("Reference Size",RefSizeColumn);
	ParentViewColumn = new Gtk::TreeViewColumn("Parent Index",ParentColumn);
	FieldOffsetViewColumn = new Gtk::TreeViewColumn("Field Offset",FieldOffsetColumn);

	TypeViewColumn->set_resizable(true);
	TypeViewColumn->set_min_width(10);
	TypeIDViewColumn->set_resizable(true);
	TypeIDViewColumn->set_min_width(10);
	RefViewColumn->set_resizable(true);
	RefViewColumn->set_min_width(10);
	RefSizeViewColumn->set_resizable(true);
	RefSizeViewColumn->set_min_width(10);
	ParentViewColumn->set_resizable(true);
	ParentViewColumn->set_min_width(10);
	FieldOffsetViewColumn->set_resizable(true);
	FieldOffsetViewColumn->set_min_width(10);

	TypeViewColumn->set_property("editable", true);
	//TypeViewColumn->set_renderer(typeRenderer, TypeColumn);

	view->append_column(*TypeViewColumn);
	view->append_column(*TypeIDViewColumn);
	view->append_column(*RefViewColumn);
	view->append_column(*RefSizeViewColumn);
	view->append_column(*ParentViewColumn);
	view->append_column(*FieldOffsetViewColumn);
	view->set_hexpand(true);
	view->set_vexpand(true);
	view->show();
	view->set_show_expanders(true);

	view->signal_row_activated().connect([this](Gtk::TreePath path, Gtk::TreeViewColumn* column){
		Gtk::TreeModel::Row row = *this->view->get_selection()->get_selected();
		int idx = row.get_value(IndexColumn);
		int ref = item->contentTable.entries[idx].ref;
		if(ref == 0xffffffff){
			return;
		}
		showDataCallback(ref);
	});

	scroller = new Gtk::ScrolledWindow();
	scroller->add(*view);
	scroller->show();
	scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	viewFrame = new Gtk::Frame();
	viewFrame->show();
	add(*viewFrame);
	viewFrame->add(*scroller);

	item = nullptr;
	// end easily copyable code

	settingsBox = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
	settingsBox->show();
	add(*settingsBox);
	settingsLabel = new Gtk::Label("Viewer Settings");
	settingsLabel->show();
	settingsBox->add(*settingsLabel);
	settingsSeparator = new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL);
	settingsSeparator->show();
	settingsBox->add(*settingsSeparator);

	typeDisplayModeLabel = new Gtk::Label("Type Display Mode:");
	typeDisplayModeLabel->show();
	typeDisplayModeLabel->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
	settingsBox->add(*typeDisplayModeLabel);

	typeDisplayModeBox = new Gtk::ComboBoxText();
	typeDisplayModeBox->show();
	typeDisplayModeBox->append(TYPE_HEXL, "Hex String - Little Endian");
	typeDisplayModeBox->append(TYPE_HEXB, "Hex String - Big Endian");
	typeDisplayModeBox->append(TYPE_PYTHON, "Python byte-literal");
	typeDisplayModeBox->set_active_id(TYPE_PYTHON);
	typeMode = MODE_PYTHON;
	typeDisplayModeBox->signal_changed().connect([this] {
		std::string active = typeDisplayModeBox->get_active_id();
		if(active == TYPE_GUID){
			typeMode = MODE_GUID;
		} else if(active == TYPE_HEXL){
			typeMode = MODE_HEXL;
		} else if(active == TYPE_HEXB){
			typeMode = MODE_HEXB;
		} else if(active == TYPE_PYTHON){
			typeMode = MODE_PYTHON;
		}

		// after changing the mode, the Store needs to be updated, as there is currently no custom CellRenderer or cell_data_func used, although that would be a cleaner way to do it
		fillStore();
	});

	settingsBox->add(*typeDisplayModeBox);

	StartOffsetCheckButton.set_label("from Start of File");
	TagOffsetCheckButton.set_label("from Start of Tag Data");
	TagOffsetCheckButton.join_group(StartOffsetCheckButton);
	StartOffsetCheckButton.show();
	TagOffsetCheckButton.show();
	settingsBox->add(StartOffsetCheckButton);
	settingsBox->add(TagOffsetCheckButton);

	Label0x.set_label("0x");
	OffsetBox.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	Label0x.show();
	OffsetBox.add(Label0x);
	OffsetEntry.show();
	OffsetBox.add(OffsetEntry);
	OffsetBox.show();
	settingsBox->add(OffsetBox);
	SearchOffsetButton.set_label("Search");
	settingsBox->add(SearchOffsetButton);
	SearchOffsetButton.show();

	SearchOffsetButton.signal_clicked().connect([this]{
		uint32_t off = std::stoul(OffsetEntry.get_text(), nullptr, 16);
		for(auto it = store->children().begin(); it != store->children().end(); it++){
			bool fromStart = StartOffsetCheckButton.get_active();
			int idx = it->get_value(IndexColumn);
			ContentTableEntry* cte = &item->contentTable.entries[idx];
			if(cte->ref == -1){
				continue;
			}
			DataTableEntry* dte = &item->dataTable.entries[cte->ref];
			uint32_t eoff = dte->offset;
			if(fromStart){
				eoff = item->getDataBlockOffset(dte);
			}
			uint32_t maxoff = eoff + dte->size;
			if(eoff <= off && off <= maxoff){
				//found it!
				Gtk::TreeModel::Row row = *it;
				Gtk::TreeModel::Path p = store->get_path(it);
				view->expand_to_path(p);
				view->scroll_to_row(p);
				view->get_selection()->unselect_all();
				view->get_selection()->select(row);
				return;
			}
			if(findOffset(off, it)){
				return;
			}

		}
	});

	view->add_events(Gdk::BUTTON_PRESS_MASK);
	view->signal_button_press_event().connect([this] (GdkEventButton* button){
		if(button->button == GDK_BUTTON_SECONDARY){
			std::string type = view->get_selection()->get_selected()->get_value(TypeColumn);
			auto clip = Gtk::Clipboard::get();
			clip->set_text(type);
			clip->store();
		}
		return false;
	},false);

}

ContentTableViewer::~ContentTableViewer(){
	delete typeDisplayModeBox;
	delete typeDisplayModeLabel;
	delete settingsSeparator;
	delete settingsLabel;
	delete settingsBox;
	delete viewFrame;
	delete scroller;
	delete TypeViewColumn;
	delete RefViewColumn;
	delete RefSizeViewColumn;
	delete ParentViewColumn;
	delete view;
	deleteTree();
}

void ContentTableViewer::setShowDataCallback(std::function<void(int)> callback){
	showDataCallback = callback;
}

std::string pythonByteLiteral(uint8_t byte){
	if(byte >= 32 && byte < 127){
		// printable ASCII-character
		if(byte == 39){
			// single quotes, since I'm putting single quotes around the literal, these have to be escaped
			return std::string("\\'");
		}
		// all other characters can just be used regularly
		return std::string(1,byte);
	}
	// all non-printable characters or invalid ASCII values have to be escaped
	char tmp[5];
	snprintf(tmp,5,"\\x%02x",byte);
	return std::string(tmp);
}

std::string uint8HexString(uint8_t byte){
	char tmp[3];
	snprintf(tmp,3,"%02X",byte);
	return std::string(tmp);
}

std::string ContentTableViewer::getTypeString(TypeGUID type){
	std::string out;
	switch(typeMode){
	case MODE_GUID:
		break;
	case MODE_HEXB:
		out = "";
		for(int i = 1; i >= 0; i--){
			out += uint8HexString((uint8_t)(type.data[i] >> 56)) + " ";	// byte 7
			out += uint8HexString((uint8_t)(type.data[i] >> 48)) + " ";	// byte 6
			out += uint8HexString((uint8_t)(type.data[i] >> 40)) + " ";	// byte 5
			out += uint8HexString((uint8_t)(type.data[i] >> 32)) + " ";	// byte 4
			out += uint8HexString((uint8_t)(type.data[i] >> 24)) + " ";	// byte 3
			out += uint8HexString((uint8_t)(type.data[i] >> 16)) + " ";	// byte 2
			out += uint8HexString((uint8_t)(type.data[i] >> 8)) + " ";	// byte 1
			out += uint8HexString((uint8_t)type.data[i]) + " ";	// byte 0
		}
		return out;
	case MODE_HEXL:
		out = "";
		for(int i = 0; i < 2; i++){
			out += uint8HexString((uint8_t)type.data[i]) + " ";	// byte 0
			out += uint8HexString((uint8_t)(type.data[i] >> 8)) + " ";	// byte 1
			out += uint8HexString((uint8_t)(type.data[i] >> 16)) + " ";	// byte 2
			out += uint8HexString((uint8_t)(type.data[i] >> 24)) + " ";	// byte 3
			out += uint8HexString((uint8_t)(type.data[i] >> 32)) + " ";	// byte 4
			out += uint8HexString((uint8_t)(type.data[i] >> 40)) + " ";	// byte 5
			out += uint8HexString((uint8_t)(type.data[i] >> 48)) + " ";	// byte 6
			out += uint8HexString((uint8_t)(type.data[i] >> 56)) + " ";	// byte 7
		}
		return out;
	case MODE_PYTHON:
		out = "b'";
		for(int i = 0; i < 2; i++){
			// the TypeGUID consists of two 64-bit integers to store all 16 bytes
			// since the shifting is different for each of the  8 bytes per int, looping over each byte would be a bit more complicated than necessary
			out += pythonByteLiteral((uint8_t)type.data[i]);	// byte 0
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 8));	// byte 1
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 16));	// byte 2
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 24));	// byte 3
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 32));	// byte 4
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 40));	// byte 5
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 48));	// byte 6
			out += pythonByteLiteral((uint8_t)(type.data[i] >> 56));	// byte 7

		}
		return out + "'";
	}
	return std::string("Internal Error!");
}

void ContentTableViewer::setItem(Item* item){
	this->item = item;
	deleteTree();
	if(item == nullptr){
		store->clear();
		printf("null\n");
		return;
	}
	generateTree(true);	//TODO: add this setting
	fillStore();
}

// these two functions are mostly identical, except that the non-recursive version creates top-level entries into the tree store
// combining them should be possible

void ContentTableViewer::fillStore(){
	store->clear();
	for(int i = 0; i < tree.size(); i++){
		// every top-level entry
		Gtk::TreeModel::iterator iter = store->append();
		iter->set_value(TypeColumn, getTypeString(item->contentTable.entries[tree[i]->idx].type));
		iter->set_value(TypeIDColumn, std::to_string(item->contentTable.entries[tree[i]->idx].type_id));
		iter->set_value(RefColumn, uint32ToHexString(item->contentTable.entries[tree[i]->idx].ref));
		if(item->contentTable.entries[tree[i]->idx].ref != 0xffffffff){
			iter->set_value(RefSizeColumn, uint32ToHexString(item->dataTable.entries[item->contentTable.entries[tree[i]->idx].ref].size));
		} else {
			iter->set_value(RefSizeColumn, std::string("-"));
		}
		iter->set_value(ParentColumn, uint32ToHexString(item->contentTable.entries[tree[i]->idx].parent));
		iter->set_value(FieldOffsetColumn, uint32ToHexString(item->contentTable.entries[tree[i]->idx].field_offset));
		iter->set_value(IndexColumn, tree[i]->idx);

		// fill in the children of this entry
		for(int c = 0; c < tree[i]->children.size(); c++){
			fillStoreRecursive(tree[i]->children[c], iter);
		}
	}
}

void ContentTableViewer::fillStoreRecursive(TreeEntry* entry, Gtk::TreeStore::iterator iter){
	// create the row for this entry
	Gtk::TreeModel::iterator this_it = store->append(iter->children());
	this_it->set_value(TypeColumn, getTypeString(item->contentTable.entries[entry->idx].type));
	this_it->set_value(TypeIDColumn, std::to_string(item->contentTable.entries[entry->idx].type_id));
	this_it->set_value(RefColumn, uint32ToHexString(item->contentTable.entries[entry->idx].ref));
	if(item->contentTable.entries[entry->idx].ref != 0xffffffff){
		this_it->set_value(RefSizeColumn, uint32ToHexString(item->dataTable.entries[item->contentTable.entries[entry->idx].ref].size));
	} else {
		this_it->set_value(RefSizeColumn, std::string("-"));
	}
	this_it->set_value(ParentColumn, uint32ToHexString(item->contentTable.entries[entry->idx].parent));
	this_it->set_value(FieldOffsetColumn, uint32ToHexString(item->contentTable.entries[entry->idx].field_offset));
	this_it->set_value(IndexColumn, entry->idx);

	// fill in the children, if there are any
	for(int i = 0; i < entry->children.size(); i++){
		fillStoreRecursive(entry->children[i], this_it);
	}
}

bool ContentTableViewer::findOffset(uint32_t off,Gtk::TreeStore::iterator iter){
	bool fromStart = StartOffsetCheckButton.get_active();
	for(auto it = iter->children().begin(); it != iter->children().end(); it++){
		int idx = it->get_value(IndexColumn);
		ContentTableEntry* cte = &item->contentTable.entries[idx];
		if(cte->ref == -1){
			continue;
		}
		DataTableEntry* dte = &item->dataTable.entries[cte->ref];
		uint32_t eoff = dte->offset;
		if(fromStart){
			eoff = item->getDataBlockOffset(dte);
		}
		uint32_t maxoff = eoff + dte->size;
		if(eoff <= off && off <= maxoff){
			//found it!
			Gtk::TreeModel::Row row = *it;
			Gtk::TreeModel::Path p = store->get_path(it);
			view->expand_to_path(p);
			view->scroll_to_row(p);
			view->get_selection()->unselect_all();
			view->get_selection()->select(row);
			return true;
		}
		if(findOffset(off, it)){
			return true;
		}

	}
	return false;
}

void ContentTableViewer::deleteTree(){
	for(int i = 0; i < tree.size(); i++){
		for(int c = 0; c < tree[i]->children.size(); c++){
			deleteTreeRecursive(tree[i]->children[c]);
		}
		delete tree[i];
	}
	tree.clear();	// clear the vector so that there are no invalid pointers
}

void ContentTableViewer::deleteTreeRecursive(TreeEntry* entry){
	for(int c = 0; c < entry->children.size(); c++){
		deleteTreeRecursive(entry->children[c]);
	}
	// all children are deleted now, so this entry can now be deleted as well
	delete entry;
}

void ContentTableViewer::generateTree(bool tree){
	if(!tree){
		// don't actually generate a tree, just create a list
		//TODO: add this
	}

	std::vector<TreeEntry*> tmpVec;
	// allocate some memory
	tmpVec.reserve(item->contentTable.entries.size());

	// create a TableEntry for every Entry
	for(int i = 0; i < item->contentTable.entries.size(); i++){
		// every entry
		TreeEntry* tmp = new TreeEntry;
		tmp->idx = i;
		tmpVec.emplace_back(tmp);
	}

	// iterate over the entries again, but this time, build the tree by looking at the parent index
	// this might be possible in one loop if parent entries are always defined before child entries
	// but it's safer this way without proper documentation on the format
	for(int i = 0; i < item->contentTable.entries.size(); i++){
		if(item->contentTable.entries[i].parent != 0xFFFFFFFF){
			// should have a valid parent
			tmpVec[i]->hasParent = true;
			// add this entry to the parents children vector
			tmpVec[item->contentTable.entries[i].parent]->children.emplace_back(tmpVec[i]);
		} else {
			tmpVec[i]->hasParent = false;
		}
	}

	// the tree is now built, but not really usable yet, as the top-level entries haven't been added to the final vector yet
	// to find them, iterate over everything a third time
	for(int i = 0; i < tmpVec.size(); i++){
		if(!tmpVec[i]->hasParent){
			// this entry didn't get added to any parent in the last loop, so it's a top level entry
			this->tree.emplace_back(tmpVec[i]);
		}
	}

}



