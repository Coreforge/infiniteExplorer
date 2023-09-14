#pragma once

#include <gtkmm.h>
#include <string>
#include "OptionalViewer.h"

#include "libInfinite/Item.h"

#include "libInfinite/models/Rendermodel.h"

#include "libInfinite/tags/Tag.h"

class MeshViewer : public OptionalViewer{

public:
	MeshViewer();
	~MeshViewer() override;
	void setItem(Item* item);

	std::string getName() { return std::string("Mesh Viewer");};

private:


	Glib::RefPtr<Gtk::Builder> builder;

	// Pointers to widgets that are needed multiple times
	Gtk::TreeView* regionsTreeView;
	Glib::RefPtr<Gtk::TreeStore> regionsStore;
	Gtk::TreeView* meshTreeView;
	Glib::RefPtr<Gtk::TreeStore> meshStore;
	Gtk::EventBox* meshesEvtBox;
	Gtk::Menu* partContextMenu;
	Gtk::MenuItem* partMaterialItem;

	// region/permutation model
	Gtk::TreeStore::ColumnRecord regionCRecord;
	Gtk::TreeModelColumn<std::string> regionNameColumn;
	Gtk::TreeModelColumn<int> regionIndexColumn;
	Gtk::TreeModelColumn<ModelPermutation*> regionPointerColumn;
	Gtk::TreeViewColumn nameViewColumn;

	// mesh/LOD model
	Gtk::TreeStore::ColumnRecord meshCRecord;
	Gtk::TreeModelColumn<std::string> meshNameColumn;
	Gtk::TreeModelColumn<void*> meshPartPointerColumn;
	Gtk::TreeModelColumn<int> meshTypeColumn;
	Gtk::TreeModelColumn<int> meshIndexColumn;
	Gtk::TreeViewColumn meshNameViewColumn;


	RenderModel model;

	Gtk::Button* loadGLButton;
	Gtk::Button* exportButton;

	//std::string runExportDialog(std::string fileName);

	Item* item;
	Tag* tag;

	void populatePermutations();
	void showPermutation(ModelPermutation* perm);

	enum regionColumns{
		COLUMN_NAME,
		COLUMN_MESH_INDEX,
		COLUMN_MESH_COUNT
	};



};
