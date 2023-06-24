#pragma once

#include <gtkmm.h>
#include <string>
#include "OptionalViewer.h"

#include "VectorViewer.h"

#include "libInfinite/Item.h"
#include "libInfinite/tags/handles/vartHandle.h"
#include "libInfinite/tags/Tag.h"
#include "libInfinite/tags/TagManager.h"

class VartViewer : public OptionalViewer{

public:
	VartViewer();
	~VartViewer() override;
	void setItem(Item* item);

	std::string getName() { return std::string("Vector Viewer");};

private:

	void updateStuff();
	void updateMipmap();

	vartHandle* tag;

	Glib::RefPtr<Gtk::Builder> builder;

	// Pointers to widgets that are needed multiple times
	Gtk::SpinButton* frameNumberButton;
	Gtk::ComboBoxText* mipmapComboBox;
	Gtk::Label* dataTypeLabel;
	Gtk::Label* maxSizeLabel;
	Gtk::Label* currentSizeLabel;
	VectorViewer* viewerDrawingArea;

	Gtk::Button* exportSingleDDS;
	Gtk::Button* exportAllDDS;
	Gtk::Button* exportSinglePNG;
	Gtk::ComboBoxText* formatComboBox;

	std::string runExportDialog(std::string fileName);

	Item* item;



};
