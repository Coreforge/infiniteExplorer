#include "MeshViewer.h"


#include "StringUtils.h"

#include "libInfinite/models/V53ModelLoader.h"

#include "libInfinite/tags/TagManager.h"

#include "MainWindow.h"
#include "materialDialog.h"

#include <memory>

enum{
	MTYPE_MESH,
	MTYPE_LOD,
	MTYPE_PART
};

extern MainWindow* globalWindowPointer;

MeshViewer::MeshViewer(){
	builder = Gtk::Builder::create_from_file("res/meshes.glade");

	Gtk::Box* root;
	builder->get_widget("meshesBox", root);
	add(*root);
	root->set_hexpand(true);
	root->set_vexpand(true);
	root->show();
	//handle = nullptr;

	builder->get_widget("regionsTreeView", regionsTreeView);
	builder->get_widget("meshesTreeView", meshTreeView);
	builder->get_widget("meshesEvtBox", meshesEvtBox);
	builder->get_widget("utilsLoadGLButton", loadGLButton);
	builder->get_widget("exportModel", exportButton);
	builder->get_widget("partContextMenu", partContextMenu);
	builder->get_widget("materialItem", partMaterialItem);

	partContextMenu->attach_to_widget(*meshTreeView);
	//builder->get_widget("regionsTreeStore", regionsStore);

	regionCRecord.add(regionNameColumn);
	regionCRecord.add(regionIndexColumn);
	regionCRecord.add(regionPointerColumn);

	regionsStore = Gtk::TreeStore::create(regionCRecord);

	regionsTreeView->set_model(regionsStore);
	regionsTreeView->append_column(nameViewColumn);
	nameViewColumn.pack_start(regionNameColumn,true);
	nameViewColumn.set_title("Name");


	regionsTreeView->get_selection()->signal_changed().connect( [this](){
		auto s = regionsTreeView->get_selection()->get_selected();
		if(s->get_value(regionPointerColumn) != nullptr){
			ModelPermutation* perm = s->get_value(regionPointerColumn);
			showPermutation(perm);
		}

	});

	meshCRecord.add(meshNameColumn);
	meshCRecord.add(meshTypeColumn);
	meshCRecord.add(meshIndexColumn);
	meshCRecord.add(meshPartPointerColumn);

	meshStore = Gtk::TreeStore::create(meshCRecord);
	meshTreeView->set_model(meshStore);
	meshTreeView->append_column(meshNameViewColumn);
	meshNameViewColumn.pack_start(meshNameColumn,true);
	meshNameViewColumn.set_title("Mesh Name");
	meshTreeView->add_events(Gdk::BUTTON_PRESS_MASK);
	meshTreeView->add_events(Gdk::EventMask::KEY_PRESS_MASK);
	meshTreeView->signal_event_after().connect([this] (GdkEvent* event){
		if(event->type == GdkEventType::GDK_BUTTON_PRESS && event->button.button == GDK_BUTTON_SECONDARY){
			if(meshTreeView->get_selection()->count_selected_rows() == 0){
				return;
			}
			auto s = meshTreeView->get_selection()->get_selected();
			if(s->get_value(meshTypeColumn) == MTYPE_PART){
				// show the context menu only for parts
				partContextMenu->popup(event->button.button, event->button.time);
			}
		}
	});

	partMaterialItem->signal_activate().connect([this]{
		if(meshTreeView->get_selection()->count_selected_rows() == 0){
			return;
		}
		auto s = meshTreeView->get_selection()->get_selected();
		materialDialog* dialog = materialDialog::createMaterialDialog();

		modeHandle* hndl = dynamic_cast<modeHandle*>(tag);
		mat_Handle* mathndl = hndl->getMaterial(((LODMeshPart*)s->get_value(meshPartPointerColumn))->materialIndex);
		dialog->setMaterial(mathndl);
		dialog->show();
	});


	loadGLButton->signal_clicked().connect([this]{
		if(meshTreeView->get_selection()->count_selected_rows() == 0){
			return;
		}
		auto s = meshTreeView->get_selection()->get_selected();

		if(s->get_value(meshTypeColumn) == MTYPE_LOD){
			//do stuff
			//printf("%p\n",s->get_value(meshPartPointerColumn));
			globalWindowPointer->viewer3D.loadLODMesh(model.geoptr, ((LODMesh*)s->get_value(meshPartPointerColumn))->lodmeshptr, tag);
		}

		if(s->get_value(meshTypeColumn) == MTYPE_MESH){
			int meshIndex = s->get_value(meshIndexColumn);
			globalWindowPointer->viewer3D.addRenderGeo(&(dynamic_cast<modeHandle*>(tag))->geoHandle, meshIndex, glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0));
			//globalWindowPointer->currentExporter->addRenderGeo(&(dynamic_cast<modeHandle*>(tag))->geoHandle, meshIndex, glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0),"owo");
			//globalWindowPointer->viewer3D.addMesh(model.geoptr, ((Mesh*)s->get_value(meshPartPointerColumn))->meshptr, tag);
		}
	});


	exportButton->signal_clicked().connect([this]{
		if(meshTreeView->get_selection()->count_selected_rows() == 0){
			return;
		}
		auto s = meshTreeView->get_selection()->get_selected();

		if(s->get_value(meshTypeColumn) == MTYPE_LOD){
			//do stuff
			//printf("%p\n",s->get_value(meshPartPointerColumn));
			//globalWindowPointer->viewer3D.loadLODMesh(model.geoptr, ((LODMesh*)s->get_value(meshPartPointerColumn))->lodmeshptr, tag);
		}

		if(s->get_value(meshTypeColumn) == MTYPE_MESH){
			int meshIndex = s->get_value(meshIndexColumn);
			//globalWindowPointer->viewer3D.addRenderGeo(&(dynamic_cast<modeHandle*>(tag))->geoHandle, meshIndex, glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0));
			globalWindowPointer->currentExporter->addRenderGeo(&(dynamic_cast<modeHandle*>(tag))->geoHandle, meshIndex, glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0),"owo");
			//globalWindowPointer->viewer3D.addMesh(model.geoptr, ((Mesh*)s->get_value(meshPartPointerColumn))->meshptr, tag);
		}
	});

	item = nullptr;
	tag = nullptr;
}

