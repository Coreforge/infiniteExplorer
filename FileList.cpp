#include "FileList.h"
#include <iostream>

namespace sigc {
  SIGC_FUNCTORS_DEDUCE_RESULT_TYPE_WITH_DECLTYPE
}

FileList::FileList(Gtk::Container* window, Glib::RefPtr<Gtk::Builder> builder){
	parent = window;
	this->builder = builder;


	//builder->get_widget("files.frame", frame);
	//builder->get_widget("files.frame.label", label);
	frame = new Gtk::Frame();
	label = new Gtk::Label("Files");
	listBox = new Gtk::Box();
	internalLayoutBox = new Gtk::Box();
	controlBox = new Gtk::Box();
	evtBox = new Gtk::EventBox();
	currentPathLabel = new Gtk::Label();
	listScroller = new Gtk::ScrolledWindow();
	pathScroller = new Gtk::ScrolledWindow();
	searchBar = new Gtk::SearchEntry();


	label->set_text("Files");
	frame->show();
	label->show();
	frame->set_label_widget(*label);
	//Gtk::Allocation alloc;
	//alloc.set_width(150);
	frame->set_label_align(0.5, 0.5);
	frame->set_size_request(150, 300);

	evtBox->add(*frame);
	parent->add(*evtBox);
	//frame->add(*label);


	listBox->set_property("orientation", Gtk::Orientation::ORIENTATION_VERTICAL);
	listBox->show();
	internalLayoutBox->set_property("orientation", Gtk::Orientation::ORIENTATION_VERTICAL);
	internalLayoutBox->show();

	//Search bar
	searchBar->show();
	internalLayoutBox->add(*searchBar);
	searchBar->signal_search_changed().connect([this]{onSearch();});


	// controls/Path
	controlBox->set_property("orientation", Gtk::Orientation::ORIENTATION_HORIZONTAL);
	controlBox->show();
	frame->add(*internalLayoutBox);
	internalLayoutBox->add(*controlBox);
	pathScroller->add(*currentPathLabel);
	controlBox->add(*pathScroller);
	pathScroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER);

//#ifndef _WIN64
	pathScroller->set_propagate_natural_width(true);
//#endif
	//currentPathLabel->set_editable(false);
	currentPathLabel->show();
	pathScroller->show();
	//internalLayoutBox->add(*listBox);

	//List
	listScroller->add(*listBox);
	listScroller->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

//#ifndef _WIN64
	listScroller->set_propagate_natural_height(true);
	//listScroller->set_property("propagate-natural-height", true);
//#endif
	internalLayoutBox->add(*listScroller);
	listScroller->show();





	evtBox->add_events(Gdk::BUTTON_PRESS_MASK);
	evtBox->add_events(Gdk::KEY_PRESS_MASK);
	evtBox->add_events(Gdk::KEY_RELEASE_MASK);
	evtBox->signal_button_press_event().connect([this] (GdkEventButton* event){onFrameClicked();return 0;});
	evtBox->signal_key_press_event().connect([this] (GdkEventKey* key){onKeyPressed(key);return 0;});
	evtBox->signal_key_release_event().connect([this] (GdkEventKey* key){onKeyReleased(key);return 0;});
	evtBox->show();

	frame->signal_check_resize().connect([this] {onResize();});
	// load the style sheet for the buttons
	cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("res/fileExplorer.css");
	selectedEntries.clear();
}

void FileList::onResize(){
	printf("Resize!\n");
}

void FileList::setSearchCallback(void (*onSearch)(void*,std::string), void* manager){
	this->onSearchCallback = onSearch;
	this->manager = manager;
}

void FileList::onSearch(){
	printf("Searching for %s\n",searchBar->get_text().c_str());
	onSearchCallback(manager, searchBar->get_text());
}

void FileList::onKeyPressed(GdkEventKey* key){
	if(key->keyval == GDK_KEY_Shift_L || key->keyval == GDK_KEY_Shift_R){
		shiftPressed = true;
	}

	if(key->keyval == GDK_KEY_Control_L || key->keyval == GDK_KEY_Control_R){
		ctrlPressed = true;
	}
}

void FileList::onKeyReleased(GdkEventKey* key){
	if(key->keyval == GDK_KEY_Shift_L || key->keyval == GDK_KEY_Shift_R){
		shiftPressed = false;
	}

	if(key->keyval == GDK_KEY_Control_L || key->keyval == GDK_KEY_Control_R){
		ctrlPressed = false;
	}
}

void FileList::onFrameClicked(){
	// deselect all selected Entries
	for(int i = 0; i < selectedEntries.size(); i++){
		Glib::RefPtr<Gtk::StyleContext> context = selectedEntries[i].second->get_style_context();
		context->remove_class("selected");
		selectedEntries[i].first->selected = false;
	}
	selectedEntries.clear();
}

void FileList::onEntryDoubleClicked(Gtk::Button* button, GdkEventButton* event, FileEntry* entry){
	if(event->type == GDK_2BUTTON_PRESS){
		std::cout << "Double click!\n";
		if(entry->onClick != nullptr){
			// we have a function to call
			entry->onClick(entry->nodeRef,entry->data);
		}
	}
	if(event->type == GDK_BUTTON_PRESS){
		std::cout << "click!\n";
	}
}

