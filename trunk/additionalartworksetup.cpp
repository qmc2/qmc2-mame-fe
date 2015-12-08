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
#include "componentsetup.h"
#include "customartwork.h"
#include "customsoftwareartwork.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern ComponentSetup *qmc2ComponentSetup;

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
	ComponentInfo *component2Info = qmc2ComponentSetup->componentInfoHash()["Component2"];
	int deleteAfterIndex = component2Info->availableFeatureList().indexOf(QMC2_SYSTEM_NOTES_INDEX);
	for (int i = component2Info->availableFeatureList().count(); i > deleteAfterIndex; i--)
		component2Info->availableFeatureList().removeAt(i);
	ComponentInfo *component4Info = qmc2ComponentSetup->componentInfoHash()["Component4"];
	deleteAfterIndex = component4Info->availableFeatureList().indexOf(QMC2_SWINFO_INFO_PAGE);
	for (int i = component4Info->availableFeatureList().count(); i > deleteAfterIndex; i--)
		component4Info->availableFeatureList().removeAt(i);
	QHash<QString, QStringList> savedActiveFormats;
	QHash<QString, int> savedFallbackSettings;
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups()) {
		if ( qmc2Config->contains(QString("%1/ActiveFormats").arg(name)) )
			savedActiveFormats.insert(name, qmc2Config->value(QString("%1/ActiveFormats").arg(name), QStringList()).toStringList());
		if ( qmc2Config->contains(QString("%1/Fallback").arg(name)) )
			savedFallbackSettings.insert(name, qmc2Config->value(QString("%1/Fallback").arg(name), 0).toInt());
	}
	qmc2Config->endGroup();
	qmc2Config->remove("Artwork");
	int systemArtworkIndex = 0, softwareArtworkIndex = 0;
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QLineEdit *lineEditName = (QLineEdit *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_NAME);
		QString name = lineEditName->text();
		if ( !name.isEmpty() ) {
			QToolButton *toolButtonIcon = (QToolButton *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_ICON);
			if ( !toolButtonIcon->whatsThis().isEmpty() )
				qmc2Config->setValue(QString("Artwork/%1/Icon").arg(name), toolButtonIcon->whatsThis());
			QComboBox *comboBoxTarget = (QComboBox *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_TARGET);
			qmc2Config->setValue(QString("Artwork/%1/Target").arg(name), comboBoxTarget->currentIndex());
			QComboBox *comboBoxScaled = (QComboBox *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_SCALED);
			qmc2Config->setValue(QString("Artwork/%1/Scaled").arg(name), comboBoxScaled->currentIndex());
			QComboBox *comboBoxType = (QComboBox *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_TYPE);
			qmc2Config->setValue(QString("Artwork/%1/Type").arg(name), comboBoxType->currentIndex());
			QComboBox *comboBoxFormat = (QComboBox *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_FORMAT);
			qmc2Config->setValue(QString("Artwork/%1/Format").arg(name), comboBoxFormat->currentIndex());
			QStackedWidget *stackedWidgetFolderOrArchive = (QStackedWidget *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_FOLDER_OR_ARCHIVE);
			DirectoryEditWidget *dewFolderOrArchive = (DirectoryEditWidget *)stackedWidgetFolderOrArchive->widget(QMC2_AW_INDEX_TYPE_FOLDER);
			if ( !dewFolderOrArchive->lineEditDirectory->text().isEmpty() )
				qmc2Config->setValue(QString("Artwork/%1/Folder").arg(name), dewFolderOrArchive->lineEditDirectory->text());
			FileEditWidget *fewFolderOrArchive = (FileEditWidget *)stackedWidgetFolderOrArchive->widget(QMC2_AW_INDEX_TYPE_ARCHIVE);
			if ( !fewFolderOrArchive->lineEditFile->text().isEmpty() )
				qmc2Config->setValue(QString("Artwork/%1/Archive").arg(name), fewFolderOrArchive->lineEditFile->text());
			if ( savedActiveFormats.contains(name) )
				qmc2Config->setValue(QString("Artwork/%1/ActiveFormats").arg(name), savedActiveFormats[name]);
			if ( savedFallbackSettings.contains(name) )
				qmc2Config->setValue(QString("Artwork/%1/Fallback").arg(name), savedFallbackSettings[name]);
			QString nameCopy = name;
			if ( comboBoxTarget->currentIndex() == QMC2_AW_INDEX_TARGET_SYSTEM ) {
				int featureIndex = QMC2_USEROFFSET_INDEX + systemArtworkIndex;
				systemArtworkIndex++;
				component2Info->setShortTitle(featureIndex, name);
				component2Info->setLongTitle(featureIndex, nameCopy.replace("&", QString()));
				component2Info->setIcon(featureIndex, QIcon(toolButtonIcon->whatsThis()));
				component2Info->availableFeatureList() << featureIndex;
				CustomArtwork *customArtwork = (CustomArtwork *)component2Info->widget(featureIndex);
				if ( customArtwork ) {
					customArtwork->setName(name);
					customArtwork->setNum(i);
					customArtwork->reopenSource();
					featureIndex = component2Info->appliedFeatureList().indexOf(featureIndex);
					if ( featureIndex >= 0 ) {
						qmc2MainWindow->tabWidgetMachineDetail->setTabText(featureIndex, name);
						qmc2MainWindow->tabWidgetMachineDetail->setTabIcon(featureIndex, QIcon(qmc2Config->value(QString("Artwork/%1/Icon").arg(name), QString()).toString()));
					}
				} else
					component2Info->setWidget(featureIndex, new CustomArtwork(0, name, i));
			} else {
				int featureIndex = QMC2_SWINFO_USEROFFSET_PAGE + softwareArtworkIndex;
				softwareArtworkIndex++;
				component4Info->setShortTitle(featureIndex, name);
				component4Info->setLongTitle(featureIndex, nameCopy.replace("&", QString()));
				component4Info->setIcon(featureIndex, QIcon(toolButtonIcon->whatsThis()));
				component4Info->availableFeatureList() << featureIndex;
				CustomSoftwareArtwork *customArtwork = (CustomSoftwareArtwork *)component4Info->widget(featureIndex);
				if ( customArtwork ) {
					customArtwork->setName(name);
					customArtwork->setNum(i);
					customArtwork->reopenSource();
					featureIndex = component4Info->appliedFeatureList().indexOf(featureIndex);
					if ( featureIndex >= 0 ) {
						qmc2MainWindow->tabWidgetSoftwareDetail->setTabText(featureIndex, name);
						qmc2MainWindow->tabWidgetSoftwareDetail->setTabIcon(featureIndex, QIcon(qmc2Config->value(QString("Artwork/%1/Icon").arg(name), QString()).toString()));
					}
				} else
					component4Info->setWidget(featureIndex, new CustomSoftwareArtwork(0, name, i));
			}
		}
	}
	qmc2ComponentSetup->saveComponent("Component2");
	qmc2ComponentSetup->saveComponent("Component4");
}

