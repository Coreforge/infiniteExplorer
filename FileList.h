#pragma once

#include <gtkmm.h>
#include <gtkmm/window.h>
#include <vector>

#include "FileEntry.h"

//define icons
#define FILE_TYPE_FILE_ICON "text-x-generic"
#define FILE_TYPE_DIRECTORY_ICON "folder"


class FileList{
public:
	FileList(Gtk::Container* window,Glib::RefPtr<Gtk::Builder> builder);

	// update the entries currently shown. The file entries in the vector will be copied, so they can be destroyed after this function finished
	void updateFiles(std::vector<FileEntry*> entries);
	void clearList();

	void onEntryDoubleClicked(Gtk::Button* button, GdkEventButton* event, FileEntry* entry);
	void onEntryClicked(Gtk::Button* button, FileEntry* entry);
	void onFrameClicked();
	void onKeyPressed(GdkEventKey* key);
	void onKeyReleased(GdkEventKey* key);

	Gtk::Label* currentPathLabel;
	std::vector<std::pair<FileEntry*,Gtk::Button*>> selectedEntries;
	std::vector<std::pair<FileEntry,Gtk::Button*>> shownEntries;
	Gtk::ScrolledWindow* listScroller;
	Gtk::ScrolledWindow* pathScroller;

private:
	Gtk::Container* parent;
	Gtk::Frame* frame;
	Gtk::Label* label;
	Gtk::Box* listBox;
	Gtk::Box* internalLayoutBox;
	Gtk::Box* controlBox;
	//Gtk::Button* goUpButton;

	void onResize();

	Gtk::EventBox* evtBox;
	Glib::RefPtr<Gtk::Builder> builder;


	Glib::RefPtr<Gtk::CssProvider> cssProvider;
	std::pair<FileEntry*,Gtk::Button*> activeEntry;

	bool shiftPressed;
	bool ctrlPressed;
};
