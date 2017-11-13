#include <QCryptographicHash>
#include <QStyleFactory>
#include <QApplication>
#include <QStringList>
#include <QFileDialog>
#include <QTranslator>
#include <QTextStream>
#include <QLineEdit>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QFont>
#include <QFile>
#include <QDir>

#include "setupwizard.h"

// external global variables
extern bool qmc2TemplateCheck;
extern QTranslator *qmc2Translator;
extern QTranslator *qmc2QtTranslator;

CustomSettings::CustomSettings(QSettings *cfg, QObject *parent) :
	QObject(parent)
{
	loadFrom(cfg);
}

void CustomSettings::loadFrom(QSettings *cfg)
{
	clear();
	if ( cfg )
		foreach (QString key, cfg->allKeys())
			m_settingsHash.insert(key, cfg->value(key));
}

void CustomSettings::saveTo(QSettings *cfg)
{
	if ( cfg ) {
		cfg->clear();
		foreach (QString key, m_settingsHash.uniqueKeys())
			cfg->setValue(key, m_settingsHash.value(key));
	}
}

void CustomSettings::setValue(const QString &key, const QVariant &value)
{
	m_settingsHash.insert(key, value);
}

QVariant CustomSettings::value(const QString &key, const QVariant &defaultValue)
{
	if ( m_settingsHash.contains(key) )
		return m_settingsHash.value(key);
	else
		return defaultValue;
}

void CustomSettings::remove(const QString &key)
{
	if ( m_settingsHash.contains(key) )
		m_settingsHash.remove(key);
}

SetupWizard::SetupWizard(QSettings *cfg, QWidget *parent) :
	QWizard(parent),
	m_startupConfig(cfg),
	m_minRequiredMameVersionMinor(0),
	m_minRequiredMameVersionMajor(183),
	m_totalMachines(-1),
	m_modificationTime(-1)
{
	// remember the default style
	m_defaultStyle = QApplication::style()->objectName();

	// initialize available languages and mewUi-to-QMC2 mappings
	m_availableLanguages << "de" << "es" << "el" << "fr" << "it" << "pl" << "pt" << "ro" << "sv" << "us";
	m_uiToQmc2Hash.insert("cabinets_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetDirectory");
	m_uiToQmc2Hash.insert("cpanels_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerDirectory");
	m_uiToQmc2Hash.insert("pcbs_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBDirectory");
	m_uiToQmc2Hash.insert("flyers_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerDirectory");
	m_uiToQmc2Hash.insert("titles_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleDirectory");
	m_uiToQmc2Hash.insert("marquees_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeDirectory");
	m_uiToQmc2Hash.insert("logos_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeDirectory");
	m_uiToQmc2Hash.insert("icons_directory", QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconDirectory");

	// create custom settings based on the stored ones
	m_customSettings = new CustomSettings(m_startupConfig, this);

	// set customized UI font when applicable
	if ( m_customSettings->contains(QMC2_FRONTEND_PREFIX + "GUI/Font") ) {
		QFont f;
		f.fromString(m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
		qApp->setFont(f);
	}

	// now setup the UI
	setupUi(this);

	// setting explicit button texts makes it possible to translate them dynamically (however, some styles ignore this)
	setButtonText(QWizard::NextButton, tr("&Next >"));
	setButtonText(QWizard::BackButton, tr("< &Back"));
	setButtonText(QWizard::CancelButton, tr("&Cancel"));
	setButtonText(QWizard::FinishButton, tr("&Finish"));

	// we want to show our version :)...
#if QMC2_SVN_REV > 0
	labelVersion->setText(QString("QMC2 v%1 (SVN r%2)").arg(XSTR(QMC2_VERSION)).arg(QMC2_SVN_REV));
#else
	labelVersion->setText(QString("QMC2 v%1").arg(XSTR(QMC2_VERSION)));
#endif

	// replace labels with clickable ones
	gridLayout3->removeWidget(labelImportMameIni);
	delete labelImportMameIni;
	m_labelImportMameIni = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportMameIni, 3, 1);
	connect(m_labelImportMameIni, SIGNAL(clicked()), this, SLOT(labelImportMameIni_clicked()));

	gridLayout3->removeWidget(labelImportUiIni);
	delete labelImportUiIni;
	m_labelImportUiIni = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportUiIni, 4, 1);
	connect(m_labelImportUiIni, SIGNAL(clicked()), this, SLOT(labelImportUiIni_clicked()));

	gridLayout3->removeWidget(labelImportBothInis);
	delete labelImportBothInis;
	m_labelImportBothInis = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportBothInis, 5, 1);
	connect(m_labelImportBothInis, SIGNAL(clicked()), this, SLOT(labelImportBothInis_clicked()));

	gridLayout3->removeWidget(labelImportNothing);
	delete labelImportNothing;
	m_labelImportNothing = new ClickableLabel(this);
	gridLayout3->addWidget(m_labelImportNothing, 6, 1);
	connect(m_labelImportNothing, SIGNAL(clicked()), this, SLOT(labelImportNothing_clicked()));

	// fill the language combo-box
	comboBoxLanguage->blockSignals(true);
	comboBoxLanguage->addItems(m_availableLanguages);
	int index = comboBoxLanguage->findText(m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString());
	if ( index >= 0 )
		comboBoxLanguage->setCurrentIndex(index);
	comboBoxLanguage->blockSignals(false);

	comboBoxStyle->blockSignals(true);
	comboBoxStyle->addItem(QObject::tr("Default"));
	comboBoxStyle->addItems(QStyleFactory::keys());
	QString myStyle(QObject::tr((const char *)m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString().toUtf8()));
	int styleIndex = comboBoxStyle->findText(myStyle, Qt::MatchFixedString);
	if ( styleIndex < 0 )
		styleIndex = 0;
	comboBoxStyle->setCurrentIndex(styleIndex);
	setupStyle(comboBoxStyle->currentText());
	comboBoxStyle->blockSignals(false);

	// connections and start-up
	connect(comboBoxStyle, SIGNAL(activated(const QString &)), this, SLOT(setupStyle(const QString &)));
	connect(comboBoxExecutableFile->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(comboBoxExecutableFile_textChanged(const QString &)));
	QTimer::singleShot(0, this, SLOT(init()));
}

