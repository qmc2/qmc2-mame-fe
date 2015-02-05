#include <QApplication>

#include "settings.h"
#include "paletteeditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern QPalette qmc2CustomPalette;

QStringList PaletteEditor::colorNames;

PaletteEditor::PaletteEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	setWindowModality(Qt::ApplicationModal);

	customPalette = activePalette = qApp->palette();

	ColorWidget *cw;
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QString colorName = item->text(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
		QPalette::ColorRole colorRole = colorNameToRole(colorName);
		cw = new ColorWidget(tr("Active"), colorName, QPalette::Active, colorRole, customPalette.color(QPalette::Active, colorRole), customPalette.brush(QPalette::Active, colorRole), this);
		cw->frameBrush->setToolTip(item->toolTip(QMC2_PALETTEEDITOR_COLUMN_COLORROLE) + " / " + tr("Active"));
		connect(cw, SIGNAL(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor)), this, SLOT(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor))),
		connect(cw, SIGNAL(brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush)), this, SLOT(brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush))),
		treeWidget->setItemWidget(item, QMC2_PALETTEEDITOR_COLUMN_ACTIVE, cw);
		cw = new ColorWidget(tr("Inactive"), colorName, QPalette::Inactive, colorRole, customPalette.color(QPalette::Inactive, colorRole), customPalette.brush(QPalette::Inactive, colorRole), this);
		cw->frameBrush->setToolTip(item->toolTip(QMC2_PALETTEEDITOR_COLUMN_COLORROLE) + " / " + tr("Inactive"));
		connect(cw, SIGNAL(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor)), this, SLOT(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor))),
		connect(cw, SIGNAL(brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush)), this, SLOT(brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush))),
		treeWidget->setItemWidget(item, QMC2_PALETTEEDITOR_COLUMN_INACTIVE, cw);
		cw = new ColorWidget(tr("Disabled"), colorName, QPalette::Disabled, colorRole, customPalette.color(QPalette::Disabled, colorRole), customPalette.brush(QPalette::Disabled, colorRole), this);
		cw->frameBrush->setToolTip(item->toolTip(QMC2_PALETTEEDITOR_COLUMN_COLORROLE) + " / " + tr("Disabled"));
		connect(cw, SIGNAL(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor)), this, SLOT(colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor))),
		connect(cw, SIGNAL(brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush)), this, SLOT(brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush))),
		treeWidget->setItemWidget(item, QMC2_PALETTEEDITOR_COLUMN_DISABLED, cw);
	}

	checkBoxCalculateDetails->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "PaletteEditor/CalculateDetails", true).toBool());
	toolButtonPreview->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "PaletteEditor/PreviewPalette", false).toBool());

	QPalette pal;
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QString colorName = item->text(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
		QPalette::ColorRole colorRole = colorNameToRole(colorName);
		cw = (ColorWidget *)treeWidget->itemWidget(item, QMC2_PALETTEEDITOR_COLUMN_ACTIVE);
		if ( cw ) {
			customPalette.setColor(QPalette::Inactive, colorRole, cw->activeColor);
			//customPalette.setBrush(QPalette::Inactive, colorRole, cw->activeBrush);
			customPalette.setColor(QPalette::Disabled, colorRole, cw->activeColor.darker(115));
			//customPalette.setBrush(QPalette::Disabled, colorRole, cw->activeBrush);
		}
		cw = (ColorWidget *)treeWidget->itemWidget(item, QMC2_PALETTEEDITOR_COLUMN_INACTIVE);
		if ( cw ) {
			cw->activeColor = customPalette.color(QPalette::Inactive, colorRole);
			cw->activeBrush = customPalette.brush(QPalette::Inactive, colorRole);
			pal = cw->frameBrush->palette();
			pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
			//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
			cw->frameBrush->setPalette(pal);
			cw->frameBrush->update();
		}
		cw = (ColorWidget *)treeWidget->itemWidget(item, QMC2_PALETTEEDITOR_COLUMN_DISABLED);
		if ( cw ) {
			cw->activeColor = customPalette.color(QPalette::Disabled, colorRole);
			cw->activeBrush = customPalette.brush(QPalette::Disabled, colorRole);
			pal = cw->frameBrush->palette();
			pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
			//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
			cw->frameBrush->setPalette(pal);
			cw->frameBrush->update();
		}
	}

	activePalette = customPalette;
}

PaletteEditor::~PaletteEditor()
{
}

