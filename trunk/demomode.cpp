#include <QMap>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QSettings>

#include "demomode.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;
extern QStringList qmc2BiosROMs;
extern QString qmc2DemoGame;
extern QStringList qmc2DemoArgs;
extern bool qmc2ReloadActive;
extern QSettings *qmc2Config;

DemoModeDialog::DemoModeDialog(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DemoModeDialog::DemoModeDialog(QWidget *parent = %1").arg((qulonglong) parent));
#endif

  setupUi(this);
  demoModeRunning = FALSE;
  emuProcess = NULL;
#if !defined(Q_WS_X11)
  checkBoxEmbedded->setVisible(FALSE);
#endif

  toolButtonSelectC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectC", TRUE).toBool());
  toolButtonSelectM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectM", TRUE).toBool());
  toolButtonSelectI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectI", FALSE).toBool());
  toolButtonSelectN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectN", FALSE).toBool());
  toolButtonSelectU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectU", FALSE).toBool());
  checkBoxFullScreen->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/FullScreen", TRUE).toBool());
  checkBoxMaximized->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/Maximized", FALSE).toBool());
#if defined(Q_WS_X11)
  checkBoxEmbedded->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/Embedded", FALSE).toBool());
#endif
  spinBoxSecondsToRun->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SecondsToRun", 60).toInt());
  spinBoxPauseSeconds->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/PauseSeconds", 2).toInt());
}

DemoModeDialog::~DemoModeDialog()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::~DemoModeDialog()");
#endif

}

void DemoModeDialog::showEvent(QShowEvent *)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DemoModeDialog::showEvent(QShowEvent *e = %1)").arg((qulonglong) e));
#endif

  // try to "grab" the input focus...
  activateWindow();
  setFocus();
}

void DemoModeDialog::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DemoModeDialog::closeEvent(QCloseEvent *e = %1)").arg((qulonglong) e));
#endif

  if ( demoModeRunning )
    pushButtonRunDemo->animateClick();

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectC", toolButtonSelectC->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectM", toolButtonSelectM->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectI", toolButtonSelectI->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectN", toolButtonSelectN->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectU", toolButtonSelectU->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/FullScreen", checkBoxFullScreen->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/Maximized", checkBoxMaximized->isChecked());
#if defined(Q_WS_X11)
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/Embedded",checkBoxEmbedded->isChecked());
#endif
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SecondsToRun", spinBoxSecondsToRun->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/PauseSeconds", spinBoxPauseSeconds->value());
}

void DemoModeDialog::on_pushButtonRunDemo_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::on_pushButtonRunDemo_clicked()");
#endif

  if ( demoModeRunning ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("demo mode stopped"));
    demoModeRunning = FALSE;
    pushButtonRunDemo->setText(tr("Run &demo"));
    pushButtonRunDemo->setToolTip(tr("Run demo now"));
    qmc2DemoGame = "";
    qmc2DemoArgs.clear();
    if ( emuProcess ) {
      emuProcess->terminate();
      emuProcess = NULL;
    }
  } else {
    if ( qmc2ReloadActive ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
      return;
    }
    selectedGames.clear();
    foreach (QString game, qmc2GamelistItemMap.keys()) {
      if ( qmc2BiosROMs.contains(game) ) continue;
      QTreeWidgetItem *gameItem = qmc2GamelistItemMap[game];
      switch ( gameItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
        case QMC2_ROMSTATE_CHAR_C:
          if ( toolButtonSelectC->isChecked() ) selectedGames << game;
          break;

        case QMC2_ROMSTATE_CHAR_M:
          if ( toolButtonSelectM->isChecked() ) selectedGames << game;
          break;

        case QMC2_ROMSTATE_CHAR_I:
          if ( toolButtonSelectI->isChecked() ) selectedGames << game;
          break;

        case QMC2_ROMSTATE_CHAR_N:
          if ( toolButtonSelectN->isChecked() ) selectedGames << game;
          break;

        case QMC2_ROMSTATE_CHAR_U:
        default:
          if ( toolButtonSelectU->isChecked() ) selectedGames << game;
          break;
      }
    }
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("demo mode started -- %n game(s) selected by filter", "", selectedGames.count()));
    demoModeRunning = TRUE;
    pushButtonRunDemo->setText(tr("Stop &demo"));
    pushButtonRunDemo->setToolTip(tr("Stop demo now"));
    QTimer::singleShot(0, this, SLOT(startNextEmu()));
  }
}

void DemoModeDialog::emuStarted()
{
  emuProcess = (QProcess *)sender();
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::emuStarted()");
#endif

}

void DemoModeDialog::emuFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::emuFinished(int exitCode = ..., QProcess::ExitStatus exitStatus = ...)");
#endif

  // try to "grab" the input focus...
  activateWindow();
  setFocus();

  qmc2DemoArgs.clear();
  qmc2DemoGame.clear();
  emuProcess = NULL;

  if ( demoModeRunning )
    QTimer::singleShot(spinBoxPauseSeconds->value() * 1000, this, SLOT(startNextEmu()));
}

void DemoModeDialog::startNextEmu()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::startNextEmu()");
#endif

  if ( !demoModeRunning )
    return;

  qmc2DemoArgs.clear();
  qmc2DemoArgs << "-str" << QString::number(spinBoxSecondsToRun->value());
  emuProcess = NULL;
  if ( checkBoxFullScreen->isChecked() ) {
    qmc2DemoArgs << "-nowindow";
  } else {
    qmc2DemoArgs << "-window";
    if ( checkBoxMaximized )
      qmc2DemoArgs << "-maximize";
    else
      qmc2DemoArgs << "-nomaximize";
  }
  qmc2DemoGame = selectedGames[qrand() % selectedGames.count()];
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("starting emulation in demo mode for '%1'").arg(qmc2GamelistDescriptionMap[qmc2DemoGame]));
#if defined(Q_WS_X11)
  if ( checkBoxEmbedded->isChecked() && !checkBoxFullScreen->isChecked() )
    QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
  else
    QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
#else
  QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
#endif
}

void DemoModeDialog::adjustIconSizes()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::adjustIconSizes()");
#endif

  QFontMetrics fm(qApp->font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);

  toolButtonSelectC->setIconSize(iconSize);
  toolButtonSelectM->setIconSize(iconSize);
  toolButtonSelectI->setIconSize(iconSize);
  toolButtonSelectN->setIconSize(iconSize);
  toolButtonSelectU->setIconSize(iconSize);

  adjustSize();
}