SetupWizard::~SetupWizard()
{
	delete m_customSettings;
}

void SetupWizard::accept()
{
	m_customSettings->saveTo(m_startupConfig);
	m_startupConfig->sync();
	QWizard::accept();
}

void SetupWizard::init()
{
	setupLanguage();
	//button(QWizard::NextButton)->setEnabled(false);
	QStringList emuHistory(m_customSettings->value(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", QStringList()).toStringList());
	emuHistory.sort();
	for (int i = 0; i < emuHistory.count(); i++) {
		QString emuPath(emuHistory.at(i));
		QFileInfo fi(emuPath);
		if ( fi.exists() && fi.isReadable() && fi.isExecutable() && fi.isFile() )
			comboBoxExecutableFile->insertItem(i, emuPath);
	}
	comboBoxExecutableFile->lineEdit()->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString());
	int index = comboBoxExecutableFile->findText(comboBoxExecutableFile->lineEdit()->text());
	if ( index >= 0 )
		comboBoxExecutableFile->setCurrentIndex(index);
}

void SetupWizard::log(const QString &message)
{
	plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void SetupWizard::probeExecutable()
{
	button(QWizard::NextButton)->setEnabled(false);
	m_totalMachines = -1;
	m_modificationTime = -1;
	m_listfullSha1.clear();
	m_emuConfigName.clear();
	QFileInfo fi(comboBoxExecutableFile->currentText());
	if ( fi.isReadable() && fi.isExecutable() ) {
		labelFileIsExecutableResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + "</b></font>");
		QStringList emulatorIdentifiers, emulatorConfigNames;
		emulatorIdentifiers << "MAME" << "M.A.M.E." << "HBMAME" << "HB.M.A.M.E." << "MESS" << "M.E.S.S.";
		emulatorConfigNames << "mame" << "mame" << "hbmame" << "hbmame" << "mess" << "mess";
		QProcess commandProc;
		bool started = false, commandProcStarted = false;
		int retries = 0;
		commandProc.start(comboBoxExecutableFile->currentText(), QStringList() << "-help");
		started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
		while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
			qApp->processEvents();
			started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
		}
		if ( started ) {
			commandProcStarted = true;
			bool commandProcRunning = (commandProc.state() == QProcess::Running);
			while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
				qApp->processEvents();
				commandProcRunning = (commandProc.state() == QProcess::Running);
			}
			if ( commandProcStarted ) {
				QString output(commandProc.readAllStandardOutput());
#if defined(QMC2_OS_WIN)
				output.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
				QStringList versionLines(output.split('\n'));
				QStringList versionWords(versionLines.first().split(' '));
				if ( versionWords.count() > 1 ) {
					if ( emulatorIdentifiers.contains(versionWords.first()) ) {
						labelIdentifiedAsMameResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + " (" + versionWords.first() + ")</b></font>");
						m_emuConfigName = emulatorConfigNames.at(emulatorIdentifiers.indexOf(versionWords.first()));
						QStringList emulatorVersionInfo(versionWords[1].remove('v').split('.'));
						if ( emulatorVersionInfo.count() > 1 ) {
							int verMinor = emulatorVersionInfo.at(0).toInt();
							int verMajor = emulatorVersionInfo.at(1).toInt();
							if ( verMinor >= m_minRequiredMameVersionMinor && verMajor >= m_minRequiredMameVersionMajor ) {
								labelVersionSupportedResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("Yes") + " (" + versionWords.at(1) + ")</b></font>");
							} else
								labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No") + " (" + versionWords.at(1) + ", " + tr("%1.%2+ required").arg(m_minRequiredMameVersionMinor).arg(m_minRequiredMameVersionMajor) + ")</b></font>");
						} else
							labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("can't parse version info") + ")</b></font>");
						commandProcStarted = false;
						retries = 0;
						commandProc.start(comboBoxExecutableFile->currentText(), QStringList() << "-listfull");
						started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
						while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
							qApp->processEvents();
							started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
						}
						if ( started ) {
							commandProcStarted = true;
							commandProcRunning = (commandProc.state() == QProcess::Running);
							while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
								qApp->processEvents();
								commandProcRunning = (commandProc.state() == QProcess::Running);
							}
							if ( commandProcStarted ) {
								QCryptographicHash sha1(QCryptographicHash::Sha1);
								QString lfOutput(commandProc.readAllStandardOutput());
								m_totalMachines = lfOutput.count('\n') - 1;
								sha1.addData(lfOutput.toUtf8().constData());
								m_listfullSha1 = sha1.result().toHex();
								labelTotalMachinesResult->setText("<font color=\"green\" size=\"4\"><b>" + QString::number(m_totalMachines) + "</b></font>");
								labelBinaryIdentHashResult->setText("<font color=\"green\" size=\"4\"><b>" + m_listfullSha1 + "</b></font>");
							} else {
								labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
								labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
							}
						} else {
							labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
							labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("Unknown") + " (" + tr("emulator didn't start") + "</b></font>");
						}
						m_modificationTime = fi.lastModified().toTime_t();
						labelFileModificationDateResult->setText("<font color=\"green\" size=\"4\"><b>" + fi.lastModified().toString(Qt::SystemLocaleShortDate) + "</b></font>");
						if ( findIniFiles() ) {
							if ( m_emulatorIniPath.isEmpty() )
								labelEmulatorIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("No ini-file found") + "</b></font>");
							else
								labelEmulatorIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + m_emulatorIniPath + "</b></font>");
							if ( m_frontendIniPath.isEmpty() )
								labelFrontendIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + tr("No ini-file found") + "</b></font>");
							else
								labelFrontendIniFileLocationResult->setText("<font color=\"green\" size=\"4\"><b>" + m_frontendIniPath + "</b></font>");
							button(QWizard::NextButton)->setEnabled(true);
						} else {
							labelEmulatorIniFileLocation->setText("<font color=\"red\" size=\"4\"><b>" + tr("Couldn't determine ini-path") + "</b></font>");
							labelFrontendIniFileLocation->setText("<font color=\"red\" size=\"4\"><b>" + tr("Couldn't determine ini-path") + "</b></font>");
						}
					} else {
						labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + " (" + tr("incompatible binary") + ")</b></font>");
						labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
						labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					}
				} else {
					labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + " (" + tr("incompatible binary") + ")</b></font>");
					labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
					labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				}
			} else {
				labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + " (" + tr("emulator didn't start") + ")</b></font>");
				labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
				labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			}
		} else {
			labelIdentifiedAsMameResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + "(" + tr("emulator didn't start") + ")</b></font>");
			labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
			labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		}
	} else {
		labelFileIsExecutableResult->setText("<font color=\"red\" size=\"4\"><b>" + tr("No") + "</b></font>");
		labelIdentifiedAsMameResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelVersionSupportedResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelTotalMachinesResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelBinaryIdentHashResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelFileModificationDateResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelEmulatorIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
		labelFrontendIniFileLocationResult->setText("<font color=\"sandybrown\" size=\"4\"><b>" + tr("No result") + "</b></font>");
	}
}

