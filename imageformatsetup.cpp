#include <QIcon>

#include "settings.h"
#include "imageformatsetup.h"
#include "imagewidget.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"
#include "softwarelist.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern SoftwareSnap *qmc2SoftwareSnap;
extern SoftwareSnapshot *qmc2SoftwareSnapshot;

QStringList ImageFormatSetup::artworkClassPrefixes;
QStringList ImageFormatSetup::artworkClassNames;
QStringList ImageFormatSetup::artworkClassIcons;

ImageFormatSetup::ImageFormatSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	mPreviousClassIndex = -1;

#if QT_VERSION < 0x050000
	treeWidget->header()->setMovable(false);
#else
	treeWidget->header()->setSectionsMovable(false);
#endif

	if ( artworkClassPrefixes.isEmpty() ) {
		artworkClassPrefixes << "prv" << "fly" << "cab" << "ctl" << "mrq" << "ttl" << "pcb" << "sws";
		artworkClassNames << tr("Previews") << tr("Flyers") << tr("Cabinets") << tr("Controllers") << tr("Marquees") << tr("Titles") << tr("PCBs") << tr("Software snaps");
		artworkClassIcons << ":/data/img/camera.png" << ":/data/img/thumbnail.png" << ":/data/img/arcadecabinet.png" << ":/data/img/joystick.png" << ":/data/img/marquee.png" << ":/data/img/arcademode.png" << ":/data/img/circuit.png" << ":/data/img/pacman.png";
	}

	restoreActiveFormats(true);
	connect(treeWidget->model(), SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(rowsInserted(const QModelIndex &, int, int)));
}

void ImageFormatSetup::restoreActiveFormats(bool init)
{
	foreach (QString fmt, mActiveFormats.keys())
		mActiveFormats[fmt].clear();
	mActiveFormats.clear();

	// FIXME: add support for additional artwork classes

	for (int i = 0; i < artworkClassPrefixes.count(); i++) {
		QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("ActiveImageFormats/%1").arg(artworkClassPrefixes[i]), QStringList()).toStringList();
		if ( imgFmts.isEmpty() )
			mActiveFormats[artworkClassPrefixes[i]] << QMC2_IMAGE_FORMAT_INDEX_PNG;
		else for (int j = 0; j < imgFmts.count(); j++)
			mActiveFormats[artworkClassPrefixes[i]] << imgFmts[j].toInt();
		if ( init )
			comboBoxImageType->addItem(QIcon(artworkClassIcons[i]), artworkClassNames[i], artworkClassPrefixes[i]);
	}
}

void ImageFormatSetup::checkForModifications()
{
	bool modified = false;
	for (int i = 0; i < artworkClassPrefixes.count() && !modified; i++) {
		QList<int> storedFormats;
		QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("ActiveImageFormats/%1").arg(artworkClassPrefixes[i]), QStringList()).toStringList();
		if ( imgFmts.isEmpty() )
			storedFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
		else for (int j = 0; j < imgFmts.count(); j++)
			storedFormats << imgFmts[j].toInt();
		modified = (mActiveFormats[artworkClassPrefixes[i]] != storedFormats);
	}
	pushButtonRestore->setEnabled(modified);
}

void ImageFormatSetup::rowsInserted(const QModelIndex &, int, int)
{
	QString artworkClass = comboBoxImageType->itemData(comboBoxImageType->currentIndex()).toString();
	mActiveFormats[artworkClass].clear();
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *tlItem = treeWidget->topLevelItem(i);
		if ( tlItem->checkState(QMC2_IMGFMT_SETUP_COLUMN_ACT) == Qt::Checked )
			mActiveFormats[artworkClass] << ImageWidget::formatNames.indexOf(tlItem->text(QMC2_IMGFMT_SETUP_COLUMN_NAME));
	}
	QTimer::singleShot(0, this, SLOT(checkForModifications()));
}

void ImageFormatSetup::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonRestore->setIconSize(iconSize);
	comboBoxImageType->setIconSize(iconSize);
}

void ImageFormatSetup::on_pushButtonOk_clicked()
{
	QString artworkClass = comboBoxImageType->itemData(comboBoxImageType->currentIndex()).toString();
	mActiveFormats[artworkClass].clear();
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *tlItem = treeWidget->topLevelItem(i);
		if ( tlItem->checkState(QMC2_IMGFMT_SETUP_COLUMN_ACT) == Qt::Checked )
			mActiveFormats[artworkClass] << ImageWidget::formatNames.indexOf(tlItem->text(QMC2_IMGFMT_SETUP_COLUMN_NAME));
	}

	QMapIterator<QString, QList<int> > it(mActiveFormats);
	while ( it.hasNext() ) {
		it.next();
		QStringList prioList;
		foreach (int format, it.value())
			prioList << QString::number(format);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("ActiveImageFormats/%1").arg(it.key()), prioList);
	}

	ImageWidget::reloadArtworkFormats();
	// FIXME: begin: integrate this in ImageWidget::reloadArtworkFormats()
	if ( qmc2SoftwareSnap )
		qmc2SoftwareSnap->reloadActiveFormats();
	if ( qmc2SoftwareSnapshot )
		qmc2SoftwareSnapshot->reloadActiveFormats();
	// FIXME: end

	accept();
}

