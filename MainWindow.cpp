#include "MainWindow.h"

MainWindow::MainWindow(){

	set_border_width(10);
	auto builder = Gtk::Builder::create_from_file("res/main.xml");

	Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("res/fileExplorer.css");

	FileList* filelist = new FileList(this,builder);
	FileEntry* entry1 = new FileEntry();
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
	filelist->updateFiles(*entries);
}
