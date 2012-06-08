#include <QApplication>
#include <QFileDialog>
#include "fileeditwidget.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif
extern QString qmc2FileEditStartPath;

FileEditWidget::FileEditWidget(QString filePath, QString filter, QString part, QWidget *parent, bool showClearButton)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: FileEditWidget::FileEditWidget(QString filePath = %1, QString filter = %2, QString part = %3, QWidget *parent = %4, bool showClearButton = %5)").arg(filePath).arg(filter).arg(part).arg((qulonglong) parent).arg(showClearButton));
#endif

	setupUi(this);

	if ( !showClearButton )
		toolButtonClear->hide();

	lineEditFile->setText(filePath);
	browserFilter = filter;
	browserPart = part;
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonBrowse->setIconSize(iconSize);
	toolButtonClear->setIconSize(iconSize);
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
	QString s = QFileDialog::getSaveFileName(this, tr("Choose file"), startPath, browserFilter);
	if ( !s.isEmpty() ) {
		if ( browserPart == "basename" ) {
			QFileInfo fi(s);
			s = fi.baseName();
		}
		lineEditFile->setText(s);
	}
}
