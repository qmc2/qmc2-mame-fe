#include <QApplication>
#include <QFileDialog>
#include <QDir>
#include "direditwidget.h"

#include "qmc2main.h"
#include "options.h"
#include "settings.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QString qmc2DirectoryEditStartPath;
extern Options *qmc2Options;
extern Settings *qmc2Config;

DirectoryEditWidget::DirectoryEditWidget(QString dirPath, QWidget *parent, QTreeWidget *treeWidget)
	: QWidget(parent)
{
	setupUi(this);
	lineEditDirectory->setText(dirPath);
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonBrowse->setIconSize(iconSize);
	myTreeWidget = treeWidget;
}

DirectoryEditWidget::~DirectoryEditWidget()
{
	// NOP
}

void DirectoryEditWidget::on_toolButtonBrowse_clicked()
{
	QString startPath = lineEditDirectory->text();
	if ( startPath.isEmpty() )
		startPath = qmc2DirectoryEditStartPath;
	if ( myTreeWidget != 0 ) {
		QDir startDir(startPath);
		if ( startDir.isRelative() ) {
			if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory") ) {
				QString workPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString();
				if ( !workPath.isEmpty() )
					startPath = QDir::cleanPath(workPath + "/" + startPath);
			} else
				startPath = QDir::cleanPath(QDir::currentPath() + "/" + startPath);
		}
	}
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose directory"), startPath, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isEmpty() )
		lineEditDirectory->setText(s);
}
