#pragma once

#include <gtkmm.h>
#include <string>
#include "OptionalViewer.h"

#include "TextureViewer.h"

#include "libInfinite/Item.h"
#include "libInfinite/BitmapHandle.h"

class BitmapViewer : public OptionalViewer{

public:
	BitmapViewer();
	~BitmapViewer() override;
	void setItem(Item* item);

	std::string getName() { return std::string("Bitmap Viewer");};

private:

	void updateStuff();
	void updateMipmap();

	BitmapHandle* handle;

	Glib::RefPtr<Gtk::Builder> builder;

	// Pointers to widgets that are needed multiple times
	Gtk::SpinButton* frameNumberButton;
	Gtk::ComboBoxText* mipmapComboBox;
	Gtk::Label* dataTypeLabel;
	Gtk::Label* maxSizeLabel;
	Gtk::Label* currentSizeLabel;
	TextureViewer* viewerDrawingArea;

	Gtk::Button* exportSingleDDS;
	Gtk::Button* exportAllDDS;
	Gtk::Button* exportSinglePNG;
	Gtk::ComboBoxText* formatComboBox;

	std::string runExportDialog(std::string fileName);

	Item* item;



};
