#ifndef _ADDITIONALARTWORKSETUP_H_
#define _ADDITIONALARTWORKSETUP_H_

#include <QHash>
#include <QTreeWidgetItem>

#include "ui_additionalartworksetup.h"

class AdditionalArtworkSetup : public QDialog, public Ui::AdditionalArtworkSetup
{
	Q_OBJECT

       	public:
		AdditionalArtworkSetup(QWidget *parent = 0);
		~AdditionalArtworkSetup();

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_toolButtonAdd_clicked();
		void on_toolButtonRemove_clicked();
		void selectionFlagsChanged(bool checked = false);
		void toggleFormatEnabled(int);
		void dataChanged(const QString &) { pushButtonRestore->setEnabled(true); }
		void dataChanged(int index = 0) { pushButtonRestore->setEnabled(true); }
		void pathBrowsed(QString);
		void chooseIcon();
		void load();
		void save();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);

	private:
		QHash<int, QTreeWidgetItem *> m_itemHash;
		int m_seq;
};

#endif
