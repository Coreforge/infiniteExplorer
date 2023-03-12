#pragma once

#include <gtkmm.h>
#include <gtkmm/window.h>
#include <vector>

#include "FileEntry.h"
#include "PropertiesDialog.h"

//define icons
#define FILE_TYPE_FILE_ICON "text-x-generic"
#define FILE_TYPE_DIRECTORY_ICON "folder"


class FileList{
public:
	FileList(Gtk::Container* window,Glib::RefPtr<Gtk::Builder> builder);

	// update the entries currently shown. The file entries in the vector will be copied, so they can be destroyed after this function finished
	void updateFiles(std::vector<FileEntry*> entries);
	void clearList();

	//void onEntryDoubleClicked(Gtk::Button* button, GdkEventButton* event, FileEntry* entry);
	//void onEntryClicked(Gtk::Button* button, FileEntry* entry);
	void onFrameClicked();
	void onKeyPressed(GdkEventKey* key);
	void onKeyReleased(GdkEventKey* key);
	void setSearchCallback(void (*onSearch)(void*,std::string), void* manager);
	void onSearch();
	std::vector<FileEntry*> getSelectedEntries();

	Gtk::Label* currentPathLabel;
	std::vector<std::pair<FileEntry*,Gtk::Button*>> selectedEntries;
	//std::vector<std::pair<FileEntry,Gtk::Button*>> shownEntries;
	Gtk::ScrolledWindow* listScroller;
	Gtk::ScrolledWindow* pathScroller;

private:
	Gtk::Container* parent;
	Gtk::Frame* frame;
	Gtk::Label* label;
	Gtk::Box* listBox;
	Gtk::Box* internalLayoutBox;
	Gtk::Box* controlBox;
	Gtk::SearchEntry* searchBar;

	// context menu stuff
	Gtk::Menu* contextMenu;
	Gtk::MenuItem* propertiesMenuItem;
	//Gtk::Button* goUpButton;


	//properties dialog
	std::unique_ptr<PropertiesDialog> propDialog;


	void onResize();

	void setupModelStuff();

	Gtk::EventBox* evtBox;
	Glib::RefPtr<Gtk::Builder> builder;


	Glib::RefPtr<Gtk::CssProvider> cssProvider;
	std::pair<FileEntry*,Gtk::Button*> activeEntry;


	// TreeView Stuff
	Gtk::TreeModel::ColumnRecord cRecord;
	Gtk::TreeViewColumn mainViewColumn;
	Gtk::TreeModelColumn<std::string> iconColumn;
	Gtk::TreeModelColumn<std::string> nameColumn;
	Gtk::TreeModelColumn<FileEntry*> entryColumn;
	Glib::RefPtr<Gtk::ListStore> store;
	Gtk::TreeView treeView;
	Gtk::CellRendererPixbuf crPixbuf;
	Gtk::CellRendererText crText;

	void (*onSearchCallback)(void*,std::string);
	void* manager;
	void* propNode;
	bool shiftPressed;
	bool ctrlPressed;
};
