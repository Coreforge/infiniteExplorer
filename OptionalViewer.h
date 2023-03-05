#pragma once

#include <gtkmm.h>
#include "libInfinite/Item.h"

#include <string>

class OptionalViewer : public Gtk::Box{
public:
	virtual ~OptionalViewer() {};
	virtual void setItem(Item* item) = 0;
	virtual std::string getName() = 0;
};
