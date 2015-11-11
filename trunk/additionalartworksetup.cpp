#include <QLineEdit>
#include <QToolButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include <QApplication>
#include <QFileDialog>
#include <QList>
#include <QIcon>

#include "settings.h"
#include "additionalartworksetup.h"
#include "fileeditwidget.h"
#include "direditwidget.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;

AdditionalArtworkSetup::AdditionalArtworkSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
#if QT_VERSION < 0x050000
	treeWidget->header()->setMovable(false);
#else
	treeWidget->header()->setSectionsMovable(false);
#endif
	load();
}

AdditionalArtworkSetup::~AdditionalArtworkSetup()
{
}

void AdditionalArtworkSetup::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonRestore->setIconSize(iconSize);
	toolButtonAdd->setIconSize(iconSize);
	toolButtonRemove->setIconSize(iconSize);
}

void AdditionalArtworkSetup::on_pushButtonOk_clicked()
{
	save();
	accept();
}

void AdditionalArtworkSetup::on_pushButtonCancel_clicked()
{
	reject();
}

void AdditionalArtworkSetup::on_pushButtonRestore_clicked()
{
	load();
}

void AdditionalArtworkSetup::save()
{
	qmc2Config->remove("Artwork");
	qmc2Config->beginGroup("Artwork");
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QLineEdit *lineEditName = (QLineEdit *)treeWidget->itemWidget(item, QMC2_ADDITIONALARTWORK_COLUMN_NAME);
		QString name = lineEditName->text();
		if ( !name.isEmpty() ) {
			QToolButton *toolButtonIcon = (QToolButton *)treeWidget->itemWidget(item, QMC2_ADDITIONALARTWORK_COLUMN_ICON);
			if ( !toolButtonIcon->whatsThis().isEmpty() )
				qmc2Config->setValue(QString("%1/Icon").arg(name), toolButtonIcon->whatsThis());
			QComboBox *comboBoxTarget = (QComboBox *)treeWidget->itemWidget(item, QMC2_ADDITIONALARTWORK_COLUMN_TARGET);
			qmc2Config->setValue(QString("%1/Target").arg(name), comboBoxTarget->currentIndex());
			QComboBox *comboBoxType = (QComboBox *)treeWidget->itemWidget(item, QMC2_ADDITIONALARTWORK_COLUMN_TYPE);
			qmc2Config->setValue(QString("%1/Type").arg(name), comboBoxType->currentIndex());
			QStackedWidget *stackedWidgetFolderOrArchive = (QStackedWidget *)treeWidget->itemWidget(item, QMC2_ADDITIONALARTWORK_COLUMN_FOLDER_OR_ARCHIVE);
			DirectoryEditWidget *dewFolderOrArchive = (DirectoryEditWidget *)stackedWidgetFolderOrArchive->widget(QMC2_ADDITIONALARTWORK_INDEX_FOLDER);
			if ( !dewFolderOrArchive->lineEditDirectory->text().isEmpty() )
				qmc2Config->setValue(QString("%1/Folder").arg(name), dewFolderOrArchive->lineEditDirectory->text());
			FileEditWidget *fewFolderOrArchive = (FileEditWidget *)stackedWidgetFolderOrArchive->widget(QMC2_ADDITIONALARTWORK_INDEX_ARCHIVE);
			if ( !fewFolderOrArchive->lineEditFile->text().isEmpty() )
				qmc2Config->setValue(QString("%1/Archive").arg(name), fewFolderOrArchive->lineEditFile->text());
		}
	}
	qmc2Config->endGroup();
}