void SetupWizard::importMameIni()
{
	button(QWizard::NextButton)->setEnabled(false);
	log(tr("importing emulator settings from '%1'").arg(m_emulatorIniPath));

	// IMPORTANT: these string lists have to be kept up-to-date using the bash script "scripts/generate-option-lists.sh"!!!
	QStringList allOptions(QStringList() << "adstick_device" << "artpath" << "artwork_crop" << "aspect" << "aspect0" << "aspect1" << "aspect2" << "aspect3" << "audiodriver" << "audio_effect0" << "audio_effect1" << "audio_effect2" << "audio_effect3" << "audio_effect4" << "audio_effect5" << "audio_effect6" << "audio_effect7" << "audio_effect8" << "audio_effect9" << "audio_latency" << "audio_output" << "autoboot_command" << "autoboot_delay" << "autoboot_script" << "autoframeskip" << "autorol" << "autoror" << "autosave" << "autostretchxy" << "aviwrite" << "beam_intensity_weight" << "beam_width_max" << "beam_width_min" << "bench" << "bgfx_avi_name" << "bgfx_backend" << "bgfx_debug" << "bgfx_path" << "bgfx_screen_chains" << "bgfx_shadow_mask" << "bios" << "bloom_blend_mode" << "bloom_lvl0_weight" << "bloom_lvl1_weight" << "bloom_lvl2_weight" << "bloom_lvl3_weight" << "bloom_lvl4_weight" << "bloom_lvl5_weight" << "bloom_lvl6_weight" << "bloom_lvl7_weight" << "bloom_lvl8_weight" << "bloom_overdrive" << "bloom_scale" << "blu_ratio" << "brightness" << "burnin" << "centerh" << "centerv" << "cfg_directory" << "cheat" << "cheatpath" << "coin_impulse" << "coin_lockout" << "comment_directory" << "comm_localhost" << "comm_localport" << "comm_remotehost" << "comm_remoteport" << "confirm_quit" << "console" << "contrast" << "converge_x" << "converge_y" << "crosshairpath" << "ctrlr" << "ctrlrpath" << "cubic_distortion" << "debug" << "debugger" << "debugger_font" << "debugger_font_size" << "debugscript" << "defocus" << "dial_device" << "diff_directory" << "distort_corner" << "distortion" << "drc" << "drc_log_native" << "drc_log_uml" << "drc_use_c" << "dual_lightgun" << "effect" << "exit_after_playback" << "filter" << "flicker" << "flipx" << "flipy" << "floor" << "fontpath" << "frameskip" << "full_screen_brightness" << "full_screen_contrast" << "full_screen_gamma" << "gamma" << "gl_forcepow2texture" << "gl_glsl" << "gl_glsl_filter" << "gl_lib" << "gl_notexturerect" << "global_inputs" << "gl_pbo" << "glsl_shader_mame0" << "glsl_shader_mame1" << "glsl_shader_mame2" << "glsl_shader_mame3" << "glsl_shader_mame4" << "glsl_shader_mame5" << "glsl_shader_mame6" << "glsl_shader_mame7" << "glsl_shader_mame8" << "glsl_shader_mame9" << "glsl_shader_screen0" << "glsl_shader_screen1" << "glsl_shader_screen2" << "glsl_shader_screen3" << "glsl_shader_screen4" << "glsl_shader_screen5" << "glsl_shader_screen6" << "glsl_shader_screen7" << "glsl_shader_screen8" << "glsl_shader_screen9" << "gl_vbo" << "grn_ratio" << "hashpath" << "hlsl_enable" << "hlsl_oversampling" << "hlslpath" << "hlsl_snap_height" << "hlsl_snap_width" << "hlsl_write" << "homepath" << "http" << "http_port" << "http_root" << "hum_bar_alpha" << "inipath" << "input_directory" << "intoverscan" << "intscalex" << "intscaley" << "joy_idx1" << "joy_idx2" << "joy_idx3" << "joy_idx4" << "joy_idx5" << "joy_idx6" << "joy_idx7" << "joy_idx8" << "joystick" << "joystick_contradictory" << "joystick_deadzone" << "joystick_map" << "joystickprovider" << "joystick_saturation" << "keepaspect" << "keyb_idx1" << "keyb_idx2" << "keyb_idx3" << "keyb_idx4" << "keyb_idx5" << "keyb_idx6" << "keyb_idx7" << "keyb_idx8" << "keyboardprovider" << "keymap" << "keymap_file" << "language" << "languagepath" << "lightgun" << "lightgun_device" << "lightgun_index1" << "lightgun_index2" << "lightgun_index3" << "lightgun_index4" << "lightgun_index5" << "lightgun_index6" << "lightgun_index7" << "lightgun_index8" << "lightgunprovider" << "log" << "maximize" << "menu" << "mngwrite" << "monitorprovider" << "mouse" << "mouse_device" << "mouse_index1" << "mouse_index2" << "mouse_index3" << "mouse_index4" << "mouse_index5" << "mouse_index6" << "mouse_index7" << "mouse_index8" << "mouseprovider" << "multikeyboard" << "multimouse" << "natural" << "noplugin" << "numprocessors" << "numscreens" << "nvram_directory" << "offscreen_reload" << "offset" << "oslog" << "output" << "pa_api" << "paddle_device" << "pa_device" << "pa_latency" << "pause_brightness" << "pedal_device" << "phosphor_life" << "playback" << "plugin" << "plugins" << "pluginspath" << "positional_device" << "power" << "prescale" << "priority" << "profile" << "radial_converge_x" << "radial_converge_y" << "ramsize" << "readconfig" << "record" << "record_timecode" << "red_ratio" << "reflection" << "refreshspeed" << "renderdriver" << "resolution" << "resolution0" << "resolution1" << "resolution2" << "resolution3" << "rol" << "rompath" << "ror" << "rotate" << "round_corner" << "samplepath" << "samplerate" << "samples" << "saturation" << "scale" << "scalemode" << "scanline_alpha" << "scanline_bright_offset" << "scanline_bright_scale" << "scanline_height" << "scanline_jitter" << "scanline_size" << "scanline_variation" << "screen" << "screen0" << "screen1" << "screen2" << "screen3" << "sdlvideofps" << "seconds_to_run" << "shadow_mask_alpha" << "shadow_mask_texture" << "shadow_mask_tile_mode" << "shadow_mask_uoffset" << "shadow_mask_usize" << "shadow_mask_voffset" << "shadow_mask_vsize" << "shadow_mask_x_count" << "shadow_mask_y_count" << "sixaxis" << "skip_gameinfo" << "sleep" << "smooth_border" << "snapbilinear" << "snapname" << "snapshot_directory" << "snapsize" << "snapview" << "sound" << "speed" << "state" << "state_directory" << "statename" << "steadykey" << "switchres" << "swpath" << "syncrefresh" << "throttle" << "trackball_device" << "triplebuffer" << "ui" << "ui_active" << "uifont" << "uifontprovider" << "uimodekey" << "ui_mouse" << "unevenstretch" << "unevenstretchx" << "unevenstretchy" << "update_in_pause" << "useallheads" << "use_backdrops" << "use_bezels" << "use_cpanels" << "use_marquees" << "use_overlays" << "vector_beam_smooth" << "vector_length_ratio" << "vector_length_scale" << "verbose" << "video" << "videodriver" << "view" << "view0" << "view1" << "view2" << "view3" << "vignetting" << "volume" << "waitvsync" << "watchdog" << "wavwrite" << "window" << "writeconfig" << "yiq_a" << "yiq_b" << "yiq_cc" << "yiq_enable" << "yiq_i" << "yiq_jitter" << "yiq_n" << "yiq_o" << "yiq_p" << "yiq_phase_count" << "yiq_q" << "yiq_scan_time" << "yiq_y");
	QStringList booleanOptions(QStringList() << "artwork_crop" << "autoframeskip" << "autorol" << "autoror" << "autosave" << "autostretchxy" << "bgfx_debug" << "burnin" << "centerh" << "centerv" << "cheat" << "coin_lockout" << "confirm_quit" << "console" << "debug" << "drc" << "drc_log_native" << "drc_log_uml" << "drc_use_c" << "dual_lightgun" << "exit_after_playback" << "filter" << "flipx" << "flipy" << "gl_forcepow2texture" << "gl_glsl" << "gl_notexturerect" << "global_inputs" << "gl_pbo" << "gl_vbo" << "hlsl_enable" << "hlsl_oversampling" << "http" << "intoverscan" << "joystick" << "joystick_contradictory" << "keepaspect" << "keymap" << "lightgun" << "log" << "maximize" << "menu" << "mouse" << "multikeyboard" << "multimouse" << "natural" << "offscreen_reload" << "oslog" << "plugins" << "readconfig" << "record_timecode" << "refreshspeed" << "rol" << "ror" << "rotate" << "samples" << "sdlvideofps" << "sixaxis" << "skip_gameinfo" << "sleep" << "snapbilinear" << "steadykey" << "switchres" << "syncrefresh" << "throttle" << "triplebuffer" << "ui_active" << "ui_mouse" << "unevenstretch" << "unevenstretchx" << "unevenstretchy" << "update_in_pause" << "useallheads" << "use_backdrops" << "use_bezels" << "use_cpanels" << "use_marquees" << "use_overlays" << "verbose" << "waitvsync" << "window" << "writeconfig" << "yiq_enable");
	QStringList floatOptions(QStringList() << "beam_intensity_weight" << "beam_width_max" << "beam_width_min" << "bloom_lvl0_weight" << "bloom_lvl1_weight" << "bloom_lvl2_weight" << "bloom_lvl3_weight" << "bloom_lvl4_weight" << "bloom_lvl5_weight" << "bloom_lvl6_weight" << "bloom_lvl7_weight" << "bloom_lvl8_weight" << "bloom_overdrive" << "bloom_scale" << "blu_ratio" << "brightness" << "contrast" << "converge_x" << "converge_y" << "cubic_distortion" << "defocus" << "distort_corner" << "distortion" << "flicker" << "floor" << "full_screen_brightness" << "full_screen_contrast" << "full_screen_gamma" << "gamma" << "grn_ratio" << "hum_bar_alpha" << "joystick_deadzone" << "joystick_saturation" << "offset" << "pa_latency" << "pause_brightness" << "phosphor_life" << "power" << "radial_converge_x" << "radial_converge_y" << "red_ratio" << "reflection" << "round_corner" << "saturation" << "scale" << "scanline_alpha" << "scanline_bright_offset" << "scanline_bright_scale" << "scanline_height" << "scanline_jitter" << "scanline_size" << "scanline_variation" << "shadow_mask_alpha" << "shadow_mask_uoffset" << "shadow_mask_usize" << "shadow_mask_voffset" << "shadow_mask_vsize" << "smooth_border" << "speed" << "vector_beam_smooth" << "vector_length_ratio" << "vector_length_scale" << "vignetting" << "yiq_a" << "yiq_b" << "yiq_cc" << "yiq_i" << "yiq_jitter" << "yiq_n" << "yiq_o" << "yiq_p" << "yiq_q" << "yiq_scan_time" << "yiq_y");
	QStringList ignoredOptions(QStringList() << "dtd");

	QFile iniFile(m_emulatorIniPath);
	if ( iniFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream ts(&iniFile);
		int lineCounter = 0;
		while ( !ts.atEnd() ) {
			if ( lineCounter % 10 )
				qApp->processEvents();
			QString lineTrimmed(ts.readLine().trimmed());
			if ( !lineTrimmed.isEmpty() && !lineTrimmed.startsWith('#') && !lineTrimmed.startsWith("<UNADORNED") ) {
				QStringList tokens(lineTrimmed.split(QRegExp("\\s+"), QString::SkipEmptyParts));
				if ( tokens.count() > 1 ) {
					QString option(tokens.at(0));
					QString value(lineTrimmed.mid(lineTrimmed.indexOf(tokens.at(1), tokens.at(0).length())));
					if ( ignoredOptions.contains(option) ) {
						log(tr("option '%1' with value '%2' ignored").arg(option).arg(value.replace("$HOME", "~")));
						continue;
					}
					if ( !allOptions.contains(option) ) {
						log(tr("WARNING: unknown option '%1' on line %2 ignored").arg(option).arg(lineCounter + 1));
						continue;
					}
					if ( booleanOptions.contains(option) )
						value = value == "1" ? "true" : "false";
					else if ( floatOptions.contains(option) ) {
						QStringList floatParts = value.split(',', QString::SkipEmptyParts);
						QStringList newValues;
						for (int i = 0; i < floatParts.count(); i++) {
							double num = floatParts.at(i).toDouble();
							newValues << QString::number(num, 'f', QMC2_EMUOPT_DFLT_DECIMALS);
						}
						value = newValues.join(",");
					}
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + option, value.replace("$HOME", "~"));
					log(tr("option '%1' with value '%2' imported").arg(option).arg(value));
				} else if ( tokens.count() > 0 ) {
					if ( allOptions.contains(tokens.at(0)) )
						log(tr("WARNING: missing value on line %1, option '%2' ignored").arg(lineCounter + 1).arg(tokens.at(0)));
				}
			}
			lineCounter++;
		}
		iniFile.close();
	} else
		log(tr("ERROR: can't open '%1' for reading").arg(m_emulatorIniPath));
	log(tr("done (importing emulator settings from '%1')").arg(m_emulatorIniPath));
	if ( radioButtonImportBothInis->isChecked() )
		QTimer::singleShot(0, this, SLOT(importUiIni()));
	else
		button(QWizard::NextButton)->setEnabled(true);
}

