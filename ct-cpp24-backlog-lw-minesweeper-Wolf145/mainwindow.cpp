#include "mainwindow.h"

#include "menu.h"

#include <QGridLayout>
#include <QMenuBar>
#include <QSettings>
#include <QToolBar>
#include <QTranslator>

MainWindow::MainWindow(bool dbg_, QWidget *parent) : QWidget(parent), dbg(dbg_)
{
	screen = new QVBoxLayout(this);
	menu = new Menu(this);
	QSettings settings("save.ini", QSettings::IniFormat);
	settings.beginGroup("Game");
	bool isnSaved = settings.value("isnSaved", true).toBool();
	settings.endGroup();

	if (isnSaved)
	{
		field = new Field(10, 10, 10, this);
	}
	else
	{
		field = new Field("save.ini", this);
	}

	setWindowTitle(tr("Minesweeper"));

	newGame = new QAction(this);
	newGame->setText(tr("Restart"));
	connect(newGame, &QAction::triggered, this, &MainWindow::NewGame);

	confGame = new QAction(this);
	confGame->setText(tr("Menu"));
	connect(confGame, &QAction::triggered, this, &MainWindow::CustomizeGame);

	leftHand = new QAction(this);
	leftHand->setText(tr("Left Hand Mode"));
	connect(leftHand, &QAction::triggered, field, &Field::changeMode);

	setts = new QMenu(this);
	setts->setTitle(QString(tr("Settings")));
	setts->addAction(confGame);
	setts->addAction(newGame);
	setts->addAction(leftHand);

	counter = new QLabel(this);
	if (isnSaved)
	{
		counter->setText("");
	}
	else
	{
		QSettings settings("save.ini", QSettings::IniFormat);
		settings.beginGroup("Game");
		counter->setText(settings.value("counterVal").toString());
		settings.endGroup();
	}
	connect(field, &Field::changeCounter, this, &MainWindow::counterUpd);

	toolBar = new QToolBar(this);
	toolBar->addAction(confGame);
	toolBar->addAction(newGame);
	toolBar->addAction(leftHand);
	toolBar->addWidget(counter);

	if (dbg)
	{
		showAll = new QAction(this);
		showAll->setText(tr("Show Field"));
		connect(showAll, &QAction::triggered, field, &Field::lookMines);

		setts->addAction(showAll);
		toolBar->addAction(showAll);
	}

	settsBar = new QMenuBar(this);
	settsBar->addMenu(setts);

	toolBarLayout = new QHBoxLayout;
	toolBarLayout->addWidget(toolBar);
	toolBarLayout->addWidget(settsBar);

	screen->addLayout(toolBarLayout);
	screen->addWidget(field);
	field->show();
	setLayout(screen);
}
void MainWindow::NewGame()
{
	int32_t x = field->szX;
	int32_t y = field->szY;
	int32_t m = field->numMines;
	delete field;
	field = new Field(x, y, m, this);
	connect(field, &Field::changeCounter, this, &MainWindow::counterUpd);
	connect(leftHand, &QAction::triggered, field, &Field::changeMode);
	counter->setText("");
	if (dbg)
		connect(showAll, &QAction::triggered, field, &Field::lookMines);
	layout()->addWidget(field);
}

void MainWindow::CustomizeGame()
{
	menu->exec();
	int32_t x = menu->getX();
	int32_t y = menu->getY();
	int32_t m = menu->getM();
	delete field;
	field = new Field(x, y, m, this);
	connect(field, &Field::changeCounter, this, &MainWindow::counterUpd);
	connect(leftHand, &QAction::triggered, field, &Field::changeMode);
	counter->setText("");
	if (dbg)
		connect(showAll, &QAction::triggered, field, &Field::lookMines);
	layout()->addWidget(field);
}
void MainWindow::counterUpd(int newVal)
{
	counter->setText("Rest Mines: " + QString::number(newVal));
}
void MainWindow::closeEvent(QCloseEvent *event)
{
	if (!field->getFlag())
	{
		QSettings settings("save.ini", QSettings::IniFormat);
		settings.beginGroup("Game");
		settings.setValue("counterVal", "Rest Mines: " + QString::number(field->numMines - field->numFlags));
		settings.endGroup();
		field->saveGame();
	}
	event->accept();
}
MainWindow::~MainWindow()
{
	delete menu;
	delete toolBarLayout;
	delete field;
	delete settsBar;
	delete setts;
	if (dbg)
		delete showAll;
	delete toolBar;
	delete counter;
	delete confGame;
	delete newGame;
	delete leftHand;
	delete screen;
}