QPalette::ColorRole PaletteEditor::colorNameToRole(QString colorName)
{
	switch ( colorNames.indexOf(colorName) ) {
		case QMC2_PALETTEEDITOR_COLIDX_WINDOW:
			return QPalette::Window;
		case QMC2_PALETTEEDITOR_COLIDX_WINDOWTEXT:
			return QPalette::WindowText;
		case QMC2_PALETTEEDITOR_COLIDX_BASE:
			return QPalette::Base;
		case QMC2_PALETTEEDITOR_COLIDX_ALTERNATEBASE:
			return QPalette::AlternateBase;
		case QMC2_PALETTEEDITOR_COLIDX_TEXT:
			return QPalette::Text;
		case QMC2_PALETTEEDITOR_COLIDX_BRIGHTTEXT:
			return QPalette::BrightText;
		case QMC2_PALETTEEDITOR_COLIDX_BUTTON:
			return QPalette::Button;
		case QMC2_PALETTEEDITOR_COLIDX_BUTTONTEXT:
			return QPalette::ButtonText;
		case QMC2_PALETTEEDITOR_COLIDX_TOOLTIPBASE:
			return QPalette::ToolTipBase;
		case QMC2_PALETTEEDITOR_COLIDX_TOOLTIPTEXT:
			return QPalette::ToolTipText;
		case QMC2_PALETTEEDITOR_COLIDX_LIGHT:
			return QPalette::Light;
		case QMC2_PALETTEEDITOR_COLIDX_MIDLIGHT:
			return QPalette::Midlight;
		case QMC2_PALETTEEDITOR_COLIDX_DARK:
			return QPalette::Dark;
		case QMC2_PALETTEEDITOR_COLIDX_MID:
			return QPalette::Mid;
		case QMC2_PALETTEEDITOR_COLIDX_SHADOW:
			return QPalette::Shadow;
		case QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHT:
			return QPalette::Highlight;
		case QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHTEDTEXT:
			return QPalette::HighlightedText;
		case QMC2_PALETTEEDITOR_COLIDX_LINK:
			return QPalette::Link;
		case QMC2_PALETTEEDITOR_COLIDX_LINKVISITED:
			return QPalette::LinkVisited;
		default:
			return QPalette::NoRole;
	}
}

QString PaletteEditor::colorRoleToName(QPalette::ColorRole colorRole)
{
	switch ( colorRole ) {
		case QPalette::Window:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_WINDOW];
		case QPalette::WindowText:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_WINDOWTEXT];
		case QPalette::Base:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_BASE];
		case QPalette::AlternateBase:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_ALTERNATEBASE];
		case QPalette::Text:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_TEXT];
		case QPalette::BrightText:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_BRIGHTTEXT];
		case QPalette::Button:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_BUTTON];
		case QPalette::ButtonText:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_BUTTONTEXT];
		case QPalette::ToolTipBase:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_TOOLTIPBASE];
		case QPalette::ToolTipText:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_TOOLTIPTEXT];
		case QPalette::Light:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_LIGHT];
		case QPalette::Midlight:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_MIDLIGHT];
		case QPalette::Dark:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_DARK];
		case QPalette::Mid:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_MID];
		case QPalette::Shadow:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_SHADOW];
		case QPalette::Highlight:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHT];
		case QPalette::HighlightedText:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHTEDTEXT];
		case QPalette::Link:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_LINK];
		case QPalette::LinkVisited:
			return colorNames[QMC2_PALETTEEDITOR_COLIDX_LINKVISITED];
		default:
			return QString();
	}
}

void PaletteEditor::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonRestore->setIconSize(iconSize);
	toolButtonPreview->setIconSize(iconSize);
}

void PaletteEditor::colorChanged(QPalette::ColorGroup cg, QPalette::ColorRole cr, QColor c)
{
	customPalette.setColor(cg, cr, c);

	if ( checkBoxCalculateDetails->isChecked() && cg == QPalette::Active ) {
		customPalette.setColor(QPalette::Inactive, cr, c);
		customPalette.setColor(QPalette::Disabled, cr, c.darker(115));
		QList<QTreeWidgetItem *> il = treeWidget->findItems(colorRoleToName(cr), Qt::MatchExactly, QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
		if ( !il.isEmpty() ) {
			ColorWidget *cw;
			cw = (ColorWidget *)treeWidget->itemWidget(il[0], QMC2_PALETTEEDITOR_COLUMN_INACTIVE);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Inactive, cr);
				QPalette pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
			cw = (ColorWidget *)treeWidget->itemWidget(il[0], QMC2_PALETTEEDITOR_COLUMN_DISABLED);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Disabled, cr);
				QPalette pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
		}
	}

	if ( toolButtonPreview->isChecked() ) {
		qApp->setPalette(customPalette);
		QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	}

	pushButtonRestore->setEnabled(true);
}

void PaletteEditor::brushChanged(QPalette::ColorGroup /*cg*/, QPalette::ColorRole /*cr*/, QBrush /*b*/)
{
	// FIXME
}

void PaletteEditor::on_pushButtonOk_clicked()
{
	activePalette = customPalette;
	qApp->setPalette(activePalette);
	accept();
}

void PaletteEditor::on_pushButtonCancel_clicked()
{
	reject();
}

void PaletteEditor::on_toolButtonPreview_toggled(bool checked)
{
	if ( checked )
		qApp->setPalette(customPalette);
	else
		qApp->setPalette(activePalette);

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "PaletteEditor/PreviewPalette", checked);
}

