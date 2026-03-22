#include "menu.h"

#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

Menu::Menu(QWidget *parent) : QDialog(parent)
{
	setWindowTitle(tr("Menu"));

	screen = new QVBoxLayout(this);

	lbl1 = new QLabel(tr("Field settings"), this);
	screen->addWidget(lbl1);

	sizeX = new QSpinBox(this);
	sizeX->setRange(6, 50);
	Sx = new QHBoxLayout(this);
	lbl2 = new QLabel(tr("Num Columns: "), this);
	Sx->addWidget(lbl2);
	Sx->addWidget(sizeX);

	sizeY = new QSpinBox(this);
	sizeY->setRange(6, 50);
	Sy = new QHBoxLayout(this);
	lbl3 = new QLabel(tr("Num Rows: "), this);
	Sy->addWidget(lbl3);
	Sy->addWidget(sizeY);

	screen->addLayout(Sx);
	screen->addLayout(Sy);

	numberMines = new QSpinBox(this);
	numberMines->setRange(6, sizeX->value() * sizeY->value() - 1);
	lbl4 = new QLabel(tr("Amount of mines: "), this);
	screen->addWidget(lbl4);
	screen->addWidget(numberMines);

	startGame = new QPushButton(tr("Start Game"), this);
	screen->addWidget(startGame);

	QObject::connect(startGame, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(sizeX, QOverload< int >::of(&QSpinBox::valueChanged), this, &Menu::updateMineRange);
	QObject::connect(sizeY, QOverload< int >::of(&QSpinBox::valueChanged), this, &Menu::updateMineRange);
}
void Menu::updateMineRange()
{
	numberMines->setMaximum(sizeX->value() * sizeY->value() - 1);
}
