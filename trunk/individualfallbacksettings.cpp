#include <QTreeWidgetItem>
#include <QString>
#include <QSize>
#include <QIcon>
#include <QFont>
#include <QFontMetrics>
#include <QComboBox>

#include "individualfallbacksettings.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

IndividualFallbackSettings::IndividualFallbackSettings(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	treeWidget->setFocusPolicy(Qt::NoFocus);
	artworkClassNames << tr("Preview")
			  << tr("Flyer")
			  << tr("Cabinet")
			  << tr("Controller")
			  << tr("Marquee")
			  << tr("Title")
			  << tr("PCB")
			  << tr("Software snap")
			  << tr("Video snap")
			  << tr("Icon");
	artworkClassDisplayNames = artworkClassNames;
	artworkClassIcons << ":/data/img/camera.png"
			  << ":/data/img/thumbnail.png"
			  << ":/data/img/arcadecabinet.png"
			  << ":/data/img/joystick.png"
			  << ":/data/img/marquee.png"
			  << ":/data/img/arcademode.png"
			  << ":/data/img/circuit.png"
			  << ":/data/img/pacman.png"
			  << ":/data/img/ghost_video.png"
			  << ":/data/img/icon.png";
	parentFallbackKeys << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/VideoFallback"
			   << QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFallback";
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups()) {
		parentFallbackKeys << QString("Artwork/%1/Fallback").arg(name);
		artworkClassIcons << qmc2Config->value(QString("%1/Icon").arg(name), QString()).toString();
		artworkClassNames << name;
		artworkClassDisplayNames << name.replace("&", QString());
	}
	qmc2Config->endGroup();
}

void IndividualFallbackSettings::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	treeWidget->setIconSize(iconSize);
}

void IndividualFallbackSettings::load()
{
	treeWidget->clear();
	for (int i = 0; i < artworkClassNames.count(); i++) {
		QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
		item->setText(QMC2_IFS_COLUMN_ARTWORK, artworkClassDisplayNames[i]);
		item->setWhatsThis(QMC2_IFS_COLUMN_ARTWORK, artworkClassNames[i]);
		item->setIcon(QMC2_IFS_COLUMN_ARTWORK, QIcon(artworkClassIcons[i]));
		QComboBox *comboBox = new QComboBox(this);
		comboBox->addItem(tr("active"));
		comboBox->addItem(tr("inactive"));
		comboBox->setToolTip(tr("Activate / deactivate parent fallback for this artwork class"));
		int index = qmc2Config->value(parentFallbackKeys[i], 0).toInt();
		if ( index >= 0 && index < 2 )
			comboBox->setCurrentIndex(index);
		treeWidget->setItemWidget(item, QMC2_IFS_COLUMN_FALLBACK, comboBox);
	}
	for (int i = 0; i < treeWidget->columnCount(); i++)
		treeWidget->resizeColumnToContents(i);
}

void IndividualFallbackSettings::save()
{
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QString artworkName = item->whatsThis(QMC2_IFS_COLUMN_ARTWORK);
		int index = artworkClassNames.indexOf(artworkName);
		if ( index >= 0 ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(item, QMC2_IFS_COLUMN_FALLBACK);
			if ( comboBox ) {
				if ( comboBox->currentIndex() == 0 )
					qmc2Config->remove(parentFallbackKeys[index]);
				else
					qmc2Config->setValue(parentFallbackKeys[index], comboBox->currentIndex());
			}
		}
	}
}

void IndividualFallbackSettings::on_pushButtonOk_clicked()
{
	save();
	accept();
}

void IndividualFallbackSettings::on_pushButtonCancel_clicked()
{
	reject();
}

void IndividualFallbackSettings::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/FallbackSettings/Geometry", QByteArray()).toByteArray());
	load();
	QDialog::showEvent(e);
}

void IndividualFallbackSettings::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/FallbackSettings/Geometry", saveGeometry());
	QDialog::hideEvent(e);
}
