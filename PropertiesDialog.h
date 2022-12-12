#pragma once
#include <gtkmm.h>
#include "libInfinite/module/ModuleNode.h"


class PropertiesDialog : public Gtk::Dialog{
public:
	PropertiesDialog(void* node, void* manager);
};
