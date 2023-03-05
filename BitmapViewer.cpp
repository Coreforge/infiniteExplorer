#include "BitmapViewer.h"

#include "texture_names.h"

BitmapViewer::BitmapViewer(){
	builder = Gtk::Builder::create_from_file("res/bitmapViewer.glade");

	Gtk::Box* root;
	builder->get_widget("bitmapViewerTLBox", root);
	add(*root);
	root->set_hexpand(true);
	root->set_vexpand(true);
	root->show();
	handle = nullptr;

	builder->get_widget("statusFormat", dataTypeLabel);
	builder->get_widget("frameIndex", frameNumberButton);
	builder->get_widget("statusCurrentMipmapSize", currentSizeLabel);
	builder->get_widget("statusMaxSize", maxSizeLabel);
	builder->get_widget("mipmapComboBox", mipmapComboBox);
	builder->get_widget_derived("bitmapDrawingArea", viewerDrawingArea);

	frameNumberButton->signal_changed().connect([this]{
		updateStuff();
	});

	mipmapComboBox->signal_changed().connect([this]{
		updateMipmap();
	});



	item = nullptr;
}

BitmapViewer::~BitmapViewer(){
	if(item != nullptr){
		delete item;
	}
}

void BitmapViewer::updateStuff(){
	// set all values for this frame
	int idx = frameNumberButton->get_adjustment()->get_value();
	if(idx >= handle->frameCount){
		return;
	}
	maxSizeLabel->set_text(std::to_string(handle->frames[idx].width) + std::string("x") + std::to_string(handle->frames[idx].height));
	mipmapComboBox->remove_all();
	for(int m = 0; m < handle->frames[idx].mipmapCount; m++){
		std::string levelLabel = "Level ";
		levelLabel += std::to_string(m);
		levelLabel += " (";
		levelLabel += std::to_string(handle->frames[idx].mipMaps[m].width);
		levelLabel += "x";
		levelLabel += std::to_string(handle->frames[idx].mipMaps[m].height);
		levelLabel += ")";
		mipmapComboBox->append(std::to_string(m),levelLabel);
		mipmapComboBox->set_active(0);
	}
	dataTypeLabel->set_text(std::string(dxgi_types[handle->frames[idx].format]));
	updateMipmap();
}

void BitmapViewer::updateMipmap(){
	int frameIdx = frameNumberButton->get_adjustment()->get_value();
	int lvl = mipmapComboBox->get_active_row_number();

	int width = handle->frames[frameIdx].mipMaps[lvl].width;
	int height = handle->frames[frameIdx].mipMaps[lvl].height;

	currentSizeLabel->set_text(std::to_string(width) + std::string("x") + std::to_string(height));

	// also, actually display the texture. That might be important
	void* buf = handle->frames[frameIdx].getR8G8B8A8Data(lvl);
	if(buf == nullptr){
		// no Texture to show, whoops
		viewerDrawingArea->pixbufValid = false;
	} else {
		viewerDrawingArea->pixbufValid = true;
		viewerDrawingArea->width = width;
		viewerDrawingArea->height = height;
		viewerDrawingArea->pixbuf = Gdk::Pixbuf::create_from_data((uint8_t*)buf, Gdk::Colorspace::COLORSPACE_RGB, true, 8, width, height, width*4);
		//free(buf);
	}
}

void BitmapViewer::setItem(Item* item){
	this->item = item;
	handle = new BitmapHandle(item->moduleItem,item->logger);
	// set the stuff that's only needed when an item is loaded
	Gtk::Label* lab;
	builder->get_widget("statusFrameCount", lab);
	lab->set_text(std::string("Frames: ") + std::to_string(handle->frameCount));


	frameNumberButton->get_adjustment()->set_upper(handle->frameCount - 1);
	frameNumberButton->get_adjustment()->set_value(0);
	frameNumberButton->get_adjustment()->set_lower(0);
	frameNumberButton->get_adjustment()->set_step_increment(1);


	updateStuff();
}
