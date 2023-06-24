#include "VectorViewer.h"

#define SCROLL_SCALE_FACTOR 1.1

VectorViewer::VectorViewer(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Glib::ObjectBase("TextureViewer"),
	  Gtk::DrawingArea(cobject){
	vectorValid = false;
	handle = nullptr;
	scale = 100.0f;
	x = 0;
	y = 0;
	motionLastX = 0;
	motionLastY = 0;
	add_events(Gdk::EventMask::SCROLL_MASK);
	add_events(Gdk::EventMask::BUTTON_MOTION_MASK);
	add_events(Gdk::EventMask::BUTTON_PRESS_MASK);
	add_events(Gdk::EventMask::BUTTON_RELEASE_MASK);
	signal_button_press_event().connect([this] (GdkEventButton* btn) -> bool{
		motionLastX = btn->x;
		motionLastY = btn->y;
		return false;
	});
	signal_button_release_event().connect([this] (GdkEventButton* btn) -> bool{
		return false;
	});
	signal_motion_notify_event().connect([this] (GdkEventMotion* motion) -> bool{

		if(motion->x - motionLastX != 0){
			x += (motion->x - motionLastX) / scale;
		}
		if(motion->y - motionLastY != 0){
			y += (motion->y - motionLastY) / scale;
		}
		motionLastX = motion->x;
		motionLastY = motion->y;

		queue_draw();
		return false;
	});
	signal_scroll_event().connect([this] (GdkEventScroll* scroll) -> bool{
		double scaleOld = scale;
		if(scroll->direction == GdkScrollDirection::GDK_SCROLL_UP){
			scale *= SCROLL_SCALE_FACTOR;
		}

		if(scroll->direction == GdkScrollDirection::GDK_SCROLL_DOWN){
			if(scale > 0.1){
				scale *= (1.0/SCROLL_SCALE_FACTOR);
			}
		}

		double dX = (scroll->x * (scale / scaleOld)) - scroll->x;	// new position - old position
		double dY = (scroll->y * (scale / scaleOld)) - scroll->y;
		x -= dX / scaleOld;
		y -= dY / scaleOld;

		queue_draw();
		// looks like this isn't happening for me, not sure about other systems
		/*if(scroll->direction == GDK_SCROLL_SMOOTH){
			double dx = scroll->delta_x;
			double dy = scroll->delta_y;
		}*/
		return false;
	});
}

VectorViewer::~VectorViewer(){

}

void useRegionColour(uint32_t region, const Cairo::RefPtr<Cairo::Context>& cr){
	switch(region){
	case 0:
		cr->set_source_rgb(0, 0, 0);
		break;
	case 1:
		cr->set_source_rgb(1, 0, 0);
		break;
	case 2:
		cr->set_source_rgb(0, 1, 0);
		break;
	case 3:
		cr->set_source_rgb(0, 0, 1);
		break;
	case 4:
		cr->set_source_rgb(1, 1, 0);
		break;
	case 5:
		cr->set_source_rgb(1, 0, 1);
		break;
	case 6:
		cr->set_source_rgb(0, 1, 1);
		break;
	default:
		printf("Unknown region %d\n",region);
		cr->set_source_rgb(1, 1, 1);
		break;
	}
}

bool VectorViewer::on_draw(const Cairo::RefPtr<Cairo::Context>& cr){

	int totalWidth = get_allocated_width();
	int totalHeight = get_allocated_height();

	int scale = Pango::SCALE;

	if(vectorValid && handle != nullptr){

		// check x and y to prevent the image being shoved outside the viewport completely
		float aspect = handle->getAspectRatio();
		std::vector<vartHandle::vertex> vertices = handle->getVertices();
		std::vector<uint16_t> indicies = handle->getIndicies();

		//double maxX = (this->scale * aspect) / totalWidth;
		//double maxY = this->scale / totalHeight;
		double maxX = (totalWidth / this->scale) - 1 * aspect;
		double maxY = totalHeight / this->scale  - 1;
		double minX = 0;
		double minY = 0;


		if(this->scale * aspect < totalWidth){
			if(x > maxX) x = maxX;
			if(x < minX) x = minX;
		} else {
			if(x < maxX) x = maxX;
			if(x > minX) x = minX;
		}

		if(this->scale < totalHeight){
			if(y > maxY) y = maxY;
			if(y < minY) y = minY;
		} else {
			if(y < maxY) y = maxY;
			if(y > minY) y = minY;
		}



		cr->scale(this->scale * aspect, this->scale);
		cr->translate(x / aspect, y);
		//Gdk::Cairo::set_source_pixbuf(cr, pixbuf, x, y);
		cr->set_source_rgb(0, 1, 0);
		cr->set_line_width(3);
		//cr->move_to(0, 0);
		//cr->line_to(200, 200);
		//cr->stroke();

		for(int i = 0; i < handle->getDrawInfoCount(); i++){
			auto info = handle->getDrawInfo(i);
			auto firstvert = vertices[indicies[info.indexStart]];
			//cr->move_to(firstvert.x * scale, firstvert.y * scale);

			// these are used to close the triangles
			float startx = firstvert.x;
			float starty = firstvert.y;

			for(int ind = info.indexStart; ind < info.indexCount + info.indexStart; ind++){
				auto vert = vertices[indicies[ind] + info.vertexStart];
				if(ind % 3 != 0){
					cr->line_to(vert.x, vert.y);
				} else {
					//cr->line_to(startx * this->scale, starty * this->scale);
					cr->close_path();
					cr->move_to(vert.x, vert.y);
					useRegionColour(vert.region, cr);
					startx = vert.x;
					starty = vert.y;
				}
				//actuallyDraw = !actuallyDraw;
			}
			//cr->stroke();
			cr->fill();
		}
		//cr->paint();
	} else {
		// Error message
		Pango::FontDescription h1font;
		//h1font.set_family("Monospace");
		h1font.set_weight(Pango::WEIGHT_BOLD);
		h1font.set_absolute_size(30000);
		auto mainMessageLayout = create_pango_layout("No Texture to show!");
		mainMessageLayout->set_font_description(h1font);

		Pango::FontDescription pfont;
		//pfont.set_family("Monospace");
		pfont.set_weight(Pango::WEIGHT_MEDIUM);
		pfont.set_size(16*scale);
		auto subTextLayout = create_pango_layout("See log for details.\nPlease include Bitmap Path and Format in bug reports!");
		subTextLayout->set_font_description(pfont);
		subTextLayout->set_alignment(Pango::ALIGN_CENTER);

		int h1_height, h1_width;
		int p_height, p_width;
		mainMessageLayout->get_pixel_size(h1_width, h1_height);
		subTextLayout->get_pixel_size(p_width, p_height);
		Gdk::RGBA colour;
		colour = get_style_context()->get_color(Gtk::STATE_FLAG_INSENSITIVE);

		cr->set_source_rgb(colour.get_red(), colour.get_green(), colour.get_blue());

		// for the main Message.
		// vertically, both layouts get centered together
		cr->move_to((totalWidth - h1_width) / 2, (totalHeight - h1_height - p_height) / 2);
		mainMessageLayout->show_in_cairo_context(cr);

		// subtext, same thing
		cr->move_to((totalWidth - p_width) / 2, ((totalHeight - h1_height - p_height) / 2 ) + h1_height);
		subTextLayout->show_in_cairo_context(cr);
	}

	return true;
}
