#ifndef INDIVIDUALFALLBACKSETTINGS_H
#define INDIVIDUALFALLBACKSETTINGS_H

#include <QTreeWidget>
#include <QStringList>

#include "ui_individualfallbacksettings.h"

class IndividualFallbackSettings : public QDialog, public Ui::IndividualFallbackSettings
{
	Q_OBJECT

       	public:
		IndividualFallbackSettings(QWidget *parent = 0);

		QStringList artworkClassNames;
		QStringList artworkClassDisplayNames;
		QStringList artworkClassIcons;
		QStringList parentFallbackKeys;

	public slots:
		void adjustIconSizes();
		void load();
		void save();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
