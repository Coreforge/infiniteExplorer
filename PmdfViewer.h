#pragma once

#include <gtkmm.h>

#include "OptionalViewer.h"

#include "libInfinite/tags/Tag.h"
#include "libInfinite/tags/handles/pmdfHandle.h"
#include "libInfinite/tags/TagManager.h"

class PmdfViewer : public OptionalViewer{
public:
	PmdfViewer();
	~PmdfViewer();

	void setItem(Item* item);

	std::string getName() { return std::string("PMDF controls");};



private:
	Glib::RefPtr<Gtk::Builder> builder;
	Item* item;
	pmdfHandle* tag;
};