void SetupWizard::importUiIni()
{
	button(QWizard::NextButton)->setEnabled(false);
	log(tr("importing front-end settings from '%1'").arg(m_frontendIniPath));
	QFile iniFile(m_frontendIniPath);
	if ( iniFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream ts(&iniFile);
		int lineCounter = 0;
		while ( !ts.atEnd() ) {
			if ( lineCounter % 10 )
				qApp->processEvents();
			QString lineTrimmed(ts.readLine().trimmed());
			if ( !lineTrimmed.isEmpty() && !lineTrimmed.startsWith('#') && !lineTrimmed.startsWith("<UNADORNED") ) {
				QStringList tokens(lineTrimmed.split(QRegExp("\\s+"), QString::SkipEmptyParts));
				if ( tokens.count() > 1 ) {
					QString option(tokens.at(0));
					QString value(lineTrimmed.mid(lineTrimmed.indexOf(tokens.at(1), tokens.at(0).length())));
					if ( m_uiToQmc2Hash.contains(option) ) {
						QString currentValue(m_customSettings->value(m_uiToQmc2Hash.value(option)).toString());
						value.replace("$HOME", "~");
						if ( currentValue.isEmpty() )
							m_customSettings->setValue(m_uiToQmc2Hash.value(option), value);
						else
							m_customSettings->setValue(m_uiToQmc2Hash.value(option), currentValue + ';' + value);
						log(tr("option '%1' with value '%2' imported").arg(option).arg(value));
					} else
						log(tr("option '%1' with value '%2' ignored").arg(option).arg(value.replace("$HOME", "~")));
				} else if ( tokens.count() > 0 ) {
					if ( m_uiToQmc2Hash.contains(tokens.at(0)) )
						log(tr("WARNING: missing value on line %1, option '%2' ignored").arg(lineCounter + 1).arg(tokens.at(0)));
				}
			}
			lineCounter++;
		}
		iniFile.close();
	} else
		log(tr("ERROR: can't open '%1' for reading").arg(m_frontendIniPath));
	log(tr("done (importing front-end settings from '%1')").arg(m_frontendIniPath));
	button(QWizard::NextButton)->setEnabled(true);
}

