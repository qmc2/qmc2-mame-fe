#include <QFileDialog>
#include <QSettings>
#include <QImage>
#include "arcade/arcadesetupdialog.h"
#include "arcade/arcadeview.h"
#include "macros.h"
#include "qmc2main.h"

// extern global variables
extern MainWindow *qmc2MainWindow;
extern bool qmc2CleaningUp;
extern QSettings *qmc2Config;
extern ArcadeView *qmc2ArcadeView;

ArcadeSetupDialog::ArcadeSetupDialog(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSetupDialog::ArcadeSetupDialog(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setupUi(this);

#if defined(QMC2_EMUTYPE_MESS)
  groupBoxGamelist->setTitle(tr("Machine list"));
  groupBoxGamelist->setToolTip(tr("Control display of machine list"));
#endif

  adjustSize();

  qApp->processEvents();

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
    QStringList keys = qmc2Config->allKeys();
    if ( keys.contains(QMC2_ARCADE_PREFIX + "ArcadeSetup/Size") )
      resize(qmc2Config->value(QMC2_ARCADE_PREFIX + "ArcadeSetup/Size").toSize());
    if ( keys.contains(QMC2_ARCADE_PREFIX + "ArcadeSetup/Position") )
      move(qmc2Config->value(QMC2_ARCADE_PREFIX + "ArcadeSetup/Position").toPoint());
  }

  if ( qmc2ArcadeView ) {
    if ( qmc2ArcadeView->isVisible() && !qmc2ArcadeView->isFullScreen() ) {
      spinBoxGraphicsWindowPosX->setValue(qmc2ArcadeView->x());
      spinBoxGraphicsWindowPosY->setValue(qmc2ArcadeView->y());
      spinBoxGraphicsWindowWidth->setValue(qmc2ArcadeView->width());
      spinBoxGraphicsWindowHeight->setValue(qmc2ArcadeView->height());
    }
    checkBoxGraphicsFullscreen->setChecked(qmc2ArcadeView->isFullScreen());
  }
}

ArcadeSetupDialog::~ArcadeSetupDialog()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeSetupDialog::~ArcadeSetupDialog()");
#endif

}

void ArcadeSetupDialog::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSetupDialog::closeEvent(QCloseEvent *e = %1)").arg((qulonglong) e));
#endif

  e->accept();
}

void ArcadeSetupDialog::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSetupDialog::showEvent(QShowEvent *e = %1)").arg((qulonglong) e));
#endif

  e->accept();
}

void ArcadeSetupDialog::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSetupDialog::hideEvent(QHideEvent *e = %1)").arg((qulonglong) e));
#endif

  e->accept();
}

void ArcadeSetupDialog::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSetupDialog::moveEvent(QMoveEvent *e = %1)").arg((qulonglong) e));
#endif

  if ( !qmc2CleaningUp && isVisible() )
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      qmc2Config->setValue(QMC2_ARCADE_PREFIX + "ArcadeSetup/Position", pos());

  e->accept();
}

void ArcadeSetupDialog::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSetupDialog::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong) e));
#endif

  if ( !qmc2CleaningUp && isVisible() )
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      qmc2Config->setValue(QMC2_ARCADE_PREFIX + "ArcadeSetup/Size", size());

  e->accept();
}
