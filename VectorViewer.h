#pragma once

#include <gtkmm.h>
#include "libInfinite/tags/handles/vartHandle.h"

class VectorViewer : public Gtk::DrawingArea{

public:
	VectorViewer(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~VectorViewer();
	vartHandle* handle;
	bool vectorValid;
	double scale;
	double x;
	double y;

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:
	double motionLastX;
	double motionLastY;

};