int SetupWizard::nextId() const
{
	switch ( currentId() ) {
		case QMC2_SETUPWIZARD_PAGE_ID_WELCOME:
			return QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE;
		case QMC2_SETUPWIZARD_PAGE_ID_CHOOSE_EXECUTABLE:
			return QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE;
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			if ( !m_emulatorIniPath.isEmpty() || !m_frontendIniPath.isEmpty() )
				return QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES;
			else
				return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES:
			if ( radioButtonImportNothing->isChecked() )
				return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS;
			else
				return QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES:
			return QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS;
		case QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS:
			return QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE;
		case QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE:
			return -1;
	}
}

void SetupWizard::initializePage(int id)
{
	switch ( id ) {
		case QMC2_SETUPWIZARD_PAGE_ID_PROBE_EXECUTABLE:
			labelFileIsExecutableResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelIdentifiedAsMameResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelVersionSupportedResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelTotalMachinesResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelBinaryIdentHashResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelFileModificationDateResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelEmulatorIniFileLocationResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			labelFrontendIniFileLocationResult->setText("<font size=\"4\">" + tr("Check result pending...") + "</font>");
			QTimer::singleShot(0, this, SLOT(probeExecutable()));
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORT_INI_FILES:
			radioButtonImportMameIni->setVisible(!m_emulatorIniPath.isEmpty());
			m_labelImportMameIni->setVisible(!m_emulatorIniPath.isEmpty());
			m_labelImportMameIni->setText("<font size=\"4\">" + tr("Import emulator settings from %1").arg(m_emulatorIniPath) + "</font>");
			toolButtonBrowseMameIni->setVisible(!m_emulatorIniPath.isEmpty());
			radioButtonImportUiIni->setVisible(!m_frontendIniPath.isEmpty());
			m_labelImportUiIni->setVisible(!m_frontendIniPath.isEmpty());
			m_labelImportUiIni->setText("<font size=\"4\">" + tr("Import front-end settings from %1").arg(m_frontendIniPath) + "</font>");
			toolButtonBrowseUiIni->setVisible(!m_frontendIniPath.isEmpty());
			radioButtonImportBothInis->setVisible(!m_emulatorIniPath.isEmpty() && !m_frontendIniPath.isEmpty());
			m_labelImportBothInis->setVisible(!m_emulatorIniPath.isEmpty() && !m_frontendIniPath.isEmpty());
			m_labelImportBothInis->setText("<font size=\"4\">" + tr("Import both emulator and front-end settings") + "</font>");
			m_labelImportNothing->setText("<font size=\"4\">" + tr("Import nothing") + "</font>");
			radioButtonImportNothing->setChecked(true);
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_IMPORTING_INI_FILES:
			plainTextEdit->clear();
			if ( radioButtonImportBothInis->isChecked() || radioButtonImportMameIni->isChecked() )
				QTimer::singleShot(0, this, SLOT(importMameIni()));
			else if ( radioButtonImportUiIni->isChecked() )
				QTimer::singleShot(0, this, SLOT(importUiIni()));
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_ADJUST_SETTINGS: {
				QString wd(m_customSettings->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());
				if ( wd.isEmpty() ) // default to emulator executable's path
					wd = QFileInfo(comboBoxExecutableFile->currentText()).absolutePath();
				lineEditWorkingDirectory->setText(wd);
				lineEditROMPath->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath", QString()).toString());
				lineEditSamplePath->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath", QString()).toString());
				lineEditHashPath->setText(m_customSettings->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", QString()).toString());
			}
			break;
		case QMC2_SETUPWIZARD_PAGE_ID_SETUP_COMPLETE: {
				if ( comboBoxStyle->currentText() == tr("Default") )
					m_customSettings->remove(QMC2_FRONTEND_PREFIX + "GUI/Style");
				else
					m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Style", comboBoxStyle->currentText());
				m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", comboBoxLanguage->currentText());
				m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", comboBoxExecutableFile->currentText());
				QStringList emuHistory;
				for (int i = 0; i < comboBoxExecutableFile->count(); i++)
					emuHistory << comboBoxExecutableFile->itemText(i);
				if ( !emuHistory.contains(comboBoxExecutableFile->currentText()) )
					emuHistory << comboBoxExecutableFile->currentText();
				if ( emuHistory.isEmpty() )
					m_customSettings->remove(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory");
				else {
					emuHistory.sort();
					m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "Welcome/EmuHistory", emuHistory);
				}
				if ( !lineEditWorkingDirectory->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory");
				if ( !lineEditROMPath->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath", lineEditROMPath->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath");
				if ( !lineEditSamplePath->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath", lineEditSamplePath->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath");
				if ( !lineEditHashPath->text().isEmpty() )
					m_customSettings->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath", lineEditHashPath->text());
				else
					m_customSettings->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath");
			}
			break;
	}
}

bool SetupWizard::findIniFiles()
{
	m_emulatorIniPath.clear();
	m_frontendIniPath.clear();
	if ( m_emuConfigName.isEmpty() )
		return false;
	QProcess commandProc;
	bool started = false, commandProcStarted = false;
	int retries = 0;
	commandProc.start(comboBoxExecutableFile->currentText(), QStringList() << "-showconfig");
	started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
	while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
		qApp->processEvents();
		started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
	}
	if ( started ) {
		commandProcStarted = true;
		bool commandProcRunning = (commandProc.state() == QProcess::Running);
		while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
			qApp->processEvents();
			commandProcRunning = (commandProc.state() == QProcess::Running);
		}
		if ( commandProcStarted ) {
			QString output(commandProc.readAllStandardOutput());
#if defined(QMC2_OS_WIN)
			output.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
			bool result = true;
			QString iniPath;
			foreach (QString configLine, output.split('\n')) {
				configLine = configLine.trimmed();
				if ( configLine.startsWith("inipath") ) {
					iniPath = configLine.mid(8).trimmed();
					if ( iniPath.isEmpty() )
						result = false;
					break;
				}
			}
			if ( result ) {
				QStringList iniPaths(iniPath.split(';', QString::SkipEmptyParts));
				QFileInfo fiExec(comboBoxExecutableFile->currentText());
				foreach (QString path, iniPaths) {
					if ( path == "." )
						path = fiExec.absolutePath();
					else
						path = path.replace("$HOME", QDir::homePath());
					QFileInfo fi(path + "/" + m_emuConfigName + ".ini");
					if ( fi.exists() && fi.isReadable() ) {
						m_emulatorIniPath = fi.absoluteFilePath();
						break;
					}
				}
				foreach (QString path, iniPaths) {
					if ( path == "." )
						path = fiExec.absolutePath();
					else
						path = path.replace("$HOME", QDir::homePath());
					QFileInfo fi(path + "/ui.ini");
					if ( fi.exists() && fi.isReadable() ) {
						m_frontendIniPath = fi.absoluteFilePath();
						break;
					}
				}
			}
			return result;
		} else
			return false;
	} else
		return false;
}

