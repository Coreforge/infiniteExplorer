#pragma once

#include <gtkmm.h>

#include "OptionalViewer.h"

#include "libInfinite/tags/Tag.h"
#include "libInfinite/tags/handles/sbspHandle.h"
#include "libInfinite/tags/TagManager.h"

class BspViewer : public OptionalViewer{
public:
	BspViewer();
	~BspViewer();

	void setItem(Item* item);

	std::string getName() { return std::string("BSP controls");};



private:
	Glib::RefPtr<Gtk::Builder> builder;
	Item* item;
	sbspHandle* tag;
};