void AdditionalArtworkSetup::load()
{
	treeWidget->clear();
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	m_seq = 0;
	m_itemHash.clear();
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups()) {
		QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidget);
		m_itemHash[m_seq] = newItem;
		QCheckBox *checkBoxSelect = new QCheckBox(this);
		checkBoxSelect->setToolTip(tr("Select / deselect this artwork class for removal"));
		connect(checkBoxSelect, SIGNAL(toggled(bool)), this, SLOT(selectionFlagsChanged(bool)));
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_SELECT, checkBoxSelect);
		QLineEdit *lineEditName = new QLineEdit(this);
		lineEditName->setPlaceholderText(tr("Artwork name"));
		lineEditName->setToolTip(tr("Enter a name for this artwork class (required)"));
		lineEditName->setText(name);
		connect(lineEditName, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_NAME, lineEditName);
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
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_ICON, toolButtonIcon);
		QComboBox *comboBoxTarget = new QComboBox(this);
		comboBoxTarget->addItem(tr("System"));
		comboBoxTarget->addItem(tr("Software"));
		comboBoxTarget->setToolTip(tr("Select system or software as <i>target</i> for this artwork class"));
		int index = qmc2Config->value(QString("%1/Target").arg(name), 0).toInt();
		if ( index >= 0 && index < 2 )
			comboBoxTarget->setCurrentIndex(index);
		connect(comboBoxTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_TARGET, comboBoxTarget);
		QComboBox *comboBoxScaled = new QComboBox(this);
		comboBoxScaled->addItem(tr("On"));
		comboBoxScaled->addItem(tr("Off"));
		comboBoxScaled->setToolTip(tr("Choose if images of this artwork class are scaled or not"));
		index = qmc2Config->value(QString("%1/Scaled").arg(name), 0).toInt();
		if ( index >= 0 && index < 2 )
			comboBoxScaled->setCurrentIndex(index);
		connect(comboBoxScaled, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_SCALED, comboBoxScaled);
		QComboBox *comboBoxType = new QComboBox(this);
		comboBoxType->addItem(tr("Folder"));
		comboBoxType->addItem(tr("Archive"));
		comboBoxType->setToolTip(tr("Choose if images are loaded from a folder or an archive for this artwork class"));
		comboBoxType->setWhatsThis(QString::number(m_seq));
		index = qmc2Config->value(QString("%1/Type").arg(name), 0).toInt();
		if ( index >= 0 && index < 2 )
			comboBoxType->setCurrentIndex(index);
		connect(comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_TYPE, comboBoxType);
		QComboBox *comboBoxFormat = new QComboBox(this);
		comboBoxFormat->addItem(QIcon(":/data/img/compressed.png"), tr("ZIP"));
		comboBoxFormat->addItem(QIcon(":/data/img/7z-compressed.png"), tr("7z"));
