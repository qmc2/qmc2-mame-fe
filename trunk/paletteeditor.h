#ifndef _PALETTEEDITOR_H_
#define _PALETTEEDITOR_H_

#include <QPalette>
#include <QStringList>

#include "colorwidget.h"
#include "ui_paletteeditor.h"

#define QMC2_PALETTEEDITOR_COLUMN_COLORROLE		0
#define QMC2_PALETTEEDITOR_COLUMN_ACTIVE		1
#define QMC2_PALETTEEDITOR_COLUMN_INACTIVE		2
#define QMC2_PALETTEEDITOR_COLUMN_DISABLED		3

#define QMC2_PALETTEEDITOR_COLIDX_WINDOW		0
#define QMC2_PALETTEEDITOR_COLIDX_WINDOWTEXT		1
#define QMC2_PALETTEEDITOR_COLIDX_BASE			2
#define QMC2_PALETTEEDITOR_COLIDX_ALTERNATEBASE		3
#define QMC2_PALETTEEDITOR_COLIDX_TEXT			4
#define QMC2_PALETTEEDITOR_COLIDX_BRIGHTTEXT		5
#define QMC2_PALETTEEDITOR_COLIDX_BUTTON		6
#define QMC2_PALETTEEDITOR_COLIDX_BUTTONTEXT		7
#define QMC2_PALETTEEDITOR_COLIDX_TOOLTIPBASE		8
#define QMC2_PALETTEEDITOR_COLIDX_TOOLTIPTEXT		9
#define QMC2_PALETTEEDITOR_COLIDX_LIGHT			10
#define QMC2_PALETTEEDITOR_COLIDX_MIDLIGHT		11
#define QMC2_PALETTEEDITOR_COLIDX_DARK			12
#define QMC2_PALETTEEDITOR_COLIDX_MID			13
#define QMC2_PALETTEEDITOR_COLIDX_SHADOW		14
#define QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHT		15
#define QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHTEDTEXT	16
#define QMC2_PALETTEEDITOR_COLIDX_LINK			17
#define QMC2_PALETTEEDITOR_COLIDX_LINKVISITED		18

class PaletteEditor : public QDialog, public Ui::PaletteEditor
{
	Q_OBJECT

       	public:
		PaletteEditor(QWidget *parent = 0);
		~PaletteEditor();

		QPalette customPalette;
		QPalette activePalette;
		static QStringList colorNames;

		static QPalette::ColorRole colorNameToRole(QString);
		static QString colorRoleToName(QPalette::ColorRole);

	public slots:
		void adjustIconSizes();
		void colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor);
		void brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush);
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_toolButtonPreview_toggled(bool);
		void on_checkBoxCalculateDetails_toggled(bool);

	protected:
		void showEvent(QShowEvent *);
		void resizeEvent(QResizeEvent *);
		void hideEvent(QHideEvent *);
};

#endif
