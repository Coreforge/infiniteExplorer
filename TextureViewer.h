#pragma once

#include <gtkmm.h>

class TextureViewer : public Gtk::DrawingArea{

public:
	TextureViewer(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~TextureViewer();
	int width;
	int height;
	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	bool pixbufValid;
	double scale;
	double x;
	double y;

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;

private:
	double motionLastX;
	double motionLastY;

};
