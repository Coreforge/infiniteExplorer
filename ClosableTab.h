#pragma once

#include <gtkmm.h>
#include <string>

class ClosableTab : public Gtk::Box{
public:
	ClosableTab(std::string name, std::string path, void* manager,void (*closeCallback)(std::string, void*));

private:
	std::string name;
	std::string path;	// also used to identify which tab should be closed
	void* man; 		// just passed to the close function
	Gtk::Label* label;
	Gtk::Button* button;
	void (*closeCallback)(std::string, void*);

};
