#include "40in_engine_v3.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QApplication>


int main(int argc, char* argv[])
{

	QApplication app(argc, argv);

	QMainWindow main_window;
	main_window.setWindowTitle("40in Engine v3");
	main_window.resize(800, 600);
	main_window.show();

	app.exec();

	return 0;
}
