#if QMC2_JOYSTICK == 1

#include <QSettings>
#include "joyfuncscan.h"
#include "macros.h"
#include "options.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern QSettings *qmc2Config;

JoystickFunctionScanner::JoystickFunctionScanner(Joystick *joystick, QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickFunctionScanner::JoystickFunctionScanner(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

  setupUi(this);

  joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
  labelStatus->setText(tr("Scanning joystick function"));
  setWindowTitle(tr("Scanning joystick function"));
  animSeq = 0;
  animationTimeout();
  connect(&animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
  animTimer.start(QMC2_ANIMATION_TIMEOUT);

  // connect joystick callbacks
  connect(joystick, SIGNAL(axisValueChanged(int, int)), this, SLOT(on_joystickAxisValueChanged(int, int)));
  connect(joystick, SIGNAL(buttonValueChanged(int, bool)), this, SLOT(on_joystickButtonValueChanged(int, bool)));
  connect(joystick, SIGNAL(hatValueChanged(int, int)), this, SLOT(on_joystickHatValueChanged(int, int)));
  connect(joystick, SIGNAL(trackballValueChanged(int, int, int)), this, SLOT(on_joystickTrackballValueChanged(int, int, int)));
}

JoystickFunctionScanner::~JoystickFunctionScanner()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: JoystickFunctionScanner::~JoystickFunctionScanner()");
#endif

}

void JoystickFunctionScanner::animationTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: JoystickFunctionScanner::animationTimeout()");
#endif

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
  animSeq++;
  if ( animSeq > 9 )
    animSeq = 0;
}

void JoystickFunctionScanner::on_joystickAxisValueChanged(int axis, int value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickFunctionScanner::on_joystickAxisValueChanged(int axis = %1, int value = %2)").arg(axis).arg(value));
#endif

  if ( qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(axis), true).toBool() ) {
    animTimer.stop();
    if ( value < 0 )
      labelJoystickFunction->setText(QString("A%1-").arg(axis));
    else if ( value > 0 )
      labelJoystickFunction->setText(QString("A%1+").arg(axis));
    pushButtonOk->setEnabled(!labelJoystickFunction->text().isEmpty());
  }
}

void JoystickFunctionScanner::on_joystickButtonValueChanged(int button, bool value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickFunctionScanner::on_joystickButtonValueChanged(int button = %1, bool value = %2)").arg(button).arg(value));
#endif

  animTimer.stop();
  labelJoystickFunction->setText(QString("B%1").arg(button));
  pushButtonOk->setEnabled(true);
}

void JoystickFunctionScanner::on_joystickHatValueChanged(int hat, int value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickFunctionScanner::on_joystickHatValueChanged(int hat = %1, int value = %2)").arg(hat).arg(value));
#endif

  if ( value != 0 ) {
    animTimer.stop();
    labelJoystickFunction->setText(QString("H%1:%2").arg(hat).arg(value));
    pushButtonOk->setEnabled(true);
  }
}

void JoystickFunctionScanner::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickFunctionScanner::on_joystickTrackballValueChanged(int trackball = %1, int deltaX = %2, int deltaY = %3)").arg(trackball).arg(deltaX).arg(deltaY));
#endif

  animTimer.stop();
  labelJoystickFunction->setText(QString("T%1:X%2,Y%3").arg(trackball)
                                 .arg(deltaX < 0 ? "-" : deltaX > 0 ? "+" : "=")
                                 .arg(deltaY < 0 ? "-" : deltaY > 0 ? "+" : "="));
  pushButtonOk->setEnabled(true);
}

void JoystickFunctionScanner::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickFunctionScanner::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  e->accept();
}

#endif
