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
	// FIXME
	accept();
}

void AdditionalArtworkSetup::on_pushButtonCancel_clicked()
{
	// FIXME
	reject();
}

void AdditionalArtworkSetup::on_pushButtonRestore_clicked()
{
	// FIXME
}

void AdditionalArtworkSetup::on_toolButtonAdd_clicked()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidget);

	QCheckBox *checkBoxSelect = new QCheckBox();
	checkBoxSelect->setToolTip(tr("Select / deselect this artwork class for removal"));
	connect(checkBoxSelect, SIGNAL(toggled(bool)), this, SLOT(selectionFlagsChanged(bool)));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_SELECT, checkBoxSelect);
	QLineEdit *lineEditName = new QLineEdit();
	lineEditName->setPlaceholderText(tr("Artwork name"));
	lineEditName->setToolTip(tr("Enter a name for this artwork class (required)"));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_NAME, lineEditName);
	QToolButton *toolButtonIcon = new QToolButton();
	toolButtonIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	toolButtonIcon->setText(tr("Choose..."));
	toolButtonIcon->setIconSize(iconSize);
	toolButtonIcon->setToolTip(tr("Choose an icon file to be used for this artwork class (optional)"));
	connect(toolButtonIcon, SIGNAL(clicked()), this, SLOT(chooseIcon()));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_ICON, toolButtonIcon);
	QLineEdit *lineEditCachePrefix = new QLineEdit();
	lineEditCachePrefix->setPlaceholderText(tr("Cache prefix"));
	lineEditCachePrefix->setToolTip(tr("Enter a <u>unique</u> cache prefix (required)"));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_CACHE_PREFIX, lineEditCachePrefix);
	QComboBox *comboBoxTarget = new QComboBox();
	comboBoxTarget->addItem(tr("System"));
	comboBoxTarget->addItem(tr("Software"));
	comboBoxTarget->setToolTip(tr("Select system or software as <i>target</i> for this artwork class"));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_TARGET, comboBoxTarget);
	QComboBox *comboBoxType = new QComboBox();
	comboBoxType->addItem(tr("Folder"));
	comboBoxType->addItem(tr("ZIP"));
	comboBoxType->setToolTip(tr("Choose if images are loaded from a folder or a ZIP archive for this artwork class"));
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_TYPE, comboBoxType);
	FileEditWidget *fewFolderOrZip = new FileEditWidget("", tr("ZIP files (*.zip)") + ";;" + tr("All files (*)"), "", 0, false);
	fewFolderOrZip->lineEditFile->setPlaceholderText(tr("Image ZIP file"));
	fewFolderOrZip->lineEditFile->setToolTip(tr("Image ZIP file for this artwork class (required)"));
	fewFolderOrZip->toolButtonBrowse->setToolTip(tr("Browse image ZIP file"));
	DirectoryEditWidget *dewFolderOrZip = new DirectoryEditWidget("", 0);
	dewFolderOrZip->lineEditDirectory->setPlaceholderText(tr("Image folder"));
	dewFolderOrZip->lineEditDirectory->setToolTip(tr("Image folder for this artwork class (required)"));
	dewFolderOrZip->toolButtonBrowse->setToolTip(tr("Browse image folder"));
	QStackedWidget *stackedWidgetFolderOrZip = new QStackedWidget();
	stackedWidgetFolderOrZip->insertWidget(QMC2_ADDITIONALARTWORK_INDEX_FOLDER, dewFolderOrZip);
	stackedWidgetFolderOrZip->insertWidget(QMC2_ADDITIONALARTWORK_INDEX_ZIP, fewFolderOrZip);
	treeWidget->setItemWidget(newItem, QMC2_ADDITIONALARTWORK_COLUMN_FOLDER_OR_ZIP, stackedWidgetFolderOrZip);
	connect(comboBoxType, SIGNAL(currentIndexChanged(int)), stackedWidgetFolderOrZip, SLOT(setCurrentIndex(int)));

	treeWidget->resizeColumnToContents(QMC2_ADDITIONALARTWORK_COLUMN_SELECT);
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

void AdditionalArtworkSetup::chooseIcon()
{
	QToolButton *tb = (QToolButton *)sender();

	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose icon file"), tb->whatsThis(), tr("PNG files (*.png)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !fileName.isEmpty() ) {
		tb->setIcon(QIcon(fileName));
		tb->setWhatsThis(fileName);
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

void AdditionalArtworkSetup::resizeEvent(QResizeEvent *e)
{
	// FIXME
	if ( e )
		QDialog::resizeEvent(e);
}

void AdditionalArtworkSetup::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/AdditionalArtworkSetup/Geometry", saveGeometry());
	on_pushButtonCancel_clicked();

	if ( e )
		QDialog::hideEvent(e);
}
