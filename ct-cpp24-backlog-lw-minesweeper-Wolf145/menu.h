#ifndef MENU_H
#define MENU_H

#include <QGridLayout>
#include <QMessageBox>
#include <QSpinBox>

class Menu : public QDialog
{
	Q_OBJECT

  private:
	QSpinBox *sizeX, *sizeY, *numberMines;
	QVBoxLayout *screen;
	QLabel *lbl1, *lbl2, *lbl3, *lbl4;
	QPushButton *startGame;
	QHBoxLayout *Sx, *Sy;

  public:
	Menu(QWidget *parent = nullptr);
	int32_t getX() const { return sizeX->value(); }
	int32_t getY() const { return sizeY->value(); }
	int32_t getM() const { return numberMines->value(); }

  private slots:
	void updateMineRange();
};

#endif	  // MENU_H
