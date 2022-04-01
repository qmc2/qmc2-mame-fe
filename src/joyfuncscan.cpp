#if QMC2_JOYSTICK == 1

#include "settings.h"
#include "joyfuncscan.h"
#include "macros.h"
#include "options.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;

JoystickFunctionScanner::JoystickFunctionScanner(Joystick *joystick, bool showClearButton, QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	if ( !showClearButton )
		pushButtonClear->hide();

	clearClicked = false;
	joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
	labelStatus->setText(tr("Scanning joystick function"));
	setWindowTitle(tr("Scanning joystick function"));

	// connect joystick callbacks
	connect(joystick, SIGNAL(axisValueChanged(int, int)), this, SLOT(on_joystickAxisValueChanged(int, int)));
	connect(joystick, SIGNAL(buttonValueChanged(int, bool)), this, SLOT(on_joystickButtonValueChanged(int, bool)));
	connect(joystick, SIGNAL(hatValueChanged(int, int)), this, SLOT(on_joystickHatValueChanged(int, int)));
	connect(joystick, SIGNAL(trackballValueChanged(int, int, int)), this, SLOT(on_joystickTrackballValueChanged(int, int, int)));

	animSeq = 0;
	animTimer = new QTimer(this);
	connect(animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
	QTimer::singleShot(0, this, SLOT(animationTimeout()));
}

JoystickFunctionScanner::~JoystickFunctionScanner()
{
	delete animTimer;
}

void JoystickFunctionScanner::animationTimeout()
{
	switch ( animSeq ) {
		case 0:
			labelJoystickFunction->setText("<   >");
			break;
		case 1:
			labelJoystickFunction->setText("< <   > >");
			break;
		case 2:
			labelJoystickFunction->setText("< < <   > > >");
			break;
		case 3:
			labelJoystickFunction->setText("< < < <   > > > >");
			break;
		case 4:
			labelJoystickFunction->setText("< < < < <   > > > > >");
			break;
		case 5:
			labelJoystickFunction->setText("> > > > >   < < < < <");
			break;
		case 6:
			labelJoystickFunction->setText("> > > >   < < < <");
			break;
		case 7:
			labelJoystickFunction->setText("> > >   < < <");
			break;
		case 8:
			labelJoystickFunction->setText("> >   < <");
			break;
		case 9:
			labelJoystickFunction->setText(">   <");
			break;
	}

	if ( ++animSeq > 9 )
		animSeq = 0;

	animTimer->start(QMC2_ANIMATION_TIMEOUT);
}

void JoystickFunctionScanner::on_joystickAxisValueChanged(int axis, int value)
{
	if ( qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(axis), true).toBool() )
		if ( value != 0 ) {
			animTimer->stop();
			if ( value < 0 )
				labelJoystickFunction->setText(QString("A%1-").arg(axis));
			else
				labelJoystickFunction->setText(QString("A%1+").arg(axis));
			pushButtonOk->setEnabled(!labelJoystickFunction->text().isEmpty());
		}
}

void JoystickFunctionScanner::on_joystickButtonValueChanged(int button, bool value)
{
	animTimer->stop();
	labelJoystickFunction->setText(QString("B%1").arg(button));
	pushButtonOk->setEnabled(true);
}

void JoystickFunctionScanner::on_joystickHatValueChanged(int hat, int value)
{
	if ( value != 0 ) {
		animTimer->stop();
		labelJoystickFunction->setText(QString("H%1:%2").arg(hat).arg(value));
		pushButtonOk->setEnabled(true);
	}
}

void JoystickFunctionScanner::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
	animTimer->stop();
	labelJoystickFunction->setText(QString("T%1:X%2,Y%3").arg(trackball).arg(deltaX < 0 ? "-" : deltaX > 0 ? "+" : "=").arg(deltaY < 0 ? "-" : deltaY > 0 ? "+" : "="));
	pushButtonOk->setEnabled(true);
}

void JoystickFunctionScanner::closeEvent(QCloseEvent *e)
{
	e->accept();
}

#endif
