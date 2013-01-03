#ifndef _ARCADEMODESETUP_H_
#define _ARCADEMODESETUP_H_

#include "ui_arcademodesetup.h"

class ArcadeModeSetup : public QDialog, public Ui::ArcadeModeSetup
{
	Q_OBJECT

       	public:
		ArcadeModeSetup(QWidget *parent = 0);
		~ArcadeModeSetup();

		bool isWritableFile(QString);

	public slots:
		void adjustIconSizes();
		void saveSettings();
		void updateCategoryFilter();
		void saveCategoryFilter();
		void on_checkBoxUseFilteredList_toggled(bool);
		void on_pushButtonExport_clicked();
		void on_toolButtonBrowseExecutableFile_clicked();
		void on_toolButtonBrowseWorkingDirectory_clicked();
		void on_toolButtonBrowseConfigurationPath_clicked();
		void on_toolButtonBrowseFilteredListFile_clicked();
		void on_lineEditFilteredListFile_textChanged(QString);
};

#endif
