#include "LogViewer.h"

LogViewer::LogViewer(LogManager* manager){
	frame = new Gtk::Frame();
	label = new Gtk::Label("Log output");
	textView = new Gtk::TextView();
	scrolledWindow = new Gtk::ScrolledWindow();

	frame->set_label("Log output");
	frame->set_label_align(0.5, 0.5);
	add(*frame);
	frame->add(*scrolledWindow);
	frame->set_size_request(150, 350);
	frame->show();

	scrolledWindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolledWindow->show();
	scrolledWindow->add(*textView);
	//scrolledWindow->set_vexpand(true);

	textView->show();
	show();
	//set_property("expand", true);
	textView->set_property("hexpand", true);
	textView->set_size_request(150, 350);
	textView->set_editable(false);


	this->manager = manager;
	// give the log manager the text buffer so the log can be displayed
	manager->setTextBuffer(textView->get_buffer().get());
	manager->setTextView(textView);
}

LogViewer::~LogViewer(){
	delete label;
	delete frame;
	delete textView;
}
