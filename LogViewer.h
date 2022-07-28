#pragma once

#include <gtkmm.h>

#include "LogManager.h"

class LogViewer : public Gtk::Box{
private:
	Gtk::Frame* frame;
	Gtk::Label* label;
	Gtk::TextView* textView;
	Gtk::ScrolledWindow* scrolledWindow;
	LogManager* manager;

public:
	LogViewer(LogManager* manager);
	~LogViewer();
};
