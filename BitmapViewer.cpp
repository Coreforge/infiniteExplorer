#include "BitmapViewer.h"

#include "texture_names.h"

#include "StringUtils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
	builder->get_widget("ddsExportSingleMip", exportSingleDDS);
	builder->get_widget("ddsExportAllMips", exportAllDDS);
	builder->get_widget("pngExport", exportSinglePNG);
	builder->get_widget("exportFormatCombo", formatComboBox);

	frameNumberButton->signal_changed().connect([this]{
		updateStuff();
	});

	mipmapComboBox->signal_changed().connect([this]{
		updateMipmap();
	});

	exportSingleDDS->signal_clicked().connect([this]{
		std::string path = runExportDialog(getFilename(item->name) + std::string(".dds"));
		if(path == std::string("")) return;
		int idx = frameNumberButton->get_adjustment()->get_value();
		int lvl = mipmapComboBox->get_active_row_number();
		detexTexture tex = handle->frames[idx].getDetexTexture(lvl);
		if(tex.data == nullptr) return;
		detexSaveDDSFile(&tex, path.c_str());
		free(tex.data);
	});

	exportAllDDS->signal_clicked().connect([this]{
		std::string path = runExportDialog(getFilename(item->name) + std::string(".dds"));
		if(path == std::string("")) return;
		int idx = frameNumberButton->get_adjustment()->get_value();

		detexTexture* texs = (detexTexture*) malloc(handle->frames[idx].mipmapCount * sizeof(detexTexture));
		detexTexture** texpointers = (detexTexture**) calloc(handle->frames[idx].mipmapCount,sizeof(detexTexture*));
		for(int i = 0; i < handle->frames[idx].mipmapCount; i++){
			texs[i] = handle->frames[idx].getDetexTexture(i);
			if(texs[i].data == nullptr){
				goto cleanup;
			}
			texpointers[i] = &texs[i];
		}
		detexSaveDDSFileWithMipmaps(texpointers, handle->frames[idx].mipmapCount, path.c_str());

		cleanup:
		for(int i = 0; i < handle->frames[idx].mipmapCount; i++){
			if(texs[i].data != nullptr){
				free(texs[i].data);
			}
		}
		free(texpointers);
		free(texs);
	});

	exportSinglePNG->signal_clicked().connect([this]{
		int idx = frameNumberButton->get_adjustment()->get_value();
		int lvl = mipmapComboBox->get_active_row_number();
		void* data = handle->frames[idx].getR8G8B8A8Data(lvl);
		if(data == nullptr){
			return;
		}
		if(formatComboBox->get_active_id() == "png"){
			// PNG export
			std::string path = runExportDialog(getFilename(item->name) + std::string(".png"));
			if(path == std::string("")) return;

			if(!stbi_write_png(path.c_str(), handle->frames[idx].mipMaps[lvl].width, handle->frames[idx].mipMaps[lvl].height,
					4, data, handle->frames[idx].mipMaps[lvl].width * 4)){
				// write failed
				printf("png export failed\n");
			}

		}
		if(formatComboBox->get_active_id() == "tga"){
			// PNG export
			std::string path = runExportDialog(getFilename(item->name) + std::string(".tga"));
			if(path == std::string("")) return;

			if(!stbi_write_tga(path.c_str(), handle->frames[idx].mipMaps[lvl].width, handle->frames[idx].mipMaps[lvl].height,
					4, data)){
				// write failed
				printf("tga export failed\n");
			}

		}
	});


	item = nullptr;
}

BitmapViewer::~BitmapViewer(){
	if(item != nullptr){
		delete item;
	}
}

std::string BitmapViewer::runExportDialog(std::string fileName){
	Gtk::FileChooserDialog* fileChooser = new Gtk::FileChooserDialog("Export",Gtk::FILE_CHOOSER_ACTION_SAVE);
	fileChooser->add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	fileChooser->add_button("_Save", Gtk::RESPONSE_OK);

	fileChooser->set_current_name(fileName);
	int response = fileChooser->run();
	fileChooser->close();
	if(response == Gtk::RESPONSE_OK){
		return fileChooser->get_filename();
	} else {
		return std::string("");
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