void ImageFormatSetup::on_pushButtonCancel_clicked()
{
	reject();
}

void ImageFormatSetup::on_pushButtonRestore_clicked()
{
	restoreActiveFormats();
	mPreviousClassIndex = -1;
	on_comboBoxImageType_currentIndexChanged(comboBoxImageType->currentIndex());
	QTimer::singleShot(0, this, SLOT(checkForModifications()));
}

void ImageFormatSetup::on_comboBoxImageType_currentIndexChanged(int index)
{
	if ( mPreviousClassIndex > -1 ) {
		QString previousArtworkClass = comboBoxImageType->itemData(mPreviousClassIndex).toString();
		mActiveFormats[previousArtworkClass].clear();
		for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
			QTreeWidgetItem *tlItem = treeWidget->topLevelItem(i);
			if ( tlItem->checkState(QMC2_IMGFMT_SETUP_COLUMN_ACT) == Qt::Checked )
				mActiveFormats[previousArtworkClass] << ImageWidget::formatNames.indexOf(tlItem->text(QMC2_IMGFMT_SETUP_COLUMN_NAME));
		}
	}

	treeWidget->clear();

	QList<QTreeWidgetItem *> insertItems;
	QList<int> activeFormats = mActiveFormats[artworkClassPrefixes[index]];
	for (int i = 0; i < QMC2_IMAGE_FORMAT_COUNT; i++) {
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		item->setText(QMC2_IMGFMT_SETUP_COLUMN_NAME, ImageWidget::formatNames[i]);
		item->setText(QMC2_IMGFMT_SETUP_COLUMN_DESC, ImageWidget::formatDescriptions[i]);
		item->setText(QMC2_IMGFMT_SETUP_COLUMN_EXT, ImageWidget::formatExtensions[i]);
		if ( activeFormats.indexOf(i) < 0 ) {
			item->setText(QMC2_IMGFMT_SETUP_COLUMN_ACT, "(" + tr("deactivated") + ")");
			item->setCheckState(QMC2_IMGFMT_SETUP_COLUMN_ACT, Qt::Unchecked);
		} else {
			item->setText(QMC2_IMGFMT_SETUP_COLUMN_ACT, "(" + tr("activated") + ")");
			item->setCheckState(QMC2_IMGFMT_SETUP_COLUMN_ACT, Qt::Checked);
		}
		insertItems << item;
	}

	int prio = 0;
	for (int i = 0; i < activeFormats.count(); i++) {
		bool found = false;
		int format = activeFormats[i];
		for (int j = 0; j < insertItems.count() && !found; j++) {
			if ( ImageWidget::formatNames.indexOf(insertItems[j]->text(QMC2_IMGFMT_SETUP_COLUMN_NAME)) == format ) {
				insertItems.insert(prio++, insertItems.takeAt(j));
				found = true;
			}
		}
	}

	treeWidget->insertTopLevelItems(0, insertItems);

	for (int i = 0; i < treeWidget->columnCount(); i++)
		treeWidget->resizeColumnToContents(i);

	mPreviousClassIndex = index;
}

void ImageFormatSetup::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_IMGFMT_SETUP_COLUMN_ACT ) {
		if ( item->text(QMC2_IMGFMT_SETUP_COLUMN_ACT) == "(" + tr("activated") + ")" ) {
			item->setText(QMC2_IMGFMT_SETUP_COLUMN_ACT, "(" + tr("deactivated") + ")");
			item->setCheckState(QMC2_IMGFMT_SETUP_COLUMN_ACT, Qt::Unchecked);
		} else {
			item->setText(QMC2_IMGFMT_SETUP_COLUMN_ACT, "(" + tr("activated") + ")");
			item->setCheckState(QMC2_IMGFMT_SETUP_COLUMN_ACT, Qt::Checked);
		}
		QString artworkClass = comboBoxImageType->itemData(comboBoxImageType->currentIndex()).toString();
		mActiveFormats[artworkClass].clear();
		for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
			QTreeWidgetItem *tlItem = treeWidget->topLevelItem(i);
			if ( tlItem->checkState(QMC2_IMGFMT_SETUP_COLUMN_ACT) == Qt::Checked )
				mActiveFormats[artworkClass] << ImageWidget::formatNames.indexOf(tlItem->text(QMC2_IMGFMT_SETUP_COLUMN_NAME));
		}
		QTimer::singleShot(0, this, SLOT(checkForModifications()));
	}
}

void ImageFormatSetup::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/Geometry", QByteArray()).toByteArray());
	comboBoxImageType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/ImageType", QMC2_IMAGE_FORMAT_INDEX_PNG).toInt());

	if ( e )
		QDialog::showEvent(e);
}

void ImageFormatSetup::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageFormatSetup/ImageType", comboBoxImageType->currentIndex());

	if ( e )
		QDialog::hideEvent(e);
}
