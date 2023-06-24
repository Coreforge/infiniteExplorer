#include <gtkmm/window.h>
#include <gtkmm.h>
#include <vector>
#include "libInfinite/logger/ConsoleLogger.h"

#include "libInfinite/StringIDLUT.h"

#include "FileList.h"
#include "ModuleDisplayManager.h"

#include "3D/interface/IEViewer.h"

class MainWindow : public Gtk::Window{
public:
	MainWindow();
	//~MainWindow();
	Glib::RefPtr<Gtk::Application> app;
	Glib::RefPtr<Gtk::AccelGroup> accelGroup;

	Gtk::MenuItem BatchExtractTexItem;
	Gtk::MenuItem viewer3DItem;

	ModuleDisplayManager* moduleManager;

	IEViewer viewer3D;
	StringIDLUT lut;
};