#if defined(QMC2_LIBARCHIVE_ENABLED)
		comboBoxFormat->addItem(QIcon(":/data/img/archive.png"), tr("Archive"));
#endif
		comboBoxFormat->setToolTip(tr("Select archive format"));
		index = qmc2Config->value(QString("%1/Format").arg(name), 0).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
		if ( index >= 0 && index < 3 )
			comboBoxFormat->setCurrentIndex(index);
#else
		if ( index >= 0 && index < 2 )
			comboBoxFormat->setCurrentIndex(index);
#endif
		comboBoxFormat->setEnabled(comboBoxType->currentIndex() == QMC2_AW_INDEX_TYPE_ARCHIVE);
		connect(comboBoxFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_FORMAT, comboBoxFormat);
		FileEditWidget *fewFolderOrArchive = new FileEditWidget(QString(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files (*)"), QString(), this, false);
		fewFolderOrArchive->lineEditFile->setPlaceholderText(tr("Image archive"));
		fewFolderOrArchive->lineEditFile->setToolTip(tr("Image archive for this artwork class (required)"));
		fewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image archive"));
		fewFolderOrArchive->lineEditFile->setText(qmc2Config->value(QString("%1/Archive").arg(name), QString()).toString());
		fewFolderOrArchive->setWhatsThis(QString::number(m_seq));
		connect(fewFolderOrArchive, SIGNAL(pathBrowsed(QString)), this, SLOT(pathBrowsed(QString)));
		connect(fewFolderOrArchive->lineEditFile, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
		DirectoryEditWidget *dewFolderOrArchive = new DirectoryEditWidget(QString(), this);
		dewFolderOrArchive->lineEditDirectory->setPlaceholderText(tr("Image folder"));
		dewFolderOrArchive->lineEditDirectory->setToolTip(tr("Image folder for this artwork class (required)"));
		dewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image folder"));
		dewFolderOrArchive->lineEditDirectory->setText(qmc2Config->value(QString("%1/Folder").arg(name), QString()).toString());
		connect(dewFolderOrArchive->lineEditDirectory, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
		QStackedWidget *stackedWidgetFolderOrArchive = new QStackedWidget(this);
		stackedWidgetFolderOrArchive->insertWidget(QMC2_AW_INDEX_TYPE_FOLDER, dewFolderOrArchive);
		stackedWidgetFolderOrArchive->insertWidget(QMC2_AW_INDEX_TYPE_ARCHIVE, fewFolderOrArchive);
		index = qmc2Config->value(QString("%1/Type").arg(name), 0).toInt();
		if ( index >= 0 && index < 2 )
			stackedWidgetFolderOrArchive->setCurrentIndex(index);
		treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_FOLDER_OR_ARCHIVE, stackedWidgetFolderOrArchive);
		connect(comboBoxType, SIGNAL(currentIndexChanged(int)), stackedWidgetFolderOrArchive, SLOT(setCurrentIndex(int)));
		connect(comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleFormatEnabled(int)));
		m_seq++;
	}
	qmc2Config->endGroup();
	for (int i = 0; i < treeWidget->columnCount(); i++)
		treeWidget->resizeColumnToContents(i);
	pushButtonRestore->setEnabled(false);
}

