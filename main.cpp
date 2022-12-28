#include <MainWindow.h>

int main(int args, char* argv[]){
	auto app = Gtk::Application::create(args, argv, "coreforge.infiniteExplorer");
	app->set_flags(Gio::ApplicationFlags::APPLICATION_NON_UNIQUE);
	MainWindow window;
	window.app = app;
	return app->run(window);
}
