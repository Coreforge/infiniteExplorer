#include <MainWindow.h>

int main(int args, char* argv[]){
	auto app = Gtk::Application::create(args, argv, "coreforge.infiniteExplorer");
	MainWindow window;

	return app->run(window);
}