void AdditionalArtworkSetup::on_toolButtonAdd_clicked()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QTreeWidgetItem *newItem = new QTreeWidgetItem(treeWidget);
	m_itemHash[m_seq] = newItem;
	QCheckBox *checkBoxSelect = new QCheckBox(this);
	checkBoxSelect->setToolTip(tr("Select / deselect this artwork class for removal"));
	connect(checkBoxSelect, SIGNAL(toggled(bool)), this, SLOT(selectionFlagsChanged(bool)));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_SELECT, checkBoxSelect);
	QLineEdit *lineEditName = new QLineEdit(this);
	lineEditName->setPlaceholderText(tr("Artwork name"));
	lineEditName->setToolTip(tr("Enter a name for this artwork class (required)"));
	connect(lineEditName, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_NAME, lineEditName);
	QToolButton *toolButtonIcon = new QToolButton(this);
	toolButtonIcon->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	toolButtonIcon->setText(tr("Choose..."));
	toolButtonIcon->setIconSize(iconSize);
	toolButtonIcon->setToolTip(tr("Choose an icon file to be used for this artwork class (optional)"));
	connect(toolButtonIcon, SIGNAL(clicked()), this, SLOT(chooseIcon()));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_ICON, toolButtonIcon);
	QComboBox *comboBoxTarget = new QComboBox(this);
	comboBoxTarget->addItem(tr("System"));
	comboBoxTarget->addItem(tr("Software"));
	comboBoxTarget->setToolTip(tr("Select system or software as <i>target</i> for this artwork class"));
	connect(comboBoxTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_TARGET, comboBoxTarget);
	QComboBox *comboBoxScaled = new QComboBox(this);
	comboBoxScaled->addItem(tr("On"));
	comboBoxScaled->addItem(tr("Off"));
	comboBoxScaled->setToolTip(tr("Choose if images of this artwork class are scaled or not"));
	connect(comboBoxScaled, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_SCALED, comboBoxScaled);
	QComboBox *comboBoxType = new QComboBox(this);
	comboBoxType->addItem(tr("Folder"));
	comboBoxType->addItem(tr("Archive"));
	comboBoxType->setToolTip(tr("Choose if images are loaded from a folder or an archive for this artwork class"));
	comboBoxType->setWhatsThis(QString::number(m_seq));
	connect(comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_TYPE, comboBoxType);
	QComboBox *comboBoxFormat = new QComboBox(this);
	comboBoxFormat->addItem(QIcon(":/data/img/compressed.png"), tr("ZIP"));
	comboBoxFormat->addItem(QIcon(":/data/img/7z-compressed.png"), tr("7z"));
#if defined(QMC2_LIBARCHIVE_ENABLED)
	comboBoxFormat->addItem(QIcon(":/data/img/archive.png"), tr("Archive"));
