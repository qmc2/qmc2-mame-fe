#include <QSettings>

#include "mawsqdlsetup.h"
#include "macros.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

MawsQuickDownloadSetup::MawsQuickDownloadSetup(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MawsQuickDownloadSetup::MawsQuickDownloadSetup(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setupUi(this);
  adjustSize();
}

MawsQuickDownloadSetup::~MawsQuickDownloadSetup()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::~MawsQuickDownloadSetup()");
#endif

}

void MawsQuickDownloadSetup::on_pushButtonOk_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_pushButtonOk_clicked()");
#endif

}

void MawsQuickDownloadSetup::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_pushButtonCancel_clicked()");
#endif

}
