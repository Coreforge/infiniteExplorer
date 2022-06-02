#include "PropertiesDialog.h"

#include "ModuleManager.h"
#include <string>

PropertiesDialog::PropertiesDialog(void* node, void* manager){
	ModuleNode* nodeRef = (ModuleNode*) node;
	ModuleManager* mgr = (ModuleManager*) manager;

	// get the sizes
	auto sizes = mgr->getSizes(nodeRef);
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
	r++;

	// using if since a folder may be in multiple modules and I'm too lazy to check that, so I'm just not displaying it for now
	if(nodeRef->type != NODE_TYPE_DIRECTORY){
		// Module path
		Gtk::Label* moduleLabel = Gtk::make_managed<Gtk::Label>("Module: ");
		Gtk::Label* moduleValueLabel = Gtk::make_managed<Gtk::Label>(nodeRef->item->module->name);
		mainGrid->attach(*moduleLabel, 0, r);
		mainGrid->attach(*moduleValueLabel, 1, r);
		moduleLabel->set_halign(Gtk::ALIGN_START);
		moduleValueLabel->set_halign(Gtk::ALIGN_START);
		moduleLabel->show();
		moduleValueLabel->show();
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