#endif
	comboBoxFormat->setToolTip(tr("Select archive format"));
	comboBoxFormat->setEnabled(false);
	connect(comboBoxFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(dataChanged(int)));
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_FORMAT, comboBoxFormat);
	FileEditWidget *fewFolderOrArchive = new FileEditWidget(QString(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files (*)"), QString(), this, false);
	fewFolderOrArchive->lineEditFile->setPlaceholderText(tr("Image archive"));
	fewFolderOrArchive->lineEditFile->setToolTip(tr("Image archive for this artwork class (required)"));
	fewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image archive"));
	fewFolderOrArchive->setWhatsThis(QString::number(m_seq));
	connect(fewFolderOrArchive, SIGNAL(pathBrowsed(QString)), this, SLOT(pathBrowsed(QString)));
	connect(fewFolderOrArchive->lineEditFile, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
	DirectoryEditWidget *dewFolderOrArchive = new DirectoryEditWidget(QString(), this);
	dewFolderOrArchive->lineEditDirectory->setPlaceholderText(tr("Image folder"));
	dewFolderOrArchive->lineEditDirectory->setToolTip(tr("Image folder for this artwork class (required)"));
	dewFolderOrArchive->toolButtonBrowse->setToolTip(tr("Browse image folder"));
	connect(dewFolderOrArchive->lineEditDirectory, SIGNAL(textChanged(const QString &)), this, SLOT(dataChanged(const QString &)));
	QStackedWidget *stackedWidgetFolderOrArchive = new QStackedWidget(this);
	stackedWidgetFolderOrArchive->insertWidget(QMC2_AW_INDEX_TYPE_FOLDER, dewFolderOrArchive);
	stackedWidgetFolderOrArchive->insertWidget(QMC2_AW_INDEX_TYPE_ARCHIVE, fewFolderOrArchive);
	treeWidget->setItemWidget(newItem, QMC2_AW_COLUMN_FOLDER_OR_ARCHIVE, stackedWidgetFolderOrArchive);
	connect(comboBoxType, SIGNAL(currentIndexChanged(int)), stackedWidgetFolderOrArchive, SLOT(setCurrentIndex(int)));
	connect(comboBoxType, SIGNAL(currentIndexChanged(int)), this, SLOT(toggleFormatEnabled(int)));
	m_seq++;
	for (int i = 0; i < treeWidget->columnCount(); i++)
		treeWidget->resizeColumnToContents(i);
	dataChanged();
}

void AdditionalArtworkSetup::toggleFormatEnabled(int index)
{
	QComboBox *cb = (QComboBox *)sender();
	if ( !cb )
		return;
	QTreeWidgetItem *item = m_itemHash[cb->whatsThis().toInt()];
	if ( !item )
		return;
	treeWidget->itemWidget(item, QMC2_AW_COLUMN_FORMAT)->setEnabled(cb->currentIndex() == QMC2_AW_INDEX_TYPE_ARCHIVE);
}

void AdditionalArtworkSetup::pathBrowsed(QString path)
{
	FileEditWidget *few = (FileEditWidget *)sender();
	if ( !few )
		return;
	QTreeWidgetItem *item = m_itemHash[few->whatsThis().toInt()];
	if ( !item )
		return;
	QComboBox *cb = (QComboBox *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_FORMAT);
	if ( !cb )
		return;
	if ( path.toLower().endsWith(".zip") )
		cb->setCurrentIndex(QMC2_AW_INDEX_FORMAT_ZIP);
	else if ( path.toLower().endsWith(".7z") )
		cb->setCurrentIndex(QMC2_AW_INDEX_FORMAT_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
	else
		cb->setCurrentIndex(QMC2_AW_INDEX_FORMAT_ARCHIVE);
#endif
}

void AdditionalArtworkSetup::on_toolButtonRemove_clicked()
{
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QCheckBox *cb = (QCheckBox *)treeWidget->itemWidget(item, QMC2_AW_COLUMN_SELECT);
		if ( cb && cb->isChecked() ) {
			item = treeWidget->takeTopLevelItem(i);
			delete item;
			i--;
		}
	}
	selectionFlagsChanged();
	dataChanged();
}

void AdditionalArtworkSetup::selectionFlagsChanged(bool)
{
	bool enable = false;
	for (int i = 0; i < treeWidget->topLevelItemCount() && !enable; i++) {
		QCheckBox *cb = (QCheckBox *)treeWidget->itemWidget(treeWidget->topLevelItem(i), QMC2_AW_COLUMN_SELECT);
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
		dataChanged();
	}
}

void AdditionalArtworkSetup::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	for (int i = 0; i < treeWidget->columnCount(); i++)
		treeWidget->resizeColumnToContents(i);
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
