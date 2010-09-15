#include <QApplication>
#include <QFileDialog>
#include "fileeditwidget.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif
extern QString qmc2FileEditStartPath;

FileEditWidget::FileEditWidget(QString filePath, QString filter, QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: FileEditWidget::FileEditWidget(QString filePath = %1, QString filter = %2, QWidget *parent = %3)").arg(filePath).arg(filter).arg((qulonglong) parent));
#endif

  setupUi(this);

  lineEditFile->setText(filePath);
  browserFilter = filter;
  QFontMetrics fm(QApplication::font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);
  toolButtonBrowse->setIconSize(iconSize);
}

FileEditWidget::~FileEditWidget()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: FileEditWidget::~FileEditWidget()");
#endif

}

void FileEditWidget::on_toolButtonBrowse_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: FileEditWidget::on_toolButtonBrowse_clicked()");
#endif

  QString startPath = lineEditFile->text();
  if ( startPath.isEmpty() )
    startPath = qmc2FileEditStartPath;
  QString s = QFileDialog::getOpenFileName(this, tr("Choose file"), startPath, browserFilter);
  if ( !s.isEmpty() )
    lineEditFile->setText(s);
}
