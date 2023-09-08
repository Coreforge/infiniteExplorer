#include "MainWindow.h"

#include "LogViewer.h"
#include "LogManager.h"
#include "ManagedLogger.h"
#include "FileViewerManager.h"

#include <fstream>

#ifdef USE_FUSE
#include "FuseProvider.h"
#include "FuseDialog.h"
#endif

MainWindow* globalWindowPointer;

MainWindow::MainWindow(){

	set_border_width(10);
	auto builder = Gtk::Builder::create_from_file("res/main.xml");

	Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("res/fileExplorer.css");

	// set up the logging backend
	// don't set the TextBuffer yet, we'll do that once the LogViewer exists
	LogManager* logManager = new LogManager(0x1000000,LOG_LEVEL_ERROR,nullptr);
	ManagedLogger* libInfiniteLogger = new ManagedLogger("[libInfinite]",logManager);

	ManagedLogger* fuseLogger = new ManagedLogger("[FUSE]",logManager);

	moduleManager = new ModuleDisplayManager((Logger*)libInfiniteLogger);
	//Logger* logger = new ConsoleLogger;
	//moduleManager->logger = (Logger*)libInfiniteLogger;


	// logging backend is done

	// load the known hashes file now, because why not
	lut.setLogger(libInfiniteLogger);
	std::ifstream hashLUT("res/hashes.txt");
	if(hashLUT.is_open()){
		std::string lutData;
		std::stringstream stream;
		stream << hashLUT.rdbuf();
		//hashLUT >> lutData;
		lut.loadMap(stream.str());
	} else {
		libInfiniteLogger->log(LOG_LEVEL_ERROR, "Failed to open hash lookup table\n");
	}


	// set up the basic FUSE stuff
#ifdef USE_FUSE
	FuseProvider* fuseProvider = new FuseProvider(fuseLogger,&moduleManager->modMan);
	FuseDialog* fuseDialog = new FuseDialog(fuseProvider);
#endif

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

	// change this once there are more tools!
	Gtk::MenuItem* toolsMenuItem = new Gtk::MenuItem("Tools");
	menuBar->add(*toolsMenuItem);
	toolsMenuItem->show();


	Gtk::Menu* fileMenu = new Gtk::Menu;
	Gtk::MenuItem* openModuleItem = new Gtk::MenuItem("Open Module");
	Gtk::MenuItem* openPathItem = new Gtk::MenuItem("Open Deploy Path");
	Gtk::MenuItem* openFileItem = new Gtk::MenuItem("Load File");
	Gtk::MenuItem* ExportItem = new Gtk::MenuItem("Export");

	fileMenuItem->set_submenu(*fileMenu);
	fileMenu->add(*openModuleItem);
	fileMenu->add(*openPathItem);
	fileMenu->add(*openFileItem);
	fileMenu->add(*ExportItem);
	fileMenuItem->show();
	fileMenu->show();

	// Tools menu
	Gtk::Menu* toolsMenu = new Gtk::Menu();
	toolsMenu->show();
	toolsMenuItem->set_submenu(*toolsMenu);

	ExportItem->show();
	ExportItem->set_accel_path("</Ctrl + E");
	ExportItem->add_accelerator("activate", accelGroup, GDK_KEY_E, Gdk::CONTROL_MASK, Gtk::AccelFlags::ACCEL_VISIBLE);
	ExportItem->signal_activate().connect([this] {moduleManager->exportEntryDialog();});
	BatchExtractTexItem.set_label("Batch Extract Textures");
	toolsMenu->add(BatchExtractTexItem);
	BatchExtractTexItem.show();
	BatchExtractTexItem.signal_activate().connect([this]{
		moduleManager->batchExtractTextures();
	});

	viewer3DItem.set_label("Open 3D Viewer");
	viewer3DItem.show();
	toolsMenu->add(viewer3DItem);
	viewer3DItem.signal_activate().connect([this]{
		viewer3D.createWindow(IEViewer::IEV_GLFW, 1920, 1080);
	});


	openModuleItem->show();
	openModuleItem->signal_activate().connect([this] {moduleManager->openModuleDialog();});
	openModuleItem->add_accelerator("activate", accelGroup, GDK_KEY_O, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);

	openPathItem->show();
	openPathItem->signal_activate().connect([this] {moduleManager->openPathDialog();});

	openFileItem->show();
	openFileItem->signal_activate().connect([this]{
		moduleManager->loadFileDialog();
	});

#ifdef USE_FUSE

	// FUSE stuff
	Gtk::MenuItem* fuseMenuItem = new Gtk::MenuItem("Mount/Unmount");
	toolsMenu->add(*fuseMenuItem);
	fuseMenuItem->signal_activate().connect([fuseProvider,fuseDialog,fuseMenuItem]{

		if(fuseProvider->mounted){
			// already mounted, so this is now the unmount option
			fuseProvider->unmount();

			// assuming unmounting was successful
			//fuseProvider->mounted = false;
			//fuseMenuItem->set_label("Mount");

		} else {
			// not mounted, try to mount
			int response = fuseDialog->run();
			fuseDialog->close();
			if(response != Gtk::RESPONSE_OK){
				return;
			}
			fuseDialog->applySettings();

			// I guess try to mount now!
			fuseProvider->mount();

			// assuming mounting was successful
			//fuseProvider->mounted = true;
			//fuseMenuItem->set_label("Unmount");
		}
	});
	fuseMenuItem->show();
#endif




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



	// Quit stuff
#ifdef USE_FUSE
	// if FUSE is still active when quitting, display a warning
	signal_delete_event().connect([this,fuseProvider](GdkEventAny* event) -> bool{
		if(!fuseProvider->mounted){
			// just exit, nothing is mounted
			return false;
		}
		Gtk::Dialog exitDialog("Quit");
		exitDialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
		exitDialog.add_button("_Quit", Gtk::RESPONSE_YES);
		Gtk::Label lab("You still have the modules mounted.\nQuitting will stop the fuse client and unmount them.\nAre you sure?");
		lab.set_line_wrap_mode(Pango::WRAP_WORD);
		lab.set_justify(Gtk::JUSTIFY_CENTER);
		lab.set_margin_top(20);
		lab.set_margin_left(10);
		lab.set_margin_right(10);
		lab.set_margin_bottom(10);
		lab.show();
		Gtk::Image icn("dialog-warning", Gtk::ICON_SIZE_DIALOG);
		icn.set_margin_top(20);
		icn.show();
		((Gtk::Box*)exitDialog.get_child())->add(icn);
		((Gtk::Box*)exitDialog.get_child())->add(lab);

		int r = exitDialog.run();
		exitDialog.close();
		if(r != Gtk::RESPONSE_YES){
			// don't exit
			return true;
		}

		fuseProvider->unmount();
		return false;
	});
#endif

	// set the global pointer to this instance
	globalWindowPointer = this;

}




