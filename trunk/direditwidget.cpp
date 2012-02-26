#include <QApplication>
#include <QFileDialog>
#include "direditwidget.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif
extern QString qmc2DirectoryEditStartPath;

DirectoryEditWidget::DirectoryEditWidget(QString dirPath, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DirectoryEditWidget::DirectoryEditWidget(QString filePath = %1, QWidget *parent = %2)").arg(dirPath).arg((qulonglong) parent));
#endif

	setupUi(this);

	lineEditDirectory->setText(dirPath);
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonBrowse->setIconSize(iconSize);
}

DirectoryEditWidget::~DirectoryEditWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DirectoryEditWidget::~DirectoryEditWidget()");
#endif

}

void DirectoryEditWidget::on_toolButtonBrowse_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DirectoryEditWidget::on_toolButtonBrowse_clicked()");
#endif

	QString startPath = lineEditDirectory->text();
	if ( startPath.isEmpty() )
		startPath = qmc2DirectoryEditStartPath;
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose directory"), startPath);
	if ( !s.isEmpty() )
		lineEditDirectory->setText(s);
}
