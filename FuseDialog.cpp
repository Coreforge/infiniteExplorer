#include "FuseDialog.h"

#ifdef _WIN64
#define MOUNTPOINT_LABEL "Mountpoint or Drive Letter:"
#else
#define MOUNTPOINT_LABEL "Mountpoint:"
#endif

FuseDialog::FuseDialog(FuseProvider* provider){
	this->provider = provider;
	((Gtk::Box*)get_child())->add(grid);

	set_size_request(400, 200);
	set_title("FUSE mount");

	grid.set_halign(Gtk::ALIGN_CENTER);
	grid.set_margin_top(20);
	grid.set_margin_left(10);
	grid.set_margin_right(10);
	grid.set_row_spacing(10);
	grid.set_column_spacing(10);
	grid.set_baseline_row(0);
	grid.show();



	int row = 0;
	mountpointLabel.set_label(MOUNTPOINT_LABEL);
	mountpointLabel.show();
	grid.attach(mountpointLabel, 0, row, 1, 1);
	mountPointEntry.set_icon_from_icon_name("folder", Gtk::ENTRY_ICON_SECONDARY);
	mountPointEntry.show();
	mountPointEntry.set_width_chars(60);
	mountPointEntry.signal_icon_press().connect([this] (Gtk::EntryIconPosition pos, const _GdkEventButton* button){
		Gtk::FileChooserDialog chooser("Mount at", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		chooser.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
		chooser.add_button("_Open", Gtk::RESPONSE_OK);
		int response = chooser.run();
		chooser.close();
		if(response != Gtk::RESPONSE_OK){
			return;
		}

		mountPointEntry.set_text(chooser.get_filename());
	});
	grid.attach(mountPointEntry, 1, row, 1, 1);

	add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	add_button("Mount",Gtk::RESPONSE_OK);

	row++;

	allowOtherCheck.set_label("allow_other");
	allowOtherCheck.show();
	grid.attach(allowOtherCheck, 0, row, 2, 1);
	row++;


	optionsEntryLabel.set_label("other options:");
	optionsEntryLabel.set_halign(Gtk::ALIGN_END);
	optionsEntryLabel.show();
	grid.attach(optionsEntryLabel, 0, row, 1, 1);
	optionsEntry.set_width_chars(60);
	optionsEntry.show();
	grid.attach(optionsEntry, 1, row, 1, 1);
	row++;

	exposeDDS.set_label("Bitmaps as DDS");
	exposeDDS.show();
	grid.attach(exposeDDS, 0, row, 2, 1);
	row++;
	exposePNG.set_label("Bitmaps as PNG");
	exposePNG.show();
	grid.attach(exposePNG, 0, row, 2, 1);

}


void FuseDialog::applySettings(){
	std::string options;
	if(allowOtherCheck.get_active()){
		options += "allow_other";
	}
	if(allowOtherCheck.get_active() && optionsEntry.get_text().length() > 0){
		options += ",";
	}
	if(optionsEntry.get_text().length() > 0){
		options += optionsEntry.get_text();
	}
	provider->setup(mountPointEntry.get_text(),options);
	provider->exposeBitmapsDDS = exposeDDS.get_active();
	provider->exposeBitmapsPNG = exposePNG.get_active();
}