void PaletteEditor::on_pushButtonRestore_clicked()
{
	customPalette = activePalette;
	ColorWidget *cw;
	QPalette pal;
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QString colorName = item->text(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
		QPalette::ColorRole colorRole = colorNameToRole(colorName);
		QList<QTreeWidgetItem *> il = treeWidget->findItems(colorName, Qt::MatchExactly, QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
		if ( !il.isEmpty() ) {
			cw = (ColorWidget *)treeWidget->itemWidget(il[0], QMC2_PALETTEEDITOR_COLUMN_ACTIVE);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Active, colorRole);
				cw->activeBrush = customPalette.brush(QPalette::Active, colorRole);
				pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
			cw = (ColorWidget *)treeWidget->itemWidget(il[0], QMC2_PALETTEEDITOR_COLUMN_INACTIVE);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Inactive, colorRole);
				cw->activeBrush = customPalette.brush(QPalette::Inactive, colorRole);
				pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
			cw = (ColorWidget *)treeWidget->itemWidget(il[0], QMC2_PALETTEEDITOR_COLUMN_DISABLED);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Disabled, colorRole);
				cw->activeBrush = customPalette.brush(QPalette::Disabled, colorRole);
				pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
		}
	}

	if ( toolButtonPreview->isChecked() ) {
		qApp->setPalette(customPalette);
		QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	}

	pushButtonRestore->setEnabled(false);
}

void PaletteEditor::on_checkBoxCalculateDetails_toggled(bool checked)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "PaletteEditor/CalculateDetails", checked);
	if ( checked ) {
		ColorWidget *cw;
		QPalette pal;
		for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidget->topLevelItem(i);
			QString colorName = item->text(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
			QPalette::ColorRole colorRole = colorNameToRole(colorName);
			cw = (ColorWidget *)treeWidget->itemWidget(item, QMC2_PALETTEEDITOR_COLUMN_ACTIVE);
			if ( cw ) {
				customPalette.setColor(QPalette::Inactive, colorRole, cw->activeColor);
				//customPalette.setBrush(QPalette::Inactive, colorRole, cw->activeBrush);
				customPalette.setColor(QPalette::Disabled, colorRole, cw->activeColor.darker(115));
				//customPalette.setBrush(QPalette::Disabled, colorRole, cw->activeBrush);
			}
			cw = (ColorWidget *)treeWidget->itemWidget(item, QMC2_PALETTEEDITOR_COLUMN_INACTIVE);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Inactive, colorRole);
				cw->activeBrush = customPalette.brush(QPalette::Inactive, colorRole);
				pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
			cw = (ColorWidget *)treeWidget->itemWidget(item, QMC2_PALETTEEDITOR_COLUMN_DISABLED);
			if ( cw ) {
				cw->activeColor = customPalette.color(QPalette::Disabled, colorRole);
				cw->activeBrush = customPalette.brush(QPalette::Disabled, colorRole);
				pal = cw->frameBrush->palette();
				pal.setColor(cw->frameBrush->backgroundRole(), cw->activeColor);
				//pal.setBrush(cw->frameBrush->backgroundRole(), cw->activeBrush);
				cw->frameBrush->setPalette(pal);
				cw->frameBrush->update();
			}
		}
	}

	resizeEvent(NULL);

	if ( toolButtonPreview->isChecked() ) {
		qApp->setPalette(customPalette);
		QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	}
}

void PaletteEditor::showEvent(QShowEvent *e)
{
	pushButtonRestore->setEnabled(activePalette != customPalette);
	adjustIconSizes();
	adjustSize();
	treeWidget->resizeColumnToContents(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
	int w = treeWidget->viewport()->width() - treeWidget->columnWidth(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
	if ( checkBoxCalculateDetails->isChecked() ) {
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, true);
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_DISABLED, true);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_ACTIVE, w);
	} else {
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, false);
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_DISABLED, false);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_ACTIVE, w/3);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, w/3);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_DISABLED, w/3);
	}
	adjustSize();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/PaletteEditor/Geometry", QByteArray()).toByteArray());

	if ( toolButtonPreview->isChecked() ) {
		qApp->setPalette(customPalette);
		QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	} else
		qApp->setPalette(activePalette);

	if ( e )
		QDialog::showEvent(e);
}

void PaletteEditor::resizeEvent(QResizeEvent *e)
{
	treeWidget->resizeColumnToContents(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
	int w = treeWidget->viewport()->width() - treeWidget->columnWidth(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
	if ( checkBoxCalculateDetails->isChecked() ) {
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, true);
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_DISABLED, true);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_ACTIVE, w);
	} else {
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, false);
		treeWidget->setColumnHidden(QMC2_PALETTEEDITOR_COLUMN_DISABLED, false);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_ACTIVE, w/3);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, w/3);
		treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_DISABLED, w/3);
	}

	if ( e )
		QDialog::resizeEvent(e);
}

void PaletteEditor::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/PaletteEditor/Geometry", saveGeometry());
	on_pushButtonCancel_clicked();

	if ( e )
		QDialog::hideEvent(e);
}
