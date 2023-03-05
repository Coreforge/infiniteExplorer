#include <ModuleDisplayManager.h>
#include "PropertiesDialog.h"

#include <string>

PropertiesDialog::PropertiesDialog(void* node, void* manager){
	ModuleNode* nodeRef = (ModuleNode*) node;
	ModuleDisplayManager* mgr = (ModuleDisplayManager*) manager;

	// get the sizes
	auto sizes = mgr->modMan.getSizes(nodeRef);
	Glib::ustring decompSize = Glib::format_size(sizes.first);
	Glib::ustring compSize = Glib::format_size(sizes.second);

	set_title("Properties of " + nodeRef->name);
	set_size_request(400, 250);

	int r = 0;

	// fill the contents

	Gtk::Grid* mainGrid = Gtk::make_managed<Gtk::Grid>();
	mainGrid->set_halign(Gtk::ALIGN_CENTER);
	mainGrid->set_margin_top(20);
	mainGrid->set_row_spacing(10);
	mainGrid->set_column_spacing(10);
	mainGrid->set_baseline_row(0);

	// Item name
	Gtk::Label* nameLabel = Gtk::make_managed<Gtk::Label>("Name: ");
	Gtk::Label* nameValueLabel = Gtk::make_managed<Gtk::Label>(nodeRef->name);
	mainGrid->attach(*nameLabel, 0, r);
	mainGrid->attach(*nameValueLabel, 1, r);
	nameLabel->set_halign(Gtk::ALIGN_START);
	nameValueLabel->set_halign(Gtk::ALIGN_START);
	nameLabel->show();
	nameValueLabel->show();
	nameValueLabel->set_selectable(true);
	r++;

	// Item name
	Gtk::Label* pathLabel = Gtk::make_managed<Gtk::Label>("Full Path: ");
	Gtk::Label* pathValueLabel = Gtk::make_managed<Gtk::Label>(nodeRef->path);
	mainGrid->attach(*pathLabel, 0, r);
	mainGrid->attach(*pathValueLabel, 1, r);
	pathLabel->set_halign(Gtk::ALIGN_START);
	pathValueLabel->set_halign(Gtk::ALIGN_START);
	pathLabel->show();
	pathValueLabel->show();
	pathValueLabel->set_selectable(true);
	r++;

	// using if since a folder may be in multiple modules and I'm too lazy to check that, so I'm just not displaying it for now
	if(nodeRef->type != NODE_TYPE_DIRECTORY){
		// Module path
		Gtk::Label* moduleLabel = Gtk::make_managed<Gtk::Label>("Module: ");
		Gtk::Label* moduleValueLabel = Gtk::make_managed<Gtk::Label>(nodeRef->item->module->path);
		mainGrid->attach(*moduleLabel, 0, r);
		mainGrid->attach(*moduleValueLabel, 1, r);
		moduleLabel->set_halign(Gtk::ALIGN_START);
		moduleValueLabel->set_halign(Gtk::ALIGN_START);
		moduleLabel->show();
		moduleValueLabel->show();
		moduleValueLabel->set_selectable(true);
		r++;
	}
	if(nodeRef->type == NODE_TYPE_FILE){

		// Tag class
		Gtk::Label* tagTypeLabel = Gtk::make_managed<Gtk::Label>("Tag class: ");
		std::string tagType((char*)&nodeRef->item->tagType);
		std::reverse(tagType.begin(),tagType.end());
		Gtk::Label* tagTypeValueLabel = Gtk::make_managed<Gtk::Label>(tagType);
		mainGrid->attach(*tagTypeLabel, 0, r);
		mainGrid->attach(*tagTypeValueLabel, 1, r);
		tagTypeLabel->set_halign(Gtk::ALIGN_START);
		tagTypeValueLabel->set_halign(Gtk::ALIGN_START);
		tagTypeLabel->show();
		tagTypeValueLabel->show();
		r++;

		// Parent item
		Gtk::Label* parentLabel = Gtk::make_managed<Gtk::Label>("Parent: ");
		std::string parentString;
		if(nodeRef->item->parent == nullptr){
			parentString = "None";
		} else {
			parentString = nodeRef->item->parent->path;
		}
		Gtk::Label* parentValueLabel = Gtk::make_managed<Gtk::Label>(parentString);
		mainGrid->attach(*parentLabel, 0, r);
		mainGrid->attach(*parentValueLabel, 1, r);
		parentLabel->set_halign(Gtk::ALIGN_START);
		parentValueLabel->set_halign(Gtk::ALIGN_START);
		parentLabel->show();
		parentValueLabel->show();
		r++;

		// Resource count
		Gtk::Label* resourceCountLabel = Gtk::make_managed<Gtk::Label>("Resource count: ");
		Gtk::Label* resourceCountValueLabel = Gtk::make_managed<Gtk::Label>(std::to_string(nodeRef->item->resources.size()));
		mainGrid->attach(*resourceCountLabel, 0, r);
		mainGrid->attach(*resourceCountValueLabel, 1, r);
		resourceCountLabel->set_halign(Gtk::ALIGN_START);
		resourceCountValueLabel->set_halign(Gtk::ALIGN_START);
		resourceCountLabel->show();
		resourceCountValueLabel->show();
		r++;
	}

	// Decompressed size
	Gtk::Label* decompLabel = Gtk::make_managed<Gtk::Label>("Decompressed Size: ");
	Gtk::Label* decompValueLabel = Gtk::make_managed<Gtk::Label>(decompSize + " (" + std::to_string(sizes.first) + " Bytes)");
	mainGrid->attach(*decompLabel, 0, r);
	mainGrid->attach(*decompValueLabel, 1, r);
	decompLabel->set_halign(Gtk::ALIGN_START);
	decompValueLabel->set_halign(Gtk::ALIGN_START);
	decompLabel->show();
	decompValueLabel->show();
	r++;

	// Compressed size
	Gtk::Label* compLabel = Gtk::make_managed<Gtk::Label>("Compressed Size: ");
	Gtk::Label* compValueLabel = Gtk::make_managed<Gtk::Label>(compSize + " (" + std::to_string(sizes.second) + " Bytes)");
	mainGrid->attach(*compLabel, 0, r);
	mainGrid->attach(*compValueLabel, 1, r);
	compLabel->set_halign(Gtk::ALIGN_START);
	compValueLabel->set_halign(Gtk::ALIGN_START);
	compLabel->show();
	compValueLabel->show();
	r++;

	((Gtk::Box*)get_child())->add(*mainGrid);

	mainGrid->show();
	show();
}
