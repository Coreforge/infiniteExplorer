#pragma once

#include <gtkmm.h>
#include <string>

#include "libInfinite/tags/handles/mat_Handle.h"

class materialDialog : public Gtk::Dialog{
public:
	materialDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	static materialDialog* createMaterialDialog();
	~materialDialog();

	void setMaterial(mat_Handle* handle);

private:
	Gtk::ListStore::ColumnRecord lsCRecord;
	Gtk::TreeModelColumn<std::string> nameColumn;
	Gtk::TreeModelColumn<std::string> typeColumn;
	Gtk::TreeModelColumn<std::string> valueColumn;
	Gtk::TreeModelColumn<std::shared_ptr<materialParameterBase>> paramColumn;
	Glib::RefPtr<Gtk::ListStore> paramsStore;

protected:
	Glib::RefPtr<Gtk::Builder> m_refGlade;
};
