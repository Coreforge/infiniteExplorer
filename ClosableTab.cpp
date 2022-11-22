#include "ClosableTab.h"

ClosableTab::ClosableTab(std::string name, std::string path, void* manager,void* ref, void (*closeCallback)(void*, std::string, void*)){
	this->name = name;
	this->path = path;
	this->man = manager;
	this->ref = ref;
	this->closeCallback = closeCallback;

	label = new Gtk::Label(name);
	set_tooltip_text(path);
	button = new Gtk::Button();
	button->set_image_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_MENU);
	button->set_relief(Gtk::RELIEF_NONE);

	button->signal_clicked().connect([this] {
		this->closeCallback(this->ref,this->path,this->man);
	});

	set_orientation(Gtk::ORIENTATION_HORIZONTAL);
	add(*label);
	add(*button);
	label->show();
	button->show();
}
