#include <QApplication>
#include <QSettings>

#include "paletteeditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

PaletteEditor::PaletteEditor(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	colorNames << "Window" << "WindowText" << "Base" << "AlternateBase" << "Text" << "BrightText" << "Button"
		   << "ButtonText" << "ToolTipBase" << "ToolTipText" << "Light" << "Midlight" << "Dark" << "Mid"
		   << "Shadow" << "Highlight" << "HighlightedText" << "Link" << "LinkVisited";

	customPalette = qApp->palette();

	ColorWidget *cw;
	for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget->topLevelItem(i);
		QString colorName = item->text(QMC2_PALETTEEDITOR_COLUMN_COLORROLE);
		QPalette::ColorRole colorRole = colorNameToRole(colorName);
		cw = new ColorWidget(QPalette::Active, colorRole, customPalette.color(QPalette::Active, colorRole), customPalette.brush(QPalette::Active, colorRole), this);
		treeWidget->setItemWidget(item, QMC2_PALETTEEDITOR_COLUMN_ACTIVE, cw);
		activeColorWidgets[colorName] = cw;
		cw = new ColorWidget(QPalette::Inactive, colorRole, customPalette.color(QPalette::Inactive, colorRole), customPalette.brush(QPalette::Inactive, colorRole), this);
		treeWidget->setItemWidget(item, QMC2_PALETTEEDITOR_COLUMN_INACTIVE, cw);
		inactiveColorWidgets[colorName] = cw;
		cw = new ColorWidget(QPalette::Disabled, colorRole, customPalette.color(QPalette::Disabled, colorRole), customPalette.brush(QPalette::Disabled, colorRole), this);
		treeWidget->setItemWidget(item, QMC2_PALETTEEDITOR_COLUMN_DISABLED, cw);
		disabledColorWidgets[colorName] = cw;
	}
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

void PaletteEditor::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	QSize iconSizeLarge = iconSize + QSize(4, 4);

	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonPreview->setIconSize(iconSize);
}

void PaletteEditor::on_pushButtonOk_clicked()
{
	accept();
}

void PaletteEditor::on_pushButtonCancel_clicked()
{
	reject();
}

void PaletteEditor::on_pushButtonPreview_clicked()
{
}

void PaletteEditor::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	int w = treeWidget->viewport()->width();
	treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_COLORROLE, w/4);
	treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_ACTIVE, w/4);
	treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, w/4);
	treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_DISABLED, w/4);
	adjustSize();
	QDialog::showEvent(e);
}

void PaletteEditor::resizeEvent(QResizeEvent *e)
{
        int w = treeWidget->viewport()->width();
        treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_COLORROLE, w/4);
        treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_ACTIVE, w/4);
        treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_INACTIVE, w/4);
        treeWidget->setColumnWidth(QMC2_PALETTEEDITOR_COLUMN_DISABLED, w/4);
	QDialog::resizeEvent(e);
}
