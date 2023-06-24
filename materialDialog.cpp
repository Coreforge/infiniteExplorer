#include "materialDialog.h"
#include "MainWindow.h"
#include <cstdio>

#include <sstream>
#include <iomanip>

extern MainWindow* globalWindowPointer;

materialDialog::materialDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) : Gtk::Dialog(cobject),
	m_refGlade(refGlade), Glib::ObjectBase("MaterialDialog")
{
	Gtk::Button* closebutton;
	m_refGlade->get_widget("closeButton", closebutton);

	lsCRecord.add(nameColumn);
	lsCRecord.add(typeColumn);
	lsCRecord.add(valueColumn);
	lsCRecord.add(paramColumn);

	paramsStore = Gtk::ListStore::create(lsCRecord);

	auto vNameColumn = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(m_refGlade->get_object("pNameColumn"));
	vNameColumn->pack_start(nameColumn, true);

	auto vTypeColumn = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(m_refGlade->get_object("pTypeColumn"));
	vTypeColumn->pack_start(typeColumn, true);

	auto vValueColumn = Glib::RefPtr<Gtk::TreeViewColumn>::cast_dynamic(m_refGlade->get_object("pValueColumn"));
	vValueColumn->pack_start(valueColumn, true);

	Gtk::TreeView* paramsView;
	m_refGlade->get_widget("paramsTreeView", paramsView);
	paramsView->set_model(paramsStore);

	paramsView->signal_row_activated().connect([this,globalWindowPointer] (const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
		auto it =paramsStore->get_iter(path);
		auto param = it->get_value(paramColumn);
		if(param->parameterType == materialParameterBase::TYPE_BITMAP){
			auto btmParam = std::dynamic_pointer_cast<bitmapParameter>(param);
			if(globalWindowPointer->moduleManager->modMan.assetIdItems.find(btmParam->globalId) == globalWindowPointer->moduleManager->modMan.assetIdItems.end()){
				// bitmap not loaded or not a valid globalId
				return;
			}
			ModuleItem* item = globalWindowPointer->moduleManager->modMan.assetIdItems[btmParam->globalId];

			globalWindowPointer->moduleManager->displayItem(item, item->name, item->path);
		}

	});

	closebutton->signal_clicked().connect([this]{
		this->close();
		//this->destroy_();
		//m_refGlade.clear();
		//delete this;	// otherwise, the dialog doesn't actually get deleted and hangs around, which isn't great
	});
	this->signal_delete_event().connect([this](GdkEventAny* event) -> bool{
		delete this;
		return false;
	});
}

materialDialog*  materialDialog::createMaterialDialog(){
	auto builder = Gtk::Builder::create_from_file("res/matDialog.glade");
	materialDialog* root;
	builder->get_widget_derived("root", root);
	return root;
}

materialDialog::~materialDialog(){
	//printf("deleting materialDialog\n");
}

void materialDialog::setMaterial(mat_Handle* handle){
	Gtk::Label* matName;
	m_refGlade->get_widget("materialNameLab", matName);
	matName->set_text(handle->item->path);

	std::stringstream stream;
	stream << std::hex << std::setw(8) << std::setfill('0') << handle->getShaderGlobalId();
	Gtk::Label* shadName;
	m_refGlade->get_widget("shaderLab", shadName);
	shadName->set_text(stream.str());

	paramsStore->clear();

	for(int i = 0; i < handle->getParameterCount(); i++){
		auto row = paramsStore->append();
		auto param = handle->getParameter(i);

		row->set_value(nameColumn, globalWindowPointer->lut.lookupID(param->nameId));
		row->set_value(typeColumn, param->getTypeString());
		row->set_value(valueColumn, param->toString());
		row->set_value(paramColumn, param);
	}
}
