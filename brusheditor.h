#ifndef _BRUSHEDITOR_H_
#define _BRUSHEDITOR_H_

#include "ui_brusheditor.h"

#define QMC2_BRUSHEDITOR_PATTERN_NOBRUSH		0
#define QMC2_BRUSHEDITOR_PATTERN_SOLIDPATTERN		1
#define QMC2_BRUSHEDITOR_PATTERN_DENSE1PATTERN		2
#define QMC2_BRUSHEDITOR_PATTERN_DENSE2PATTERN		3
#define QMC2_BRUSHEDITOR_PATTERN_DENSE3PATTERN		4
#define QMC2_BRUSHEDITOR_PATTERN_DENSE4PATTERN		5
#define QMC2_BRUSHEDITOR_PATTERN_DENSE5PATTERN		6
#define QMC2_BRUSHEDITOR_PATTERN_DENSE6PATTERN		7
#define QMC2_BRUSHEDITOR_PATTERN_DENSE7PATTERN		8
#define QMC2_BRUSHEDITOR_PATTERN_HORPATTERN		9
#define QMC2_BRUSHEDITOR_PATTERN_VERPATTERN		10
#define QMC2_BRUSHEDITOR_PATTERN_CROSSPATTERN		11
#define QMC2_BRUSHEDITOR_PATTERN_BDIAGPATTERN		12
#define QMC2_BRUSHEDITOR_PATTERN_FDIAGPATTERN		13
#define QMC2_BRUSHEDITOR_PATTERN_DIAGCROSSPATTERN	14
#define QMC2_BRUSHEDITOR_PATTERN_TEXTUREPATTERN		15

#define QMC2_BRUSHEDITOR_GRADIENT_LINEAR		0
#define QMC2_BRUSHEDITOR_GRADIENT_RADIAL		1
#define QMC2_BRUSHEDITOR_GRADIENT_CONICAL		2

class BrushEditor : public QDialog, public Ui::BrushEditor
{
	Q_OBJECT

       	public:
		BrushEditor(QWidget *parent = 0);
		~BrushEditor();

		static QStringList patternNames;

		static Qt::BrushStyle patternNameToStyle(QString);
		static QString patternStyleToName(Qt::BrushStyle);

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_toolButtonBrowseImageFile_clicked();
		void on_toolButtonPatternColor_clicked();
		void on_toolButtonBrowseTextureImage_clicked();
		void on_toolButtonAddColorStop_clicked();
		void on_toolButtonRemoveColorStop_clicked();
		void on_comboBoxPatternType_currentIndexChanged(int);
		void on_comboBoxGradientType_currentIndexChanged(int);

	protected:
		void showEvent(QShowEvent *);
};

#endif
