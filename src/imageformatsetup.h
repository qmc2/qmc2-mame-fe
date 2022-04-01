#ifndef IMAGEFORMATSETUP_H
#define IMAGEFORMATSETUP_H

#include <QMap>
#include <QList>
#include <QStringList>

#include "ui_imageformatsetup.h"

class ImageFormatSetup : public QDialog, public Ui::ImageFormatSetup
{
	Q_OBJECT

       	public:
		ImageFormatSetup(QWidget *parent = 0);

		QStringList artworkClassPrefixes;
		QStringList artworkClassNames;
		QStringList artworkClassIcons;

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_comboBoxImageType_currentIndexChanged(int);
		void on_treeWidget_itemClicked(QTreeWidgetItem *, int);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);

	private:
		void restoreActiveFormats(bool init = false);

		QMap<QString, QList<int> > m_activeFormats;
		int m_previousClassIndex;

	private slots:
		void checkForModifications();
		void rowsInserted(const QModelIndex &, int, int);
};

#endif
