#include "FileViewerManager.h"



void closeCallback(void* ref, std::string path, void* man){
	FileViewerManager* manager = (FileViewerManager*)man;
	InfiniteFileViewer* viewer = (InfiniteFileViewer*)ref;
	//Gtk::Label* lab = manager->items[path].second;
	//int idx = manager->itemNotebook->page_num(*lab);
	manager->itemNotebook->remove_page(*viewer);
	delete viewer;
	manager->items.erase(path);
	/*if(manager->itemNotebook->get_n_pages() == 0){
		// no pages open anymore
		manager->viewer->setItem(nullptr);	// nullptr displays no item
	}*/
}

void switchCallback(std::string path, void* man){
	FileViewerManager* manager = (FileViewerManager*)man;
	//manager->viewer->setItem(manager->items[path].first);

}

FileViewerManager::FileViewerManager(){
	mainBox = new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0);
	mainBox->show();
	switcher = new Gtk::StackSwitcher();
	switcher->show();
	//mainBox->add(*switcher);
	add(*mainBox);

	itemNotebook = new Gtk::Notebook();
	itemNotebook->show();
	itemNotebook->set_scrollable(true);

	mainBox->add(*itemNotebook);

	noFileLabel = new Gtk::Label();
	noFileLabel->set_text("No files opened");
	noFileLabel->set_halign(Gtk::Align::ALIGN_CENTER);
	noFileLabel->set_valign(Gtk::Align::ALIGN_CENTER);
	noFileLabel->show();	// since there aren't any files opened right now, just show this message
	//mainBox->add(*noFileLabel);

	mainBox->set_hexpand(true);
	mainBox->set_vexpand(true);
	set_hexpand(true);
    set_vexpand(true);

    // the stack
    stack = new Gtk::Stack();
    switcher->set_stack(*stack);
    //switcher->show_all();

    stack->show();
    //mainBox->add(*stack);

    /*viewer = new InfiniteFileViewer();
    mainBox->add(*viewer);
    viewer->show();*/


    itemNotebook->signal_switch_page().connect([this] (Gtk::Widget* page, guint idx){
    	// find which item this is
    	for(auto const&[path,pair] : items){
    		if(page == pair.second){
    			// found it
    			//switchCallback(path, this);

    		}
    	}
    });

}



// assumes control over the item and deletes the object when the page gets closed
void FileViewerManager::addItem(Item* item){
	//items.emplace_back(item);

	// first check if this item is already opened
	if(items.count(item->path) != 0){
		// already open, just switch to it
		itemNotebook->set_current_page(itemNotebook->page_num(*(items[item->path].second)));
		return;
	}

	InfiniteFileViewer* viewer = new InfiniteFileViewer();
	viewer->setItem(item);
	viewer->show();
	//stack->add(*lab,"Page","Page 1");

	ClosableTab* tab = new ClosableTab(item->name, item->path,this,viewer,&closeCallback);
	tab->show();
	itemNotebook->append_page(*viewer, *tab);

	items.insert({item->path,{item,viewer}});

	// the label isn't shown on purpose, as
	/*lab->set_text("");
	lab->show();
	lab->set_lines(0);*/
}
