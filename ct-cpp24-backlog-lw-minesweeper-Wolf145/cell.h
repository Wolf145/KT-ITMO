#ifndef CELL_H
#define CELL_H

#include <QMouseEvent>
#include <QPushButton>

class Cell : public QPushButton
{
	Q_OBJECT

  protected:
	void mousePressEvent(QMouseEvent *event) override;

  public:
	bool covered = true;
	bool look = false;
	int8_t isFlag = 0;
	int16_t countMine = 0;	  // countMine == -1 => this is a mine
	int16_t x, y;

	Cell(int16_t x, int16_t y, QWidget *parent = nullptr);
	Cell(QWidget *parent = nullptr);
	void paintNums();
  signals:
	void pressLeft(int16_t x, int16_t y);
	void pressRight(int16_t x, int16_t y);
	void pressMid();
  public slots:
};

#endif	  // CELL_H
