#if QMC2_JOYSTICK == 1

#ifndef _JOYFUNCSCAN_H_
#define _JOYFUNCSCAN_H_

#include <QTimer>
#include "joystick.h"
#include "ui_joyfuncscan.h"

class JoystickFunctionScanner : public QDialog, public Ui::JoystickFunctionScanner
{
	Q_OBJECT

	public:
		QTimer animTimer;
		int animSeq;
		int joyIndex;
		bool clearClicked;

		JoystickFunctionScanner(Joystick *joystick, bool showClearButton = false, QWidget *parent = 0);

	public slots:
		void animationTimeout();
		void on_joystickAxisValueChanged(int, int);
		void on_joystickButtonValueChanged(int, bool);
		void on_joystickHatValueChanged(int, int);
		void on_joystickTrackballValueChanged(int, int, int);
		void on_pushButtonClear_clicked() { clearClicked = true; }

	protected:
		void closeEvent(QCloseEvent *);
};

#endif

#endif
