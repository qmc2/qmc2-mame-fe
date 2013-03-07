#ifndef _PALETTEEDITOR_H_
#define _PALETTEEDITOR_H_

#include <QMap>
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
#define QMC2_PALETTEEDITOR_COLIDX_DARK			11
#define QMC2_PALETTEEDITOR_COLIDX_MID			12
#define QMC2_PALETTEEDITOR_COLIDX_SHADOW		13
#define QMC2_PALETTEEDITOR_COLIDX_HIGHLIGHT		14
#define QMC2_PALETTEEDITOR_COLIDX_LINK			15
#define QMC2_PALETTEEDITOR_COLIDX_LINKVISITED		16

class PaletteEditor : public QDialog, public Ui::PaletteEditor
{
	Q_OBJECT

       	public:
		PaletteEditor(QWidget *parent = 0);
		~PaletteEditor();

		QPalette customPalette;
		QStringList colorNames;
		QMap<QString, ColorWidget *> activeColorWidgets;
		QMap<QString, ColorWidget *> inactiveColorWidgets;
		QMap<QString, ColorWidget *> disabledColorWidgets;

		QPalette::ColorRole colorNameToRole(QString);

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonPreview_clicked();

	protected:
		void showEvent(QShowEvent *);
		void resizeEvent(QResizeEvent *);
};

#endif
