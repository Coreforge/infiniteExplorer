#include "MainWindow.h"

MainWindow::MainWindow(){

	set_border_width(10);
	auto builder = Gtk::Builder::create_from_file("res/main.xml");

	Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("res/fileExplorer.css");

	ModuleManager* moduleManager = new ModuleManager;


	accelGroup = Gtk::AccelGroup::create();
	add_accel_group(accelGroup);

	Gtk::Box* mainVBox = new Gtk::Box();
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


	FileList* filelist = new FileList(mainVBox,builder);
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
}
