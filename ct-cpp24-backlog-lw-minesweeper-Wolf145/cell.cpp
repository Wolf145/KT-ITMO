#include "cell.h"

#include <QMouseEvent>
#include <QPushButton>
#include <QResizeEvent>

Cell::Cell(int16_t x, int16_t y, QWidget *parent) : QPushButton(parent), x(x), y(y)
{
	setStyleSheet("background-color: rgb(170, 170, 170);");
	setFixedSize(46, 46);
	heightForWidth(true);
}
void Cell::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		emit pressRight(x, y);
	else if (event->button() == Qt::LeftButton)
		emit pressLeft(x, y);
	else
		emit pressMid();
}
Cell::Cell(QWidget *parent) : QPushButton(parent) {}
void Cell::paintNums()
{
	this->setText(QString::number(this->countMine));
	switch (this->countMine)
	{
	case 1:
		this->setStyleSheet("color: darkblue;");
		break;
	case 2:
		this->setStyleSheet("color: green;");
		break;
	case 3:
		this->setStyleSheet("color: red;");
		break;
	case 4:
		this->setStyleSheet("color: purple;");
		break;
	case 5:
		this->setStyleSheet("color: orange;");
		break;
	case 6:
		this->setStyleSheet("color: lightblue;");
		break;
	case 7:
		this->setStyleSheet("color: yellow;");
		break;
	case 8:
		this->setStyleSheet("color: black;");
		break;
	}
}
