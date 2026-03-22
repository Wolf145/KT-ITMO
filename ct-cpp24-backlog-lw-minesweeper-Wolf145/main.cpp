#include "mainwindow.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w(argc > 1 && !strcmp(argv[1], "dbg"));
	w.show();
	return a.exec();
}