void SetupWizard::comboBoxExecutableFile_textChanged(const QString &text)
{
	QFileInfo fi(text);
	button(QWizard::NextButton)->setEnabled(fi.exists() && fi.isFile());
}

void SetupWizard::on_toolButtonBrowseExecutableFile_clicked()
{
	QString s;
	if ( comboBoxExecutableFile->lineEdit()->text().isEmpty() )
		s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), QString(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	else {
		QFileInfo fileInfo(comboBoxExecutableFile->lineEdit()->text());
		s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), fileInfo.absoluteFilePath(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	}
	if ( !s.isNull() )
		comboBoxExecutableFile->lineEdit()->setText(s);
}

void SetupWizard::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditWorkingDirectory->setText(s);
}

void SetupWizard::on_toolButtonBrowseROMPath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditROMPath->setText(s);
}

void SetupWizard::on_toolButtonBrowseSamplePath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditSamplePath->setText(s);
}

void SetupWizard::on_toolButtonBrowseHashPath_clicked()
{
	QString s(QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
	if ( !s.isNull() )
		lineEditHashPath->setText(s);
}

void SetupWizard::on_toolButtonBrowseMameIni_clicked()
{
	QString s(QFileDialog::getOpenFileName(this, tr("Choose mame.ini"), m_emulatorIniPath, tr("Ini files (*.ini)") + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		m_emulatorIniPath = s;
	m_labelImportMameIni->setText("<font size=\"4\">" + tr("Import emulator settings from %1").arg(m_emulatorIniPath) + "</font>");
}

void SetupWizard::on_toolButtonBrowseUiIni_clicked()
{
	QString s(QFileDialog::getOpenFileName(this, tr("Choose ui.ini"), m_frontendIniPath, tr("Ini files (*.ini)") + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		m_frontendIniPath = s;
	m_labelImportUiIni->setText("<font size=\"4\">" + tr("Import front-end settings from %1").arg(m_frontendIniPath) + "</font>");
}

void SetupWizard::on_comboBoxLanguage_currentIndexChanged(int index)
{
	m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", m_availableLanguages.at(index));
	setupLanguage();
	retranslateUi(this);
	setButtonText(QWizard::NextButton, tr("&Next >"));
	setButtonText(QWizard::BackButton, tr("< &Back"));
	setButtonText(QWizard::CancelButton, tr("&Cancel"));
	setButtonText(QWizard::FinishButton, tr("&Finish"));
}

void SetupWizard::setupLanguage()
{
	QString lang(m_customSettings->value(QMC2_FRONTEND_PREFIX + "GUI/Language", QString()).toString());
	if ( lang.isEmpty() || !m_availableLanguages.contains(lang) ) {
		// try to use default system locale - use "us" if a translation is not available for the system locale
		switch ( QLocale::system().language() ) {
			case QLocale::German:
				lang = "de";
				break;
			case QLocale::Spanish:
				lang = "es";
				break;
			case QLocale::French:
				lang = "fr";
				break;
			case QLocale::Greek:
				lang = "el";
				break;
			case QLocale::Italian:
				lang = "it";
				break;
			case QLocale::Polish:
				lang = "pl";
				break;
			case QLocale::Portuguese:
				lang = "pt";
				break;
			case QLocale::Romanian:
				lang = "ro";
				break;
			case QLocale::Swedish:
				lang = "sv";
				break;
			default:
				lang = "us";
				break;
		}
		m_customSettings->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", lang);
	}
	if ( qmc2QtTranslator ) {
		qApp->removeTranslator(qmc2QtTranslator);
		delete qmc2QtTranslator;
	}
	qmc2QtTranslator = new QTranslator(0);
	qmc2QtTranslator->load(QString(":/data/lng/qt_%1.qm").arg(lang));
	qApp->installTranslator(qmc2QtTranslator);
	if ( qmc2Translator ) {
		qApp->removeTranslator(qmc2Translator);
		delete qmc2Translator;
	}
	qmc2Translator = new QTranslator(0);
	qmc2Translator->load(QString(":/data/lng/qmc2_%1.qm").arg(lang));
	qApp->installTranslator(qmc2Translator);

	// we need to "retranslate" the style list due to "Default"
	comboBoxStyle->blockSignals(true);
	int styleIndex = comboBoxStyle->currentIndex();
	comboBoxStyle->clear();
	comboBoxStyle->addItem(QObject::tr("Default"));
	comboBoxStyle->addItems(QStyleFactory::keys());
	comboBoxStyle->setCurrentIndex(styleIndex);
	comboBoxStyle->blockSignals(false);
}

void SetupWizard::setupStyle(const QString &styleName)
{
	if ( styleName == QObject::tr("Default") )
		qApp->setStyle(QStyleFactory::create(m_defaultStyle));
	else
		qApp->setStyle(QStyleFactory::create(styleName));
}