MeshViewer::~MeshViewer(){
	if(item != nullptr){
		delete item;
	}
}

/*std::string BitmapViewer::runExportDialog(std::string fileName){
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
}*/

void MeshViewer::showPermutation(ModelPermutation* perm){
	meshStore->clear();
	for(int m = 0; m < perm->meshes.size(); m++){
		Gtk::TreeStore::iterator meshIt = meshStore->append();
		meshIt->set_value(meshNameColumn,std::string("Mesh ") + std::to_string(m));
		meshIt->set_value(meshPartPointerColumn, (void*)&perm->meshes[m]);
		meshIt->set_value(meshTypeColumn, (int)MTYPE_MESH);
		meshIt->set_value(meshIndexColumn, perm->meshes[m].meshIndex);
		for(int l = 0; l < perm->meshes[m].lods.size(); l++){
			Gtk::TreeStore::iterator lodIt = meshStore->append(meshIt->children());
			lodIt->set_value(meshNameColumn,std::string("LOD ") + std::to_string(l));
			lodIt->set_value(meshPartPointerColumn, (void*)&perm->meshes[m].lods[l]);
			lodIt->set_value(meshTypeColumn, (int)MTYPE_LOD);
			for(int p = 0; p < perm->meshes[m].lods[l].parts.size(); p++){
				Gtk::TreeStore::iterator partIt = meshStore->append(lodIt->children());
				partIt->set_value(meshPartPointerColumn, (void*)&perm->meshes[m].lods[l].parts[p]);
				partIt->set_value(meshNameColumn,std::string("Part ") + std::to_string(p));
				partIt->set_value(meshTypeColumn, (int)MTYPE_PART);
			}
		}
	}
	meshTreeView->expand_all();
}

void MeshViewer::populatePermutations(){
	regionsStore->clear();
	for(int r = 0; r < model.regions.size(); r++){
		Gtk::TreeStore::iterator regionIt = regionsStore->append();
		regionIt->set_value(regionNameColumn, model.regions[r].nameStr);
		regionIt->set_value(regionPointerColumn, (ModelPermutation*)nullptr);
		for(int p = 0; p < model.regions[r].permutations.size(); p++){
			Gtk::TreeStore::iterator permutationIt = regionsStore->append(regionIt->children());
			permutationIt->set_value(regionNameColumn, model.regions[r].permutations[p].nameStr);
			permutationIt->set_value(regionPointerColumn, &model.regions[r].permutations[p]);
		}
	}
}

void MeshViewer::setItem(Item* item){
	this->item = item;

	if(item->tagManager != nullptr){
		tag = item->tagManager->getTag(item->moduleItem->assetID);
		if(tag == nullptr){
			return;
		}
		V53ModelLoader loader;
		loader.loadModel(model, tag, &globalWindowPointer->lut);
		populatePermutations();
	}
	//handle = new BitmapHandle(item->moduleItem,item->logger);
	// set the stuff that's only needed when an item is loaded

}
