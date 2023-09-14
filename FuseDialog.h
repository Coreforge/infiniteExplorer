#pragma once
#include <gtkmm.h>

#include "FuseProvider.h"

class FuseDialog : public Gtk::Dialog{
public:
	FuseDialog(FuseProvider* provider);
	void applySettings();

private:
	FuseProvider* provider;

	Gtk::Grid grid;

	Gtk::Label mountpointLabel;
	Gtk::Entry mountPointEntry;
	//Gtk::FileChooserDialog mountPointDialog;
	Gtk::CheckButton allowOtherCheck;
	Gtk::CheckButton exposeDDS;
	Gtk::CheckButton exposePNG;

	Gtk::Label optionsEntryLabel;
	Gtk::Entry optionsEntry;

};
