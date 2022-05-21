#include <MainWindow.h>

int main(int args, char* argv[]){
	auto app = Gtk::Application::create(args, argv, "coreforge.infiniteExplorer");
	MainWindow window;
	window.app = app;
	return app->run(window);
}
