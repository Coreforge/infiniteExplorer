#include "MainWindow.h"

#include "LogViewer.h"
#include "LogManager.h"
#include "ManagedLogger.h"
#include "FileViewerManager.h"


MainWindow::MainWindow(){

	set_border_width(10);
	auto builder = Gtk::Builder::create_from_file("res/main.xml");

	Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("res/fileExplorer.css");

	// set up the logging backend
	// don't set the TextBuffer yet, we'll do that once the LogViewer exists
	LogManager* logManager = new LogManager(15000,LOG_LEVEL_ERROR,nullptr);
	ManagedLogger* libInfiniteLogger = new ManagedLogger("[libInfinite]",logManager);

	ModuleManager* moduleManager = new ModuleManager;
	//Logger* logger = new ConsoleLogger;
	moduleManager->logger = (Logger*)libInfiniteLogger;


	// logging backend is done

	set_icon_from_file("res/icons/ie-512x512.png");

	accelGroup = Gtk::AccelGroup::create();
	add_accel_group(accelGroup);

	Gtk::Box* mainVBox = new Gtk::Box();	// this box contains two things. The menu bar, and a grid that contains everything else
	this->add(*mainVBox);
	mainVBox->set_property("orientation", Gtk::Orientation::ORIENTATION_VERTICAL);
	mainVBox->show();
	Gtk::MenuBar* menuBar = new Gtk::MenuBar();
	Gtk::MenuItem* fileMenuItem = new Gtk::MenuItem("File");
	mainVBox->add(*menuBar);
	menuBar->add(*fileMenuItem);
	menuBar->show();
	fileMenuItem->show();
	Gtk::Menu* fileMenu = new Gtk::Menu;
	Gtk::MenuItem* openModuleItem = new Gtk::MenuItem("Open Module");
	Gtk::MenuItem* openPathItem = new Gtk::MenuItem("Open Deploy Path");
	Gtk::MenuItem* ExportItem = new Gtk::MenuItem("Export");

	fileMenuItem->set_submenu(*fileMenu);
	fileMenu->add(*openModuleItem);
	fileMenu->add(*openPathItem);
	fileMenu->add(*ExportItem);
	fileMenuItem->show();
	fileMenu->show();
	ExportItem->show();
	ExportItem->set_accel_path("</Ctrl + E");
	ExportItem->add_accelerator("activate", accelGroup, GDK_KEY_E, Gdk::CONTROL_MASK, Gtk::AccelFlags::ACCEL_VISIBLE);
	ExportItem->signal_activate().connect([moduleManager] {moduleManager->exportEntryDialog();});

	openModuleItem->show();
	openModuleItem->signal_activate().connect([moduleManager] {moduleManager->openModuleDialog();});
	openModuleItem->add_accelerator("activate", accelGroup, GDK_KEY_O, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);

	openPathItem->show();
	openPathItem->signal_activate().connect([moduleManager] {moduleManager->openPathDialog();});

	// the menu bar is done, now set up the grid
	//Gtk::Grid* grid = new Gtk::Grid();
	//grid->show();

	// The Paned that contains the log output and another paned
	Gtk::Paned* mainPaned = new Gtk::Paned(Gtk::ORIENTATION_VERTICAL);
	mainPaned->show();
	// The Paned inside the main paned, containing the file list and whatever editor/viewer is open
	Gtk::Paned* contentPaned = new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL);
	contentPaned->show();
	mainVBox->add(*mainPaned);
	mainPaned->pack1(*contentPaned, true, false);


	// the FileList wants a container to be passed to it, and Gtk::Grid needs children added a bit differently
	// so I'm putting a box into the grid, which then gets passed to the FileList. This is a bit inefficient, but not too bad
	Gtk::Box* fileBox = new Gtk::Box();
	fileBox->show();
	contentPaned->pack1(*fileBox, false, false);
	//grid->attach(*fileBox, 0, 0, 1, 1);	// the file list should be on the left

	FileList* filelist = new FileList(fileBox,builder);
	/*FileEntry* entry1 = new FileEntry();
	FileEntry* entry2 = new FileEntry();
	FileEntry* entry3 = new FileEntry();
	FileEntry* entry4 = new FileEntry();
	std::vector<FileEntry*>* entries = new std::vector<FileEntry*>();
	entry1->name = "1";
	entry2->name = "Another File";
	entry3->name = "YATE";
	entry4->name = "ATFE";
	entries->emplace_back(entry1);
	entries->emplace_back(entry2);
	entries->emplace_back(entry3);
	entries->emplace_back(entry4);
	filelist->updateFiles(*entries);*/
	moduleManager->fileList = filelist;
	moduleManager->setupCallbacks();

	// file list is done, now set up the log front end


	LogViewer* logViewer = new LogViewer(logManager);
	mainPaned->pack2(*logViewer, false, true);
	//grid->attach(*logViewer, 0, 1, 1, 1);


	// set up the file viewer stuff

	FileViewerManager* fileViewerManager = new FileViewerManager();
	fileViewerManager->show();
	moduleManager->fileViewerManager = fileViewerManager;
	contentPaned->pack2(*fileViewerManager, true, false);

}
