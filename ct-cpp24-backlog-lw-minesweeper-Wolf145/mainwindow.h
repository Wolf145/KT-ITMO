#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "field.h"
#include "menu.h"

#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>

class MainWindow : public QWidget
{
	Q_OBJECT
	QVBoxLayout *screen;
	QHBoxLayout *toolBarLayout;
	QAction *newGame, *confGame, *leftHand, *showAll;
	QMenuBar *settsBar;
	QMenu *setts;
	QToolBar *toolBar;
	QLabel *counter;
	Field *field;
	Menu *menu;
	bool dbg = false;

  protected:
	void closeEvent(QCloseEvent *event) override;

  public:
	MainWindow(bool dbg_, QWidget *parent = nullptr);
	~MainWindow();
  signals:
	void changeMode();
  public slots:
	void counterUpd(int newVal);
	void NewGame();
	void CustomizeGame();
};
#endif	  // MAINWINDOW_H
