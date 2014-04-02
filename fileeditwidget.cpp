#include <QApplication>
#include <QFileDialog>
#include <QTreeWidgetItem>
#include <QDir>
#include <QFileInfo>

#include "fileeditwidget.h"
#include "settings.h"
#include "qmc2main.h"
#include "options.h"
#include "emuopt.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern QString qmc2FileEditStartPath;
extern Options *qmc2Options;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;

FileEditWidget::FileEditWidget(QString filePath, QString filter, QString part, QWidget *parent, bool showClearButton, QString relativeTo, QTreeWidget *treeWidget)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: FileEditWidget::FileEditWidget(QString filePath = %1, QString filter = %2, QString part = %3, QWidget *parent = %4, bool showClearButton = %5, QString relativeTo = %6, QTreeWidget *treeWidget = %7)").arg(filePath).arg(filter).arg(part).arg((qulonglong)parent).arg(showClearButton).arg(relativeTo).arg((qulonglong)treeWidget));
#endif

	setupUi(this);

	if ( !showClearButton )
		toolButtonClear->hide();

	relativeToFolderOption = relativeTo;
	lineEditFile->setText(filePath);
	browserFilter = filter;
	browserPart = part;
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonBrowse->setIconSize(iconSize);
	toolButtonClear->setIconSize(iconSize);
	myTreeWidget = treeWidget;
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

	if ( returnRelativePath() ) {
		QDir relativeToDir(relativeToPath());
		QString relToPath = relativeToDir.path();
		if ( relativeToDir.isRelative() ) {
			if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory") ) {
				QString workPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString();
				if ( !workPath.isEmpty() )
					startPath = QDir::cleanPath(workPath + "/" + relToPath + "/" + startPath);
				else
					startPath = QDir::cleanPath(QDir::currentPath() + "/" + relToPath + "/" + startPath);
			} else
				startPath = QDir::cleanPath(QDir::currentPath() + "/" + relToPath + "/" + startPath);
		} else
			startPath = QDir::cleanPath(relativeToPath() + "/" + startPath);
	}

	if ( startPath.isEmpty() )
		startPath = qmc2FileEditStartPath;

	if ( startPath.isEmpty() )
		startPath = QDir::currentPath();

	QString s;
	if ( toolButtonClear->isVisible() )
		s = QFileDialog::getOpenFileName(this, tr("Choose file"), startPath, browserFilter, 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	else
		s = QFileDialog::getSaveFileName(this, tr("Choose file"), startPath, browserFilter, 0, QFileDialog::DontConfirmOverwrite | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));

	if ( !s.isEmpty() ) {
		if ( returnRelativePath() ) {
			QDir relativeToDir(relativeToPath());
			if ( relativeToDir.isRelative() ) {
				if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory") ) {
					QString workPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString();
					if ( !workPath.isEmpty() ) {
						QDir fullRelDir(QDir::cleanPath(workPath + "/" + relativeToDir.path()));
						s = QDir::cleanPath(fullRelDir.relativeFilePath(s));
					} else
						s = QDir::cleanPath(relativeToDir.relativeFilePath(s));
				} else
					s = QDir::cleanPath(relativeToDir.relativeFilePath(s));
			} else
				s = QDir::cleanPath(relativeToDir.relativeFilePath(s));
		} else if ( browserPart == "basename" ) {
			QFileInfo fi(s);
			s = fi.baseName();
		}
		lineEditFile->setText(s);
	}
}

QString FileEditWidget::relativeToPath()
{
	if ( returnRelativePath() ) {
		if ( relativeToFolderOption == "emulatorWorkingDirectory" ) {
			if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory") )
				return QDir::cleanPath(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString());
			else
				return QDir::currentPath();
		} else {
			QList<QTreeWidgetItem *> itemList;
			if ( myTreeWidget == qmc2GlobalEmulatorOptions )
				itemList = qmc2GlobalEmulatorOptions->findItems(relativeToFolderOption, Qt::MatchRecursive | Qt::MatchExactly, QMC2_EMUOPT_COLUMN_OPTION);
			else if ( myTreeWidget == qmc2EmulatorOptions )
				itemList = qmc2EmulatorOptions->findItems(relativeToFolderOption, Qt::MatchRecursive | Qt::MatchExactly, QMC2_EMUOPT_COLUMN_OPTION);
			if ( !itemList.isEmpty() )
				return itemList[0]->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
			else
				return QString();
		}
	} else
		return QString();
}
