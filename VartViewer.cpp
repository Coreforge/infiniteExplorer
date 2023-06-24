#include "VartViewer.h"

#include "StringUtils.h"

#include "stb_image_write.h"

VartViewer::VartViewer(){
	builder = Gtk::Builder::create_from_file("res/bitmapViewer.glade");

	Gtk::Box* root;
	builder->get_widget("bitmapViewerTLBox", root);
	add(*root);
	root->set_hexpand(true);
	root->set_vexpand(true);
	root->show();
	tag = nullptr;

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

	/*frameNumberButton->signal_changed().connect([this]{
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
	});*/


	item = nullptr;
}

VartViewer::~VartViewer(){
	if(item != nullptr){
		delete item;
	}
}

std::string VartViewer::runExportDialog(std::string fileName){
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

void VartViewer::updateStuff(){
	// set all values for this frame

	updateMipmap();
}

void VartViewer::updateMipmap(){
	//int frameIdx = frameNumberButton->get_adjustment()->get_value();
	//int lvl = mipmapComboBox->get_active_row_number();

	//int width = handle->frames[frameIdx].mipMaps[lvl].width;
	//int height = handle->frames[frameIdx].mipMaps[lvl].height;

	// also, actually display the texture. That might be importan
	//if(1){
		// no Texture to show, whoops
		//viewerDrawingArea->vectorValid = false;
	//} else {
		viewerDrawingArea->vectorValid = true;
		viewerDrawingArea->handle = tag;
		//viewerDrawingArea->width = width;
		//viewerDrawingArea->height = height;
		//viewerDrawingArea->pixbuf = Gdk::Pixbuf::create_from_data((uint8_t*)buf, Gdk::Colorspace::COLORSPACE_RGB, true, 8, width, height, width*4);
		//free(buf);
	//}
}

void VartViewer::setItem(Item* item){
	this->item = item;
	if(item->tagManager != nullptr){
		Tag* tmptag = item->tagManager->getTag(item->moduleItem->assetID);
		tag = dynamic_cast<vartHandle*>(tmptag);
		if(tag == nullptr){
			return;
		}
	}
	// set the stuff that's only needed when an item is loaded
	Gtk::Label* lab;
	builder->get_widget("statusFrameCount", lab);



	updateStuff();
}
