#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>
#include <QSpinBox>
#include <QScrollArea>

#include "settings.h"
#include "ui_options.h"
#include "macros.h"

#if QMC2_JOYSTICK == 1
#include "joystick.h"

class JoystickCalibrationWidget : public QWidget
{
	Q_OBJECT

	public:
		Joystick *myJoystick;
		QMap<int, QCheckBox *> axesEnablers;
		QMap<int, QProgressBar *> axesRanges;
		QMap<int, QSpinBox *> axesDeadzones;
		QMap<int, QSpinBox *> axesSensitivities;
		QMap<int, QToolButton *> axesButtons;
		QMap<int, QLabel *> sensitivityLabels;
		QMap<int, QLabel *> deadzoneLabels;
		QMap<int, int> axesMinimums;
		QMap<int, int> axesMaximums;
		QGridLayout *myLayout;

		JoystickCalibrationWidget(Joystick *joystick, QWidget *parent = 0);
		~JoystickCalibrationWidget();

	public slots:
		void on_joystickAxisValueChanged(int, int);
		void on_joystickButtonValueChanged(int, bool);
		void on_joystickHatValueChanged(int, int);
		void on_joystickTrackballValueChanged(int, int, int);
		void on_resetAxisCalibration();
		void on_deadzoneValueChanged(int);
		void on_sensitivityValueChanged(int);
		void on_axisEnablerStateChanged(int);
};

class JoystickTestWidget : public QWidget
{
	Q_OBJECT

	public:
		Joystick *myJoystick;
		QMap<int, QProgressBar *> axesRanges;
		QMap<int, QLabel *> buttonLabels;
		QMap<int, QLabel *> hatValueLabels;
		QMap<int, QLabel *> trackballDeltaXLabels;
		QMap<int, QLabel *> trackballDeltaYLabels;
		QGridLayout *myLayout;

		JoystickTestWidget(Joystick *joystick, QWidget *parent = 0);
		~JoystickTestWidget();

	public slots:
		void on_joystickAxisValueChanged(int, int);
		void on_joystickButtonValueChanged(int, bool);
		void on_joystickHatValueChanged(int, int);
		void on_joystickTrackballValueChanged(int, int, int);
		void cleanupPalette();
};
#endif

class Options : public QDialog, public Ui::Options
{
	Q_OBJECT

	public:
		Settings *config;
		bool applied;
		bool cancelClicked;
#if QMC2_JOYSTICK == 1
		Joystick *joystick;
		JoystickCalibrationWidget *joystickCalibrationWidget;
		JoystickTestWidget *joystickTestWidget;
		QScrollArea *scrollArea;
#endif
		QStringList registeredEmulatorsToBeRemoved;
		static QBrush redBrush;
		static QBrush greenBrush;
		static QBrush lightgreyBrush;
		static QBrush greyBrush;
		static QBrush yellowBrush;
		static QBrush blueBrush;

		Options(QWidget *parent = 0);
		~Options();

		void apply();
		void restoreCurrentConfig(bool useDefaultSettings = false);

