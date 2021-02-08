#include <QApplication>
#include <QtWidgets>

#include "..\headers\MainWindow.hpp"
#include "..\headers\Optimizer.hpp"
#include "..\headers\Controller.hpp"
#include "..\headers\TableSaver.hpp"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

	QFont font("times new roman", 12);
	QApplication::setFont(font);

	MainWindow ui;
	Optimizer optimizer;
	TableSaver tableSaver;
	Controller controller(&ui, &optimizer, &tableSaver);

	ui.show();

	return app.exec();
}