void FileList::onEntryClicked(Gtk::Button* button, FileEntry* entry){

	// neither shift or ctrl is pressed (normal selection), or shift is pressed, but there is no active entry
	if((!shiftPressed && !ctrlPressed) || (shiftPressed && activeEntry.first == nullptr)){
		for(int i = 0; i < selectedEntries.size(); i++){
			Glib::RefPtr<Gtk::StyleContext> context = selectedEntries[i].second->get_style_context();
			selectedEntries[i].first->selected = false;
			context->remove_class("selected");
		}
		selectedEntries.clear();
		activeEntry.first = entry;
		activeEntry.second = button;
		selectedEntries.emplace_back(entry,button);
		Glib::RefPtr<Gtk::StyleContext> context = button->get_style_context();
		context->add_class("selected");
		entry->selected = true;
	}

	if(ctrlPressed){
		Glib::RefPtr<Gtk::StyleContext> context = button->get_style_context();
		if(entry->selected){
			// entry is selected, remove it from selection
			// find the index of the current entry to remove it
			for(int i = 0; i < selectedEntries.size(); i++){
				if(selectedEntries[i].first != entry){
					continue;
				}
				// found the entry, now remove it
				selectedEntries.erase(selectedEntries.begin() + i);
				context->remove_class("selected");
				entry->selected = false;
				break;
			}
		} else {
			// entry isn't selected yet, add it to selection
			context->add_class("selected");
			selectedEntries.emplace_back(entry,button);
			entry->selected = true;
		}
	}

	if(shiftPressed && activeEntry.first != nullptr){
		// get the index of the active entry
		int activeEntryIndex;
		for(int i = 0; i < shownEntries.size(); i++){
			if(shownEntries[i].second == activeEntry.second){
				// found the index of the active entry
				activeEntryIndex = i;
				break;
			}
		}
		// now get the index of the entry that was clicked
		int clickedEntryIndex;
		for(int i = 0; i < shownEntries.size(); i++){
			if(shownEntries[i].second == button){
				// found the index of the active entry
				clickedEntryIndex = i;
				break;
			}
		}

		// clear selection
		for(int i = 0; i < selectedEntries.size(); i++){
			std::cout << selectedEntries.size() << " selected Entries\n";
			Glib::RefPtr<Gtk::StyleContext> context = selectedEntries[i].second->get_style_context();
			selectedEntries[i].first->selected = false;
			context->remove_class("selected");
		}
		selectedEntries.clear();

		for(int i = std::min(activeEntryIndex, clickedEntryIndex); i <= std::max(activeEntryIndex, clickedEntryIndex); i++){
			// select all entries that should be selected
			selectedEntries.emplace_back(&shownEntries[i].first,shownEntries[i].second);
			Glib::RefPtr<Gtk::StyleContext> context = shownEntries[i].second->get_style_context();
			context->add_class("selected");
			shownEntries[i].first.selected = true;
		}

	}

}

void FileList::clearList(){
	for(int i = 0; i < shownEntries.size(); i++){
		listBox->remove(*shownEntries[i].second);
		shownEntries[i].second->remove();
		shownEntries[i].second->~Button();

	}
	shownEntries.clear();
	selectedEntries.clear();

}

void FileList::updateFiles(std::vector<FileEntry*> entries){
	clearList();
	for(int i = 0; i < entries.size(); i++){
		Gtk::Button* button = new Gtk::Button();
		button->set_label(entries[i]->name);
		if(entries[i]->onClick != nullptr){
			//button->signal_clicked().connect(sigc::bind(sigc::ptr_fun(entries[i]->onClickID),entries[i]->ID));
			button->add_events(Gdk::BUTTON_PRESS_MASK);
			//button->signal_button_press_event().connect([this,button] (GdkEventButton* event){return onEntryClicked(button, event, entries[i]);});
		}

		Glib::RefPtr<Gtk::StyleContext> buttonContext = button->get_style_context();
		buttonContext->add_provider(cssProvider, GTK_STYLE_PROVIDER_PRIORITY_USER);
		//buttonContext->add_class("selected");


		const char* iconName;
		switch(entries[i]->type){
		case FILE_TYPE_FILE:
			iconName = FILE_TYPE_FILE_ICON;
			break;
		case FILE_TYPE_DIRECTORY:
			iconName = FILE_TYPE_DIRECTORY_ICON;
		}
		button->set_image_from_icon_name(iconName, Gtk::ICON_SIZE_SMALL_TOOLBAR);
		button->set_always_show_image(true);
		button->set_image_position(Gtk::PositionType::POS_LEFT);
		button->set_tooltip_text(entries[i]->path);
		button->set_property("xalign", 0.0);
		listBox->add(*button);
		button->show();
		shownEntries.emplace_back(*entries[i],button);
		FileEntry* entryCopy = entries[i];//&shownEntries.back().first;

		button->add_events(Gdk::BUTTON_PRESS_MASK);
		button->signal_button_press_event().connect([this,button,entryCopy] (GdkEventButton* event){onEntryDoubleClicked(button, event, entryCopy); return 0;});
		button->signal_clicked().connect([this,button,entryCopy] {onEntryClicked(button,entryCopy);});
	}
	selectedEntries.clear();
	activeEntry.first = nullptr;
	activeEntry.second = nullptr;
}
