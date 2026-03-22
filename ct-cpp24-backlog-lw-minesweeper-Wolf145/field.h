#ifndef FIELD_H
#define FIELD_H

#include "QtWidgets/qmessagebox.h"
#include "cell.h"

#include <QGridLayout>

class Field : public QWidget
{
	Q_OBJECT
	int32_t countOpen = 0;
	bool firstClick = true, saved = false, leftMd = false;
	QVector< QVector< Cell * > > board;
	QGridLayout *table;
	QMessageBox *info;

  public:
	int16_t szX, szY;
	int32_t numMines, numFlags;

	Field(int16_t x, int16_t y, int32_t m, QWidget *parent = nullptr);
	Field(const QString &saveFileName, QWidget *parent = nullptr);
	bool getFlag();

  private:
	void createField();
	void generateBoard();
	void recurOpenCells(int16_t x, int16_t y);
	void lose(Cell *button);
	void win();
	void openNeighbours(Cell *button);
	void setSave() { saved = false; }
  signals:
	void changeCounter(int val);
  public slots:
	void lookMines();
	void leftClick(int16_t x, int16_t y);
	void rightClick(int16_t x, int16_t y);
	void midClick();
	void saveGame();
	void changeMode();
};

#endif	  // FIELD_H
