#pragma once

#include <stdint.h>
#include <gtkmm.h>

class LogManager{
private:

	// the log gets stored in a ring buffer
	// currently, no locking is done as nothing is threaded yet. It will likely be needed later though
	char* buffer;	// the buffer the log gets stored in
	uint32_t max;	// the maximum offset in the buffer
	uint32_t start;	// the offset at which the earliest full log message starts in the buffer
	uint32_t wptr;	// wptr and rptr for the ring buffer
	uint32_t rptr;
	int consoleOutput;	// from which log level upwards the log messages should also get printed to the console
	Gtk::TextBuffer* textBuffer;
	Gtk::TextView* view;	// needed to enable auto scrolling

	// the text tags to format the different log levels
	// this might be better placed in LogViewer, but that mostly just contains the widgets, while the LogManager does everything else
	Glib::RefPtr<Gtk::TextTag> debugTag;
	Glib::RefPtr<Gtk::TextTag> infoTag;
	Glib::RefPtr<Gtk::TextTag> warningTag;
	Glib::RefPtr<Gtk::TextTag> errorTag;
	Glib::RefPtr<Gtk::TextTag> criticalTag;
	void createTags();
	Glib::RefPtr<Gtk::TextTag> getTag(int level);

public:
	LogManager(uint32_t size, int consoleOutputLevel, Gtk::TextBuffer* textBuffer);	// the size of the buffer
	~LogManager();
	void submitLog(const char* buf, uint32_t len);	// expects a null-terminated string
	void updateBuffer();	// writes everything that hasn't been read yet from the ring buffer into the text buffer and updates the rptr
	void setTextBuffer(Gtk::TextBuffer* textBuffer);
	void setTextView(Gtk::TextView* view);

};