void AdditionalArtworkSetup::load()
{
	treeWidget->clear();
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups()) {
		QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidget);
		QCheckBox *checkBoxSelect = new QCheckBox(this);
		checkBoxSelect->setToolTip(tr("Select / deselect this artwork class for removal"));
		connect(checkBoxSelect, SIGNAL(toggled(bool)), this, SLOT(selectionFlagsChanged(bool)));
		treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_SELECT, checkBoxSelect);
		QLineEdit *lineEditName = new QLineEdit(this);
		lineEditName->setPlaceholderText(tr("Artwork name"));
		lineEditName->setToolTip(tr("Enter a name for this artwork class (required)"));
		lineEditName->setText(name);
		connect(lineEditName, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
		treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_NAME, lineEditName);
		QToolButton *toolButtonIcon = new QToolButton(this);
		toolButtonIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		toolButtonIcon->setText(tr("Choose..."));
		toolButtonIcon->setIconSize(iconSize);
		toolButtonIcon->setToolTip(tr("Choose an icon file to be used for this artwork class (optional)"));
		QString iconPath = qmc2Config->value(QString("%1/Icon").arg(name), QString()).toString();
		if ( !iconPath.isEmpty() ) {
			toolButtonIcon->setIcon(QIcon(iconPath));
			toolButtonIcon->setWhatsThis(iconPath);
		}
		connect(toolButtonIcon, SIGNAL(clicked()), this, SLOT(chooseIcon()));
		treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_ICON, toolButtonIcon);
		QComboBox *comboBoxTarget = new QComboBox(this);
		comboBoxTarget->addItem(tr("System"));
		comboBoxTarget->addItem(tr("Software"));
		comboBoxTarget->setToolTip(tr("Select system or software as <i>target</i> for this artwork class"));
		int index = qmc2Config->value(QString("%1/Target").arg(name), 0).toInt();
		if ( index >= 0 && index < 2 )
			comboBoxTarget->setCurrentIndex(index);
		connect(comboBoxTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
		treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_TARGET, comboBoxTarget);
		QComboBox *comboBoxType = new QComboBox(this);
		comboBoxType->addItem(tr("Folder"));
		comboBoxType->addItem(tr("Archive"));
		comboBoxType->setToolTip(tr("Choose if images are loaded from a folder or an archive for this artwork class"));
		index = qmc2Config->value(QString("%1/Type").arg(name), 0).toInt();
		if ( index >= 0 && index < 2 )
			comboBoxType->setCurrentIndex(index);
		connect(comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
		treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_TYPE, comboBoxType);
		FileEditWidget *fewFolderOrArchive = new FileEditWidget(QString(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files (*)"), QString(), this, false);
		fewFolderOrArchive->lineEditFile->setPlaceholderText(tr("Image archive"));
		fewFolderOrArchive->lineEditFile->setToolTip(tr("Image archive for this artwork class (required)"));
		fewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image archive"));
		fewFolderOrArchive->lineEditFile->setText(qmc2Config->value(QString("%1/Archive").arg(name), QString()).toString());
		connect(fewFolderOrArchive->lineEditFile, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
		DirectoryEditWidget *dewFolderOrArchive = new DirectoryEditWidget(QString(), this);
		dewFolderOrArchive->lineEditDirectory->setPlaceholderText(tr("Image folder"));
		dewFolderOrArchive->lineEditDirectory->setToolTip(tr("Image folder for this artwork class (required)"));
		dewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image folder"));
		dewFolderOrArchive->lineEditDirectory->setText(qmc2Config->value(QString("%1/Folder").arg(name), QString()).toString());
		connect(dewFolderOrArchive->lineEditDirectory, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
		QStackedWidget *stackedWidgetFolderOrArchive = new QStackedWidget(this);
		stackedWidgetFolderOrArchive->insertWidget(QMC2_ADDITIONALARTWORK_INDEX_FOLDER, dewFolderOrArchive);
		stackedWidgetFolderOrArchive->insertWidget(QMC2_ADDITIONALARTWORK_INDEX_ARCHIVE, fewFolderOrArchive);
		if ( index >= 0 && index < 2 )
			stackedWidgetFolderOrArchive->setCurrentIndex(index);
		treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_FOLDER_OR_ARCHIVE, stackedWidgetFolderOrArchive);
		connect(comboBoxType, SIGNAL(currentIndexChanged(int)), stackedWidgetFolderOrArchive, SLOT(setCurrentIndex(int)));
	}
	qmc2Config->endGroup();
	treeWidget->resizeColumnToContents(QMC2_ADDITIONALARTWORK_COLUMN_SELECT);
	pushButtonRestore->setEnabled(false);
}

void AdditionalArtworkSetup::on_toolButtonAdd_clicked()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidget);
	QCheckBox *checkBoxSelect = new QCheckBox(this);
	checkBoxSelect->setToolTip(tr("Select / deselect this artwork class for removal"));
	connect(checkBoxSelect, SIGNAL(toggled(bool)), this, SLOT(selectionFlagsChanged(bool)));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_SELECT, checkBoxSelect);
	QLineEdit *lineEditName = new QLineEdit(this);
	lineEditName->setPlaceholderText(tr("Artwork name"));
	lineEditName->setToolTip(tr("Enter a name for this artwork class (required)"));
	connect(lineEditName, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_NAME, lineEditName);
	QToolButton *toolButtonIcon = new QToolButton(this);
	toolButtonIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	toolButtonIcon->setText(tr("Choose..."));
	toolButtonIcon->setIconSize(iconSize);
	toolButtonIcon->setToolTip(tr("Choose an icon file to be used for this artwork class (optional)"));
	connect(toolButtonIcon, SIGNAL(clicked()), this, SLOT(chooseIcon()));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_ICON, toolButtonIcon);
	QComboBox *comboBoxTarget = new QComboBox(this);
	comboBoxTarget->addItem(tr("System"));
	comboBoxTarget->addItem(tr("Software"));
	comboBoxTarget->setToolTip(tr("Select system or software as <i>target</i> for this artwork class"));
	connect(comboBoxTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_TARGET, comboBoxTarget);
	QComboBox *comboBoxType = new QComboBox(this);
	comboBoxType->addItem(tr("Folder"));
	comboBoxType->addItem(tr("Archive"));
	comboBoxType->setToolTip(tr("Choose if images are loaded from a folder or an archive for this artwork class"));
	connect(comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_TYPE, comboBoxType);
	FileEditWidget *fewFolderOrArchive = new FileEditWidget(QString(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files (*)"), QString(), this, false);
	fewFolderOrArchive->lineEditFile->setPlaceholderText(tr("Image archive"));
	fewFolderOrArchive->lineEditFile->setToolTip(tr("Image archive for this artwork class (required)"));
	fewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image archive"));
	connect(fewFolderOrArchive->lineEditFile, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
	DirectoryEditWidget *dewFolderOrArchive = new DirectoryEditWidget(QString(), this);
	dewFolderOrArchive->lineEditDirectory->setPlaceholderText(tr("Image folder"));
	dewFolderOrArchive->lineEditDirectory->setToolTip(tr("Image folder for this artwork class (required)"));
	dewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image folder"));
	connect(dewFolderOrArchive->lineEditDirectory, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
	QStackedWidget *stackedWidgetFolderOrArchive = new QStackedWidget(this);
	stackedWidgetFolderOrArchive->insertWidget(QMC2_ADDITIONALARTWORK_INDEX_FOLDER, dewFolderOrArchive);
	stackedWidgetFolderOrArchive->insertWidget(QMC2_ADDITIONALARTWORK_INDEX_ARCHIVE, fewFolderOrArchive);
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_FOLDER_OR_ARCHIVE, stackedWidgetFolderOrArchive);
	connect(comboBoxType, SIGNAL(currentIndexChanged(int)), stackedWidgetFolderOrArchive, SLOT(setCurrentIndex(int)));
	treeWidget->resizeColumnToContents(QMC2_ADDITIONALARTWORK_COLUMN_SELECT);
	dataChanged(0);
}

void AdditionalArtworkSetup::on_toolButtonRemove_clicked()
{
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QCheckBox *cb = (QCheckBox *)treeWidget->itemWidget(item, QMC2_ADDITIONALARTWORK_COLUMN_SELECT);
		if ( cb && cb->isChecked() ) {
			item = treeWidget->takeTopLevelItem(i);
			delete item;
			i--;
		}
	}
	selectionFlagsChanged();
	dataChanged(0);
}

void AdditionalArtworkSetup::selectionFlagsChanged(bool)
{
	bool enable = false;
	for (int i = 0; i < treeWidget->topLevelItemCount() && !enable; i++) {
		QCheckBox *cb = (QCheckBox *)treeWidget->itemWidget(treeWidget->topLevelItem(i), QMC2_ADDITIONALARTWORK_COLUMN_SELECT);
		if ( cb )
			enable = cb->isChecked();
	}
	toolButtonRemove->setEnabled(enable);
}

void AdditionalArtworkSetup::dataChanged(const QString &)
{
	pushButtonRestore->setEnabled(true);
}

void AdditionalArtworkSetup::dataChanged(int)
{
	pushButtonRestore->setEnabled(true);
}

void AdditionalArtworkSetup::chooseIcon()
{
	QToolButton *tb = (QToolButton *)sender();
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose icon file"), tb->whatsThis(), tr("PNG files (*.png)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !fileName.isEmpty() ) {
		tb->setIcon(QIcon(fileName));
		tb->setWhatsThis(fileName);
		dataChanged(0);
	}
}

void AdditionalArtworkSetup::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	treeWidget->resizeColumnToContents(QMC2_ADDITIONALARTWORK_COLUMN_SELECT);
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/AdditionalArtworkSetup/Geometry", QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void AdditionalArtworkSetup::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/AdditionalArtworkSetup/Geometry", saveGeometry());
	on_pushButtonCancel_clicked();
	if ( e )
		QDialog::hideEvent(e);
}
