#include <gtkmm/window.h>
#include <gtkmm.h>
#include <vector>
#include "libInfinite/logger/ConsoleLogger.h"

#include "libInfinite/StringIDLUT.h"

#include "FileList.h"
#include "ModuleDisplayManager.h"

#include "3D/interface/IEViewer.h"
#include "3D/ExporterBase.h"

#include "3D/AssImpExporter.h"

class MainWindow : public Gtk::Window{
public:
	MainWindow();
	//~MainWindow();
	Glib::RefPtr<Gtk::Application> app;
	Glib::RefPtr<Gtk::AccelGroup> accelGroup;

	Gtk::MenuItem BatchExtractTexItem;
	Gtk::MenuItem viewer3DItem;
	Gtk::MenuItem newSceneItem;
	Gtk::MenuItem exportSceneItem;
	Gtk::MenuItem exportSceneBitmapsItem;

	ModuleDisplayManager* moduleManager;

	IEViewer viewer3D;
	StringIDLUT lut;
	ExporterBase* currentExporter;

private:
	AssImpExporter assimpExporterIF;
};
