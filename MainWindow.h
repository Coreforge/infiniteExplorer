#include <gtkmm/window.h>
#include <gtkmm.h>
#include <vector>
#include "ModuleManager.h"

#include "FileList.h"

class MainWindow : public Gtk::Window{
public:
	MainWindow();
	//~MainWindow();
	Glib::RefPtr<Gtk::Application> app;
	Glib::RefPtr<Gtk::AccelGroup> accelGroup;
};
