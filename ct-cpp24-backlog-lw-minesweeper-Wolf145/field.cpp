#include "field.h"

#include "cell.h"

#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QString>
#include <QTimer>
#include <QtGlobal>
#include <QtWidgets>

Field::Field(int16_t x, int16_t y, int32_t m, QWidget *parent) :
	QWidget(parent), szX(x), szY(y), numMines(m), numFlags(0)
{
	info = new QMessageBox(this);
	table = new QGridLayout(this);
	createField();
	setLayout(table);
}
Field::Field(const QString &saveFileName, QWidget *parent) : QWidget(parent)
{
	info = new QMessageBox(this);
	table = new QGridLayout(this);
	QSettings settings(saveFileName, QSettings::IniFormat);

	settings.beginGroup("Game");
	countOpen = settings.value("countOpen").toInt();
	firstClick = settings.value("firstClick").toBool();
	szX = settings.value("szX").toInt();
	szY = settings.value("szY").toInt();
	numMines = settings.value("numMines").toInt();
	numFlags = settings.value("numFlags").toInt();
	leftMd = settings.value("leftMd").toBool();
	settings.endGroup();

	createField();

	for (int i = 0; i < szX; ++i)
	{
		for (int j = 0; j < szY; ++j)
		{
			Cell *button = board[i][j];
			settings.beginGroup(QString("Cell_%1_%2").arg(i).arg(j));
			button->covered = settings.value("covered").toBool();
			button->isFlag = settings.value("isFlag").toInt();
			button->countMine = settings.value("countMine").toInt();
			button->x = settings.value("x").toInt();
			button->y = settings.value("y").toInt();
			settings.endGroup();

			if (!button->covered)
			{
				if (button->countMine > 0)
				{
					button->paintNums();
				}
				else
				{
					button->setText("");
					button->setStyleSheet("background-color: rgb(150, 150, 150);");
				}
			}
			else if (button->isFlag == 1)
			{
				button->setText("|>");
				button->setStyleSheet("color: red");
			}
			else if (button->isFlag == 2)
			{
				button->setText("?");
				button->setStyleSheet("color: black");
			}
			else
			{
				button->setText("");
				button->setStyleSheet("background-color: rgb(170, 170, 170);");
			}
		}
	}

	setLayout(table);
}
void Field::createField()
{
	board.resize(szX);
	for (uint16_t i = 0; i < szX; i++)
		board[i].resize(szY);

	table->setSpacing(1);
	for (int16_t i = 0; i < szX; i++)
		table->setColumnStretch(i, 1);
	for (int16_t i = 0; i < szY; i++)
		table->setRowStretch(i, 1);

	for (int16_t i = 0; i < szX; i++)
	{
		for (int16_t j = 0; j < szY; j++)
		{
			board[i][j] = new Cell(i, j, this);
			table->addWidget(board[i][j], i, j);
			QObject::connect(board[i][j], &Cell::pressLeft, this, &Field::leftClick);
			QObject::connect(board[i][j], &Cell::pressRight, this, &Field::rightClick);
			QObject::connect(board[i][j], &Cell::pressMid, this, &Field::midClick);
		}
	}
}
void Field::generateBoard()
{
	for (int32_t cntMine = 0; cntMine < numMines;)
	{
		int16_t i = QRandomGenerator::global()->bounded(szX);
		int16_t j = QRandomGenerator::global()->bounded(szY);
		if (board[i][j]->countMine != -1 && board[i][j]->covered)
		{
			board[i][j]->countMine = -1;
			++cntMine;
		}
	}
	for (int16_t i = 0; i < szX; i++)
	{
		for (int16_t j = 0; j < szY; j++)
		{
			if (board[i][j]->countMine == -1)
				continue;
			int16_t count = 0;
			for (int16_t di = -1; di < 2; di++)
				for (int16_t dj = -1; dj < 2; dj++)
					count += (0 <= i + di && i + di < szX && 0 <= j + dj && j + dj < szY && board[i + di][j + dj]->countMine == -1);
			board[i][j]->countMine = count;
		}
	}
}
void Field::recurOpenCells(int16_t x, int16_t y)
{
	Cell *button = board[x][y];
	if (button->isFlag)
	{
		return;
	}
	button->covered = false;
	++countOpen;
	if (button->countMine > 0)
		button->paintNums();
	else
	{
		button->setStyleSheet("background-color: rgb(150, 150, 150);");
		button->setText("");
		button->setEnabled(false);
		for (int8_t i = -1; i < 2; i++)
			for (int8_t j = -1; j < 2; j++)
				if (0 <= x + i && x + i < szX && 0 <= y + j && y + j < szY && board[x + i][y + j]->covered)
					recurOpenCells(x + i, y + j);
	}
}
void Field::lose(Cell *button)
{
	for (int16_t i = 0; i < szX; i++)
	{
		for (int16_t j = 0; j < szY; j++)
		{
			board[i][j]->setEnabled(false);
			if (board[i][j]->countMine == -1)
			{
				board[i][j]->setText("M");
				board[i][j]->setStyleSheet("background-color: rgb(140, 0, 0);");
			}
			else if (board[i][j]->isFlag)
			{
				board[i][j]->setStyleSheet("background-color: rgb(255,255,153);");
			}
			else if (board[i][j]->countMine > 0)
			{
				board[i][j]->paintNums();
			}
			else
			{
				board[i][j]->setText("");
				board[i][j]->setStyleSheet("background-color: rgb(150, 150, 150);");
			}
		}
	}
	QApplication::processEvents();
	info->setText(tr("You died, try again"));
	info->show();
	button->setStyleSheet("background-color: rgb(200, 0, 0);");
	saved = true;
}
void Field::win()
{
	for (int16_t i = 0; i < szX; i++)
	{
		for (int16_t j = 0; j < szY; j++)
		{
			board[i][j]->setEnabled(false);
			if (board[i][j]->covered)
			{
				board[i][j]->setText("M");
				board[i][j]->setStyleSheet("color: black; background-color: rgb(200, 0, 0)");
				QFont font = board[i][j]->font();
				font.setBold(true);
				board[i][j]->setFont(font);
			}
		}
	}
	QApplication::processEvents();
	info->setText(tr("You win"));
	info->show();
	saved = true;
}
void Field::openNeighbours(Cell *button)
{
	int32_t i = button->x;
	int32_t j = button->y;
	QVector< Cell * > close;
	for (int8_t di = -1; di < 2; di++)
	{
		for (int8_t dj = -1; dj < 2; dj++)
		{
			if (0 <= i + di && i + di < szX && 0 <= j + dj && j + dj < szY && board[i + di][j + dj]->covered &&
				!(board[i + di][j + dj]->isFlag))
			{
				board[i + di][j + dj]->setStyleSheet("border: 2px solid lightyellow;");
				close.push_back(board[i + di][j + dj]);
			}
		}
	}
	QTimer::singleShot(
		500,
		this,
		[close]()
		{
			for (Cell *b : close)
			{
				if (b->look)
				{
					b->setText("M");
					QFont font = b->font();
					font.setPointSize(12);
					font.setBold(true);
					b->setFont(font);
					b->setStyleSheet("background-color: rgb(200, 0, 0);");
				}
				else
				{
					b->setText("");
					b->setStyleSheet("background-color: rgb(189, 189, 189);");
				}
			}
		});
}
void Field::lookMines()
{
	if (saved)
		return;
	for (int32_t i = 0; i < szX; i++)
	{
		for (int32_t j = 0; j < szY; j++)
		{
			if (board[i][j]->countMine == -1)
			{
				if (board[i][j]->look)
				{
					if (board[i][j]->isFlag == 1)
					{
						board[i][j]->setText("|>");
						board[i][j]->setStyleSheet("color: red");
					}
					else if (board[i][j]->isFlag == 2)
					{
						board[i][j]->setText("?");
						board[i][j]->setStyleSheet("color: black");
					}
					else
					{
						board[i][j]->setText("");
						board[i][j]->setStyleSheet("background-color: rgb(189, 189, 189);");
					}
				}
				else
				{
					board[i][j]->setText("M");
					board[i][j]->setStyleSheet("background-color: rgb(200, 0, 0);");
				}
				board[i][j]->look = !board[i][j]->look;
			}
		}
	}
}
void Field::leftClick(int16_t x, int16_t y)
{
	if (leftMd)
	{
		changeMode();
		rightClick(x, y);
		changeMode();
		return;
	}
	Cell *button = board[x][y];
	if (!button->isFlag && button->covered)
	{
		emit changeCounter(numMines - numFlags);
		button->covered = false;
		++countOpen;
		if (firstClick)
		{
			generateBoard();
			firstClick = false;
		}
		if (button->countMine == -1)
		{
			lose(button);
			return;
		}
		else if (button->countMine == 0)
		{
			--countOpen;
			recurOpenCells(button->x, button->y);
		}
		else
		{
			button->paintNums();
		}
	}
	if ((int32_t)szX * szY == numMines + countOpen)
		win();
}
void Field::rightClick(int16_t x, int16_t y)
{
	if (leftMd)
	{
		changeMode();
		leftClick(x, y);
		changeMode();
		return;
	}
	Cell *button = board[x][y];
	if (button->covered && countOpen)
	{
		button->isFlag = (button->isFlag + 1) % 3;
		if (button->look)
			return;
		if (button->isFlag == 1)
		{
			++numFlags;
			emit changeCounter(numMines - numFlags);
			button->setText("|>");
			button->setStyleSheet("color: red");
		}
		else if (button->isFlag == 2)
		{
			--numFlags;
			emit changeCounter(numMines - numFlags);
			button->setText("?");
			button->setStyleSheet("color: black");
		}
		else
		{
			button->setText("");
			button->setStyleSheet("background-color: rgb(170, 170, 170);");
		}
	}
}
void Field::midClick()
{
	Cell *button = qobject_cast< Cell * >(sender());
	if (!button->covered && button->countMine > 0)
	{
		int32_t count = 0;
		int16_t i = button->x;
		int16_t j = button->y;
		for (int8_t di = -1; di < 2; di++)
			for (int8_t dj = -1; dj < 2; dj++)
				if (0 <= i + di && i + di < szX && 0 <= j + dj && j + dj < szY && board[i + di][j + dj]->isFlag == 1)
					count++;
		if (count != button->countMine)
		{
			openNeighbours(button);
			return;
		}
		for (int8_t di = -1; di < 2; di++)
		{
			for (int8_t dj = -1; dj < 2; dj++)
			{
				if (!(0 <= i + di && i + di < szX && 0 <= j + dj && j + dj < szY && board[i + di][j + dj]->covered))
					continue;
				if (board[i + di][j + dj]->countMine == -1)
				{
					if (!board[i + di][j + dj]->isFlag)
					{
						countOpen = -1;
						lose(board[i + di][j + dj]);
					}
					continue;
				}
				board[i + di][j + dj]->covered = true;
				if (board[i + di][j + dj]->countMine > 0)
				{
					board[i + di][j + dj]->paintNums();
					countOpen++;
				}
				else
					recurOpenCells(board[i + di][j + dj]->x, board[i + di][j + dj]->y);
			}
		}
		if ((int32_t)szX * szY == numMines + countOpen)
			win();
	}
}

void Field::saveGame()
{
	QSettings settings("save.ini", QSettings::IniFormat);
	if (saved)
	{
		settings.clear();
		return;
	}
	settings.beginGroup("Game");
	settings.setValue("isnSaved", false);
	settings.setValue("countOpen", countOpen);
	settings.setValue("firstClick", firstClick);
	settings.setValue("szX", szX);
	settings.setValue("szY", szY);
	settings.setValue("numMines", numMines);
	settings.setValue("numFlags", numFlags);
	settings.setValue("leftMd", leftMd);
	settings.endGroup();

	for (int32_t i = 0; i < szX; ++i)
	{
		for (int32_t j = 0; j < szY; ++j)
		{
			settings.beginGroup(QString("Cell_%1_%2").arg(i).arg(j));
			settings.setValue("covered", board[i][j]->covered);
			settings.setValue("isFlag", board[i][j]->isFlag);
			settings.setValue("countMine", board[i][j]->countMine);
			settings.setValue("x", board[i][j]->x);
			settings.setValue("y", board[i][j]->y);
			settings.endGroup();
		}
	}
}
void Field::changeMode()
{
	leftMd = !leftMd;
}
bool Field::getFlag()
{
	return firstClick;
}
