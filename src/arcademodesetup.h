#ifndef ARCADEMODESETUP_H
#define ARCADEMODESETUP_H

#include <QStringList>

#include "ui_arcademodesetup.h"
#include "machinelist.h"
#include "rankitemwidget.h"

class ArcadeModeSetup : public QDialog, public Ui::ArcadeModeSetup
{
	Q_OBJECT

       	public:
		ArcadeModeSetup(QWidget *parent = 0);
		~ArcadeModeSetup();

		bool isWritableFile(QString);
		static bool lessThan(const MachineListItem *, const MachineListItem *);

		static QStringList keySequenceMapBases;
#if QMC2_JOYSTICK == 1
		static QStringList joyFunctionMapBases;
		QLabel *joyStatusLabel;
#endif

	public slots:
		void scanCustomKeySequence(QTreeWidgetItem *, int);
		void loadKeySequenceMaps();
		void saveKeySequenceMaps();
		void checkKeySequenceMaps();
#if QMC2_JOYSTICK == 1
		void scanCustomJoyFunction(QTreeWidgetItem *, int);
		void loadJoyFunctionMaps();
		void saveJoyFunctionMaps();
#endif
		void adjustIconSizes();
		void saveSettings();
		void updateCategoryFilter();
		void saveCategoryFilter();
		void on_checkBoxUseFilteredList_toggled(bool);
		void on_checkBoxFavoriteSetsOnly_toggled(bool);
		void on_checkBoxTaggedSetsOnly_toggled(bool);
		void on_pushButtonExport_clicked();
		void on_toolButtonBrowseExecutableFile_clicked();
		void on_toolButtonBrowseWorkingDirectory_clicked();
		void on_toolButtonBrowseConfigurationPath_clicked();
		void on_toolButtonBrowseFilteredListFile_clicked();
		void on_lineEditFilteredListFile_textChanged(QString);
		void on_toolButtonSelectAll_clicked();
		void on_toolButtonDeselectAll_clicked();
		void rankChanged(int);

	private:
		bool m_useCategories;
		bool m_useVersions;
		RankItemWidget *m_rankItemWidget;
		QLabel *m_rankLabel;
};

#endif