		bool useNativeFileDialogs() { return config->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", false).toBool(); }

	public slots:
		// callback functions
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonDefault_clicked();
		void on_toolButtonBrowseStyleSheet_clicked();
		void on_toolButtonBrowseTemporaryFile_clicked();
		void on_toolButtonBrowseFrontendLogFile_clicked();
		void on_toolButtonBrowseEmulatorLogFile_clicked();
		void on_toolButtonBrowseFavoritesFile_clicked();
		void on_toolButtonBrowseHistoryFile_clicked();
		void on_toolButtonBrowseGamelistCacheFile_clicked();
		void on_toolButtonBrowseROMStateCacheFile_clicked();
		void on_toolButtonBrowseSlotInfoCacheFile_clicked();
		void on_toolButtonBrowseMAWSCacheDirectory_clicked();
		void on_toolButtonBrowseSoftwareListCacheDb_clicked();
		void on_toolButtonBrowseSoftwareStateCache_clicked();
		void on_toolButtonBrowseGeneralSoftwareFolder_clicked();
		void on_toolButtonBrowsePreviewDirectory_clicked();
		void on_toolButtonBrowsePreviewFile_clicked();
		void on_toolButtonBrowseFlyerDirectory_clicked();
		void on_toolButtonBrowseFlyerFile_clicked();
		void on_toolButtonBrowseIconDirectory_clicked();
		void on_toolButtonBrowseIconFile_clicked();
		void on_toolButtonBrowseCabinetDirectory_clicked();
		void on_toolButtonBrowseCabinetFile_clicked();
		void on_toolButtonBrowseControllerDirectory_clicked();
		void on_toolButtonBrowseControllerFile_clicked();
		void on_toolButtonBrowseMarqueeDirectory_clicked();
		void on_toolButtonBrowseMarqueeFile_clicked();
		void on_toolButtonBrowseTitleDirectory_clicked();
		void on_toolButtonBrowseTitleFile_clicked();
		void on_toolButtonBrowsePCBDirectory_clicked();
		void on_toolButtonBrowsePCBFile_clicked();
		void on_toolButtonBrowseSoftwareSnapDirectory_clicked();
		void on_toolButtonBrowseSoftwareSnapFile_clicked();
		void on_toolButtonBrowseSoftwareNotesFolder_clicked();
		void on_toolButtonBrowseSoftwareNotesTemplate_clicked();
		void on_toolButtonBrowseSystemNotesFolder_clicked();
		void on_toolButtonBrowseSystemNotesTemplate_clicked();
		void on_toolButtonBrowseExecutableFile_clicked();
		void on_toolButtonBrowseCookieDatabase_clicked();
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
		void on_toolButtonBrowseMAMEVariantExe_clicked();
		void on_toolButtonBrowseMESSVariantExe_clicked();
		void on_toolButtonBrowseUMEVariantExe_clicked();
		void mameVariantSpecifyArguments();
		void messVariantSpecifyArguments();
		void umeVariantSpecifyArguments();
#endif
		void on_toolButtonBrowseDataDirectory_clicked();
		void on_toolButtonBrowseDatInfoDatabase_clicked();
		void on_toolButtonBrowseMameHistoryDat_clicked();
		void on_toolButtonBrowseMessSysinfoDat_clicked();
		void on_toolButtonBrowseMameInfoDat_clicked();
		void on_toolButtonBrowseMessInfoDat_clicked();
		void on_toolButtonBrowseSoftwareInfoDB_clicked();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		void on_toolButtonBrowseCatverIniFile_clicked();
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		void on_toolButtonBrowseCategoryIniFile_clicked();
#endif
		void on_toolButtonBrowseWorkingDirectory_clicked();
		void on_toolButtonBrowseOptionsTemplateFile_clicked();
		void on_toolButtonBrowseXmlCacheDatabase_clicked();
		void on_toolButtonClearUserDataDatabase_clicked();
		void on_toolButtonCleanupUserDataDatabase_clicked();
		void on_toolButtonBrowseUserDataDatabase_clicked();
		void on_toolButtonBrowseFont_clicked();
		void on_toolButtonBrowseLogFont_clicked();
		void on_toolButtonBrowseZipTool_clicked();
		void on_toolButtonBrowseSevenZipTool_clicked();
		void on_toolButtonBrowseRomTool_clicked();
		void on_toolButtonBrowseRomToolWorkingDirectory_clicked();
		void on_radioButtonPreviewSelect_clicked();
		void on_radioButtonFlyerSelect_clicked();
		void on_radioButtonIconSelect_clicked();
		void on_radioButtonCabinetSelect_clicked();
		void on_radioButtonControllerSelect_clicked();
		void on_radioButtonMarqueeSelect_clicked();
		void on_radioButtonTitleSelect_clicked();
		void on_radioButtonPCBSelect_clicked();
		void on_radioButtonSoftwareSnapSelect_clicked();
		void on_treeWidgetShortcuts_itemActivated(QTreeWidgetItem *item);
		void on_treeWidgetShortcuts_itemSelectionChanged();
		void on_pushButtonRedefineKeySequence_clicked();
		void on_pushButtonResetShortcut_clicked();
		void on_pushButtonComponentSetup_clicked();
		void on_pushButtonCustomizeToolBar_clicked();
		void on_pushButtonEditPalette_clicked();
		void on_toolButtonBrowseAdditionalEmulatorExecutable_clicked();
		void on_toolButtonBrowseAdditionalEmulatorWorkingDirectory_clicked();
		void on_toolButtonAddEmulator_clicked();
		void on_toolButtonSaveEmulator_clicked();
		void on_toolButtonRemoveEmulator_clicked();
		void on_toolButtonCustomIDs_clicked();
		void on_tableWidgetRegisteredEmulators_itemSelectionChanged();
		void on_tableWidgetRegisteredEmulators_itemClicked(QTableWidgetItem *) { on_tableWidgetRegisteredEmulators_itemSelectionChanged(); }
		void on_lineEditAdditionalEmulatorName_textChanged(const QString &);
		void on_lineEditAdditionalEmulatorArguments_textChanged(const QString &);
		void on_pushButtonClearCookieDatabase_clicked();
		void on_pushButtonManageCookies_clicked();
		void on_pushButtonAdditionalArtworkSetup_clicked();
		void on_pushButtonImageFormats_clicked();
		void on_toolButtonImportGameInfo_clicked();
		void on_toolButtonImportMachineInfo_clicked();
		void on_toolButtonImportMameInfo_clicked();
		void on_toolButtonImportMessInfo_clicked();
		void on_toolButtonImportSoftwareInfo_clicked();

		// joystick setup callbacks
#if QMC2_JOYSTICK == 1
		void on_pushButtonRescanJoysticks_clicked();
		void on_toolButtonCalibrateAxes_clicked();
		void on_toolButtonTestJoystick_clicked();
		void on_toolButtonMapJoystick_clicked();
		void on_comboBoxSelectJoysticks_currentIndexChanged(int);
		void on_checkBoxEnableJoystickControl_toggled(bool);
		void on_checkBoxJoystickAutoRepeat_toggled(bool);
		void on_spinBoxJoystickAutoRepeatTimeout_valueChanged(int);
		void on_spinBoxJoystickEventTimeout_valueChanged(int);
		void on_treeWidgetJoystickMappings_itemActivated(QTreeWidgetItem *item);
		void on_treeWidgetJoystickMappings_itemSelectionChanged();
		void on_pushButtonRemapJoystickFunction_clicked();
		void on_pushButtonRemoveJoystickMapping_clicked();
		void checkJoystickMappings();
#endif

		// other slots
		void checkShortcuts();
		void setupShortcutActions();
		void applyDelayed();
		void setupCustomIDsClicked();
		void chooseEmuIconClicked();
		void actionDefaultEmuIconTriggered();
		void actionNoEmuIconTriggered();
		void checkPlaceholderStatus();
		void loadCustomPalette(QString);
		void saveCustomPalette();

	protected:
		void moveEvent(QMoveEvent *);
		void resizeEvent(QResizeEvent *);
};

#endif
