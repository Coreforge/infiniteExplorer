#include <gtkmm.h>
#include <gtkmm/window.h>
#include <vector>

#include "FileEntry.h"

class FileList{
public:
	FileList(Gtk::Window* window,Glib::RefPtr<Gtk::Builder> builder);

	// update the entries currently shown. The file entries in the vector will be copied, so they can be destroyed after this function finished
	void updateFiles(std::vector<FileEntry*> entries);

	void onEntryDoubleClicked(Gtk::Button* button, GdkEventButton* event, FileEntry* entry);
	void onEntryClicked(Gtk::Button* button, FileEntry* entry);
	void onFrameClicked();
	void onKeyPressed(GdkEventKey* key);
	void onKeyReleased(GdkEventKey* key);

private:
	Gtk::Window* parent;
	Gtk::Frame* frame;
	Gtk::Label* label;
	Gtk::Box* listBox;
	Gtk::EventBox* evtBox;
	Glib::RefPtr<Gtk::Builder> builder;
	std::vector<std::pair<FileEntry,Gtk::Button*>> shownEntries;
	std::vector<std::pair<FileEntry*,Gtk::Button*>> selectedEntries;
	Glib::RefPtr<Gtk::CssProvider> cssProvider;
	std::pair<FileEntry*,Gtk::Button*> activeEntry;

	bool shiftPressed;
	bool ctrlPressed;
};
