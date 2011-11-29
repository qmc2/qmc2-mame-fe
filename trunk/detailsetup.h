#ifndef _DETAILSETUP_H_
#define _DETAILSETUP_H_

#include <QMap>
#include <QIcon>
#include "ui_detailsetup.h"

class DetailSetup : public QDialog, public Ui::DetailSetup
{
	Q_OBJECT

	public:
		QMap<int, QString> shortTitleMap;
		QMap<int, QString> longTitleMap;
		QMap<int, QIcon> iconMap;
		QList<int> availableDetailList;
		QList<int> configurableDetailList;
		QList<int> activeDetailList;
		QList<int> appliedDetailList;
		QMap<int, QWidget *> tabWidgetsMap;

		DetailSetup(QWidget *parent = 0);
		~DetailSetup();

	public slots:
		void loadDetail();
		void saveDetail();
		void adjustIconSizes();

		// automatically connected slots
		void on_listWidgetAvailableDetails_itemSelectionChanged(); 
		void on_listWidgetActiveDetails_itemSelectionChanged(); 
		void on_pushButtonActivateDetails_clicked();
		void on_pushButtonConfigureDetail_clicked();
		void on_pushButtonDeactivateDetails_clicked();
		void on_pushButtonDetailsUp_clicked();
		void on_pushButtonDetailsDown_clicked();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonCancel_clicked();
};

#endif
