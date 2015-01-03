#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QFileDialog>
#include <QInputDialog>
#endif

#include "detailsetup.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;

DetailSetup::DetailSetup(QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DetailSetup::DetailSetup(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

#if defined(QMC2_EMUTYPE_MAME)
	shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Pre&view");
	longTitleMap[QMC2_PREVIEW_INDEX] = tr("Game preview image");
	iconMap[QMC2_PREVIEW_INDEX] = QIcon(QString::fromUtf8(":/data/img/camera.png"));
	shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
	longTitleMap[QMC2_FLYER_INDEX] = tr("Game flyer image");
	iconMap[QMC2_FLYER_INDEX] = QIcon(QString::fromUtf8(":/data/img/thumbnail.png"));
	shortTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game &info");
	longTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game information");
	iconMap[QMC2_GAMEINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
	shortTitleMap[QMC2_EMUINFO_INDEX] = tr("Em&ulator info");
	longTitleMap[QMC2_EMUINFO_INDEX] = tr("Emulator information");
	iconMap[QMC2_EMUINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
	shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
	longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
	iconMap[QMC2_CONFIG_INDEX] = QIcon(QString::fromUtf8(":/data/img/work.png"));
	shortTitleMap[QMC2_CABINET_INDEX] = tr("Ca&binet");
	longTitleMap[QMC2_CABINET_INDEX] = tr("Arcade cabinet image");
	iconMap[QMC2_CABINET_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png"));
	shortTitleMap[QMC2_CONTROLLER_INDEX] = tr("C&ontroller");
	longTitleMap[QMC2_CONTROLLER_INDEX] = tr("Control panel image");
	iconMap[QMC2_CONTROLLER_INDEX] = QIcon(QString::fromUtf8(":/data/img/joystick.png"));
	shortTitleMap[QMC2_MARQUEE_INDEX] = tr("Mar&quee");
	longTitleMap[QMC2_MARQUEE_INDEX] = tr("Marquee image");
	iconMap[QMC2_MARQUEE_INDEX] = QIcon(QString::fromUtf8(":/data/img/marquee.png"));
	shortTitleMap[QMC2_TITLE_INDEX] = tr("Titl&e");
	longTitleMap[QMC2_TITLE_INDEX] = tr("Title screen image");
	iconMap[QMC2_TITLE_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcademode.png"));
	shortTitleMap[QMC2_MAWS_INDEX] = tr("MA&WS");
	longTitleMap[QMC2_MAWS_INDEX] = tr("MAWS page (web lookup)");
	iconMap[QMC2_MAWS_INDEX] = QIcon(QString::fromUtf8(":/data/img/internet.png"));
	shortTitleMap[QMC2_PCB_INDEX] = tr("&PCB");
	longTitleMap[QMC2_PCB_INDEX] = tr("PCB image");
	iconMap[QMC2_PCB_INDEX] = QIcon(QString::fromUtf8(":/data/img/circuit.png"));
	shortTitleMap[QMC2_SOFTWARE_LIST_INDEX] = tr("Softwar&e list");
	longTitleMap[QMC2_SOFTWARE_LIST_INDEX] = tr("Software list");
	iconMap[QMC2_SOFTWARE_LIST_INDEX] = QIcon(QString::fromUtf8(":/data/img/pacman.png"));
#if QMC2_YOUTUBE_ENABLED
	shortTitleMap[QMC2_YOUTUBE_INDEX] = tr("&YouTube");
	longTitleMap[QMC2_YOUTUBE_INDEX] = tr("YouTube videos");
	iconMap[QMC2_YOUTUBE_INDEX] = QIcon(QString::fromUtf8(":/data/img/youtube.png"));
#endif
	shortTitleMap[QMC2_SYSTEM_NOTES_INDEX] = tr("&Notes");
	longTitleMap[QMC2_SYSTEM_NOTES_INDEX] = tr("System notes");
	iconMap[QMC2_SYSTEM_NOTES_INDEX] = QIcon(QString::fromUtf8(":/data/img/notes.png"));
	availableDetailList << QMC2_PREVIEW_INDEX
			<< QMC2_FLYER_INDEX
			<< QMC2_GAMEINFO_INDEX
			<< QMC2_EMUINFO_INDEX
			<< QMC2_CONFIG_INDEX
			<< QMC2_CABINET_INDEX
			<< QMC2_CONTROLLER_INDEX
			<< QMC2_MARQUEE_INDEX
			<< QMC2_TITLE_INDEX
			<< QMC2_MAWS_INDEX
			<< QMC2_PCB_INDEX
			<< QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
	availableDetailList << QMC2_YOUTUBE_INDEX
			<< QMC2_SYSTEM_NOTES_INDEX;
#else
	availableDetailList << QMC2_SYSTEM_NOTES_INDEX;
#endif
	tabWidgetsMap[QMC2_PREVIEW_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX);
	tabWidgetsMap[QMC2_FLYER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX);
	tabWidgetsMap[QMC2_GAMEINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_GAMEINFO_INDEX);
	tabWidgetsMap[QMC2_EMUINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX);
	tabWidgetsMap[QMC2_CONFIG_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX);
	tabWidgetsMap[QMC2_CABINET_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX);
	tabWidgetsMap[QMC2_CONTROLLER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX);
	tabWidgetsMap[QMC2_MARQUEE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX);
	tabWidgetsMap[QMC2_TITLE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_TITLE_INDEX);
	tabWidgetsMap[QMC2_MAWS_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MAWS_INDEX);
	tabWidgetsMap[QMC2_PCB_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PCB_INDEX);
	tabWidgetsMap[QMC2_SOFTWARE_LIST_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SOFTWARE_LIST_INDEX);
#if QMC2_YOUTUBE_ENABLED
	tabWidgetsMap[QMC2_YOUTUBE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_YOUTUBE_INDEX);
#endif
	tabWidgetsMap[QMC2_SYSTEM_NOTES_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SYSTEM_NOTES_INDEX);
	configurableDetailList << QMC2_MAWS_INDEX;
#if QMC2_YOUTUBE_ENABLED
	configurableDetailList << QMC2_YOUTUBE_INDEX;
#endif

#elif defined(QMC2_EMUTYPE_MESS)
	shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Pre&view");
	longTitleMap[QMC2_PREVIEW_INDEX] = tr("Machine preview image");
	iconMap[QMC2_PREVIEW_INDEX] = QIcon(QString::fromUtf8(":/data/img/camera.png"));
	shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
	longTitleMap[QMC2_FLYER_INDEX] = tr("Machine flyer image");
	iconMap[QMC2_FLYER_INDEX] = QIcon(QString::fromUtf8(":/data/img/thumbnail.png"));
	shortTitleMap[QMC2_MACHINEINFO_INDEX] = tr("Machine &info");
	longTitleMap[QMC2_MACHINEINFO_INDEX] = tr("Machine information");
	iconMap[QMC2_MACHINEINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
	shortTitleMap[QMC2_EMUINFO_INDEX] = tr("Em&ulator info");
	longTitleMap[QMC2_EMUINFO_INDEX] = tr("Emulator information");
	iconMap[QMC2_EMUINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
	shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
	longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
	iconMap[QMC2_CONFIG_INDEX] = QIcon(QString::fromUtf8(":/data/img/work.png"));
	shortTitleMap[QMC2_DEVICE_INDEX] = tr("De&vices");
	longTitleMap[QMC2_DEVICE_INDEX] = tr("Device configuration");
	iconMap[QMC2_DEVICE_INDEX] = QIcon(QString::fromUtf8(":/data/img/tape.png"));
	shortTitleMap[QMC2_PROJECTMESS_INDEX] = tr("Pr&ojectMESS");
	longTitleMap[QMC2_PROJECTMESS_INDEX] = tr("ProjectMESS (web lookup)");
	iconMap[QMC2_PROJECTMESS_INDEX] = QIcon(QString::fromUtf8(":/data/img/project_mess.png"));
	shortTitleMap[QMC2_PCB_INDEX] = tr("&PCB");
	longTitleMap[QMC2_PCB_INDEX] = tr("PCB image");
	iconMap[QMC2_PCB_INDEX] = QIcon(QString::fromUtf8(":/data/img/circuit.png"));
	shortTitleMap[QMC2_CABINET_INDEX] = tr("Ca&binet");
	longTitleMap[QMC2_CABINET_INDEX] = tr("Machine cabinet image");
	iconMap[QMC2_CABINET_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png"));
	shortTitleMap[QMC2_CONTROLLER_INDEX] = tr("C&ontroller");
	longTitleMap[QMC2_CONTROLLER_INDEX] = tr("Control panel image");
	iconMap[QMC2_CONTROLLER_INDEX] = QIcon(QString::fromUtf8(":/data/img/joystick.png"));
	shortTitleMap[QMC2_MARQUEE_INDEX] = tr("Lo&go");
	longTitleMap[QMC2_MARQUEE_INDEX] = tr("Logo image");
	iconMap[QMC2_MARQUEE_INDEX] = QIcon(QString::fromUtf8(":/data/img/marquee.png"));
	shortTitleMap[QMC2_SOFTWARE_LIST_INDEX] = tr("Softwar&e list");
	longTitleMap[QMC2_SOFTWARE_LIST_INDEX] = tr("Software list");
	iconMap[QMC2_SOFTWARE_LIST_INDEX] = QIcon(QString::fromUtf8(":/data/img/pacman.png"));
#if QMC2_YOUTUBE_ENABLED
	shortTitleMap[QMC2_YOUTUBE_INDEX] = tr("&YouTube");
	longTitleMap[QMC2_YOUTUBE_INDEX] = tr("YouTube videos");
	iconMap[QMC2_YOUTUBE_INDEX] = QIcon(QString::fromUtf8(":/data/img/youtube.png"));
#endif
	shortTitleMap[QMC2_SYSTEM_NOTES_INDEX] = tr("&Notes");
	longTitleMap[QMC2_SYSTEM_NOTES_INDEX] = tr("System notes");
	iconMap[QMC2_SYSTEM_NOTES_INDEX] = QIcon(QString::fromUtf8(":/data/img/notes.png"));
	availableDetailList << QMC2_PREVIEW_INDEX
			<< QMC2_FLYER_INDEX
			<< QMC2_MACHINEINFO_INDEX
			<< QMC2_EMUINFO_INDEX
			<< QMC2_CONFIG_INDEX
			<< QMC2_DEVICE_INDEX
			<< QMC2_PROJECTMESS_INDEX
			<< QMC2_PCB_INDEX
			<< QMC2_CABINET_INDEX
			<< QMC2_CONTROLLER_INDEX
			<< QMC2_MARQUEE_INDEX
			<< QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
	availableDetailList << QMC2_YOUTUBE_INDEX
			<< QMC2_SYSTEM_NOTES_INDEX;
#else
	availableDetailList << QMC2_SYSTEM_NOTES_INDEX;
#endif
	tabWidgetsMap[QMC2_PREVIEW_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX);
	tabWidgetsMap[QMC2_FLYER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX);
	tabWidgetsMap[QMC2_MACHINEINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MACHINEINFO_INDEX);
	tabWidgetsMap[QMC2_EMUINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX);
	tabWidgetsMap[QMC2_CONFIG_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX);
	tabWidgetsMap[QMC2_DEVICE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_DEVICE_INDEX);
	tabWidgetsMap[QMC2_PROJECTMESS_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PROJECTMESS_INDEX);
	tabWidgetsMap[QMC2_PCB_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PCB_INDEX);
	tabWidgetsMap[QMC2_CABINET_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX);
	tabWidgetsMap[QMC2_CONTROLLER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX);
	tabWidgetsMap[QMC2_MARQUEE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX);
	tabWidgetsMap[QMC2_SOFTWARE_LIST_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SOFTWARE_LIST_INDEX);
#if QMC2_YOUTUBE_ENABLED
	tabWidgetsMap[QMC2_YOUTUBE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_YOUTUBE_INDEX);
	configurableDetailList << QMC2_YOUTUBE_INDEX;
#endif
	tabWidgetsMap[QMC2_SYSTEM_NOTES_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SYSTEM_NOTES_INDEX);

#elif defined(QMC2_EMUTYPE_UME)
	shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Pre&view");
	longTitleMap[QMC2_PREVIEW_INDEX] = tr("Game preview image");
	iconMap[QMC2_PREVIEW_INDEX] = QIcon(QString::fromUtf8(":/data/img/camera.png"));
	shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
	longTitleMap[QMC2_FLYER_INDEX] = tr("Game flyer image");
	iconMap[QMC2_FLYER_INDEX] = QIcon(QString::fromUtf8(":/data/img/thumbnail.png"));
	shortTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game &info");
	longTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game information");
	iconMap[QMC2_GAMEINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
	shortTitleMap[QMC2_EMUINFO_INDEX] = tr("Em&ulator info");
	longTitleMap[QMC2_EMUINFO_INDEX] = tr("Emulator information");
	iconMap[QMC2_EMUINFO_INDEX] = QIcon(QString::fromUtf8(":/data/img/info.png"));
	shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
	longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
	iconMap[QMC2_CONFIG_INDEX] = QIcon(QString::fromUtf8(":/data/img/work.png"));
	shortTitleMap[QMC2_DEVICE_INDEX] = tr("De&vices");
	longTitleMap[QMC2_DEVICE_INDEX] = tr("Device configuration");
	iconMap[QMC2_DEVICE_INDEX] = QIcon(QString::fromUtf8(":/data/img/tape.png"));
	shortTitleMap[QMC2_PROJECTMESS_INDEX] = tr("Pr&ojectMESS");
	longTitleMap[QMC2_PROJECTMESS_INDEX] = tr("ProjectMESS (web lookup)");
	iconMap[QMC2_PROJECTMESS_INDEX] = QIcon(QString::fromUtf8(":/data/img/project_mess.png"));
	shortTitleMap[QMC2_CABINET_INDEX] = tr("Ca&binet");
	longTitleMap[QMC2_CABINET_INDEX] = tr("Arcade cabinet image");
	iconMap[QMC2_CABINET_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png"));
	shortTitleMap[QMC2_CONTROLLER_INDEX] = tr("C&ontroller");
	longTitleMap[QMC2_CONTROLLER_INDEX] = tr("Control panel image");
	iconMap[QMC2_CONTROLLER_INDEX] = QIcon(QString::fromUtf8(":/data/img/joystick.png"));
	shortTitleMap[QMC2_MARQUEE_INDEX] = tr("Mar&quee");
	longTitleMap[QMC2_MARQUEE_INDEX] = tr("Marquee image");
	iconMap[QMC2_MARQUEE_INDEX] = QIcon(QString::fromUtf8(":/data/img/marquee.png"));
	shortTitleMap[QMC2_TITLE_INDEX] = tr("Titl&e");
	longTitleMap[QMC2_TITLE_INDEX] = tr("Title screen image");
	iconMap[QMC2_TITLE_INDEX] = QIcon(QString::fromUtf8(":/data/img/arcademode.png"));
	shortTitleMap[QMC2_PCB_INDEX] = tr("&PCB");
	longTitleMap[QMC2_PCB_INDEX] = tr("PCB image");
	iconMap[QMC2_PCB_INDEX] = QIcon(QString::fromUtf8(":/data/img/circuit.png"));
	shortTitleMap[QMC2_SOFTWARE_LIST_INDEX] = tr("Softwar&e list");
	longTitleMap[QMC2_SOFTWARE_LIST_INDEX] = tr("Software list");
	iconMap[QMC2_SOFTWARE_LIST_INDEX] = QIcon(QString::fromUtf8(":/data/img/pacman.png"));
#if QMC2_YOUTUBE_ENABLED
	shortTitleMap[QMC2_YOUTUBE_INDEX] = tr("&YouTube");
	longTitleMap[QMC2_YOUTUBE_INDEX] = tr("YouTube videos");
	iconMap[QMC2_YOUTUBE_INDEX] = QIcon(QString::fromUtf8(":/data/img/youtube.png"));
#endif
	shortTitleMap[QMC2_SYSTEM_NOTES_INDEX] = tr("&Notes");
	longTitleMap[QMC2_SYSTEM_NOTES_INDEX] = tr("System notes");
	iconMap[QMC2_SYSTEM_NOTES_INDEX] = QIcon(QString::fromUtf8(":/data/img/notes.png"));
	availableDetailList << QMC2_PREVIEW_INDEX
			<< QMC2_FLYER_INDEX
			<< QMC2_GAMEINFO_INDEX
			<< QMC2_EMUINFO_INDEX
			<< QMC2_CONFIG_INDEX
			<< QMC2_DEVICE_INDEX
			<< QMC2_PROJECTMESS_INDEX
			<< QMC2_CABINET_INDEX
			<< QMC2_CONTROLLER_INDEX
			<< QMC2_MARQUEE_INDEX
			<< QMC2_TITLE_INDEX
			<< QMC2_PCB_INDEX
			<< QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
	availableDetailList << QMC2_YOUTUBE_INDEX
			<< QMC2_SYSTEM_NOTES_INDEX;
#else
	availableDetailList << QMC2_SYSTEM_NOTES_INDEX;
#endif
	tabWidgetsMap[QMC2_PREVIEW_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX);
	tabWidgetsMap[QMC2_FLYER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX);
	tabWidgetsMap[QMC2_GAMEINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_GAMEINFO_INDEX);
	tabWidgetsMap[QMC2_EMUINFO_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX);
	tabWidgetsMap[QMC2_CONFIG_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX);
	tabWidgetsMap[QMC2_DEVICE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_DEVICE_INDEX);
	tabWidgetsMap[QMC2_PROJECTMESS_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PROJECTMESS_INDEX);
	tabWidgetsMap[QMC2_CABINET_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX);
	tabWidgetsMap[QMC2_CONTROLLER_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX);
	tabWidgetsMap[QMC2_MARQUEE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX);
	tabWidgetsMap[QMC2_TITLE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_TITLE_INDEX);
	tabWidgetsMap[QMC2_PCB_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PCB_INDEX);
	tabWidgetsMap[QMC2_SOFTWARE_LIST_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SOFTWARE_LIST_INDEX);
#if QMC2_YOUTUBE_ENABLED
	tabWidgetsMap[QMC2_YOUTUBE_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_YOUTUBE_INDEX);
	configurableDetailList << QMC2_YOUTUBE_INDEX;
#endif
	tabWidgetsMap[QMC2_SYSTEM_NOTES_INDEX] = qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SYSTEM_NOTES_INDEX);
#endif

	setupUi(this);
	hide();

	QMapIterator<int, QString> it(longTitleMap);
	while ( it.hasNext() ) {
		it.next();
		listWidgetAvailableDetails->addItem(new QListWidgetItem(iconMap[it.key()], it.value()));
	}

	loadDetail();
	adjustSize();
}

DetailSetup::~DetailSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::~DetailSetup()");
#endif

}

void DetailSetup::loadDetail()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::loadDetail()");
#endif

	activeDetailList.clear();
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ActiveDetails") ) {
		// use default detail list
#if defined(QMC2_EMUTYPE_MAME)
		activeDetailList << QMC2_PREVIEW_INDEX
				<< QMC2_FLYER_INDEX
				<< QMC2_GAMEINFO_INDEX
				<< QMC2_EMUINFO_INDEX
				<< QMC2_CONFIG_INDEX
				<< QMC2_CABINET_INDEX
				<< QMC2_CONTROLLER_INDEX
				<< QMC2_MARQUEE_INDEX
				<< QMC2_TITLE_INDEX
				<< QMC2_MAWS_INDEX
				<< QMC2_PCB_INDEX
				<< QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
		activeDetailList << QMC2_YOUTUBE_INDEX;
#endif
		activeDetailList << QMC2_SYSTEM_NOTES_INDEX;
#elif defined(QMC2_EMUTYPE_MESS)
		activeDetailList << QMC2_PREVIEW_INDEX
				<< QMC2_FLYER_INDEX
				<< QMC2_MACHINEINFO_INDEX
				<< QMC2_EMUINFO_INDEX
				<< QMC2_CONFIG_INDEX
				<< QMC2_DEVICE_INDEX
				<< QMC2_PROJECTMESS_INDEX
				<< QMC2_CABINET_INDEX
				<< QMC2_CONTROLLER_INDEX
				<< QMC2_LOGO_INDEX
				<< QMC2_PCB_INDEX
				<< QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
		activeDetailList << QMC2_YOUTUBE_INDEX;
#endif
		activeDetailList << QMC2_SYSTEM_NOTES_INDEX;
#elif defined(QMC2_EMUTYPE_UME)
		activeDetailList << QMC2_PREVIEW_INDEX
				<< QMC2_FLYER_INDEX
				<< QMC2_GAMEINFO_INDEX
				<< QMC2_EMUINFO_INDEX
				<< QMC2_CONFIG_INDEX
				<< QMC2_DEVICE_INDEX
				<< QMC2_PROJECTMESS_INDEX
				<< QMC2_CABINET_INDEX
				<< QMC2_CONTROLLER_INDEX
				<< QMC2_MARQUEE_INDEX
				<< QMC2_TITLE_INDEX
				<< QMC2_PCB_INDEX
				<< QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
		activeDetailList << QMC2_YOUTUBE_INDEX;
#endif
		activeDetailList << QMC2_SYSTEM_NOTES_INDEX;
#endif
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ActiveDetails").toStringList();
		foreach (QString s, activeIndexList) {
			int i = s.toInt();
			if ( availableDetailList.contains(i) )
				activeDetailList << i;
		}
	}

	appliedDetailList = activeDetailList;

	listWidgetActiveDetails->clear();
	foreach (int i, activeDetailList)
		listWidgetActiveDetails->addItem(new QListWidgetItem(iconMap[i], longTitleMap[i]));
}

void DetailSetup::saveDetail()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::saveDetail()");
#endif

	int oldIndex = appliedDetailList[qmc2MainWindow->tabWidgetGameDetail->currentIndex()];

	qmc2MainWindow->tabWidgetGameDetail->clear();

	appliedDetailList.clear();

	QStringList activeIndexList;
	foreach (int i, activeDetailList)
		if ( availableDetailList.contains(i) ) {
			appliedDetailList << i;
			activeIndexList << QString::number(i);
			qmc2MainWindow->tabWidgetGameDetail->addTab(tabWidgetsMap[i], iconMap[i], shortTitleMap[i]);
		}

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ActiveDetails", activeIndexList);

	if ( appliedDetailList.contains(oldIndex) )
		qmc2MainWindow->tabWidgetGameDetail->setCurrentIndex(appliedDetailList.indexOf(oldIndex));
}

void DetailSetup::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::adjustIconSizes()");
#endif

	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	pushButtonConfigureDetail->setIconSize(iconSize);
	pushButtonActivateDetails->setIconSize(iconSize);
	pushButtonDeactivateDetails->setIconSize(iconSize);
	pushButtonDetailsUp->setIconSize(iconSize);
	pushButtonDetailsDown->setIconSize(iconSize);
	pushButtonOk->setIconSize(iconSize);
	pushButtonApply->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	listWidgetAvailableDetails->setIconSize(iconSize);
	listWidgetActiveDetails->setIconSize(iconSize);
}

void DetailSetup::on_listWidgetAvailableDetails_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_listWidgetAvailableDetails_itemSelectionChanged()");
#endif

	if ( listWidgetAvailableDetails->selectedItems().count() > 0 ) {
		pushButtonActivateDetails->setEnabled(true);
		if ( listWidgetAvailableDetails->selectedItems().count() == 1 ) {
			if ( configurableDetailList.contains(longTitleMap.key(listWidgetAvailableDetails->selectedItems()[0]->text())) )
				pushButtonConfigureDetail->setEnabled(true);
			else
				pushButtonConfigureDetail->setEnabled(false);
		} else
			pushButtonConfigureDetail->setEnabled(false);
	} else
		pushButtonActivateDetails->setEnabled(false);
}
 
void DetailSetup::on_listWidgetActiveDetails_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_listWidgetActiveDetails_itemSelectionChanged()");
#endif

	if ( listWidgetActiveDetails->selectedItems().count() > 0 ) {
		pushButtonDeactivateDetails->setEnabled(true);
		if ( listWidgetActiveDetails->selectedItems().count() == 1 ) {
			pushButtonDetailsUp->setEnabled(true);
			pushButtonDetailsDown->setEnabled(true);
		} else {
			pushButtonDetailsUp->setEnabled(false);
			pushButtonDetailsDown->setEnabled(false);
		}
	} else {
		pushButtonDeactivateDetails->setEnabled(false);
		pushButtonDetailsUp->setEnabled(false);
		pushButtonDetailsDown->setEnabled(false);
	}
}

void DetailSetup::on_pushButtonActivateDetails_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonActivateDetails_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetAvailableDetails->selectedItems()) {
		if ( item ) {
			QList<QListWidgetItem *> il = listWidgetActiveDetails->findItems(item->text(), Qt::MatchExactly); 
			if ( il.count() == 0 ) {
				int pageIndex = longTitleMap.key(item->text());
				if ( availableDetailList.contains(pageIndex) ) {
					listWidgetActiveDetails->addItem(new QListWidgetItem(iconMap[pageIndex], item->text()));
					activeDetailList << pageIndex;
				}
			}
		}
	}
}

void DetailSetup::on_pushButtonConfigureDetail_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonConfigureDetail_clicked()");
#endif

	if ( listWidgetAvailableDetails->selectedItems().count() == 1 ) {
		int pageIndex = longTitleMap.key(listWidgetAvailableDetails->selectedItems()[0]->text());
		if ( configurableDetailList.contains(pageIndex) ) {
			switch ( pageIndex ) {
#if defined(QMC2_EMUTYPE_MAME)
				case QMC2_MAWS_INDEX: {
					bool ok;
					QStringList suggestedUrls;
					QString currentUrl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_DEFAULT_URL).toString();
					suggestedUrls << QMC2_MAWS_DEFAULT_URL
						<< "http://www.arcadehits.net/index.php?p=roms&jeu=%1"
						<< "http://www.mamedb.com/game/%1"
						<< QMC2_MAWS_BASE_URL
						<< "http://maws.mameworld.info/minimaws/romset/%1";
					if ( !suggestedUrls.contains(currentUrl) ) suggestedUrls << currentUrl;
					int current = suggestedUrls.indexOf(currentUrl);
					QString baseUrl = QInputDialog::getItem(this,
								tr("MAWS configuration (1/2)"),
								tr("MAWS URL pattern (use %1 as placeholder for game ID):").arg("%1"),
								suggestedUrls,
								current < 0 ? 0 : current,
								true,
								&ok);

					if ( ok ) {
						if ( baseUrl == QMC2_MAWS_BASE_URL ) {
							QStringList items;
							items << tr("Yes") << tr("No");
							bool mawsQuickDownloadEnabled = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/QuickDownload", false).toBool();
							QString mawsQuickDownload = QInputDialog::getItem(this, tr("MAWS configuration (2/2)"), tr("Enable MAWS quick download?"), items, mawsQuickDownloadEnabled ? 0 : 1, false, &ok);
							if ( ok && !mawsQuickDownload.isEmpty() )
								qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/QuickDownload", mawsQuickDownload == tr("Yes"));
						} else
							qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/QuickDownload", false);

						if ( ok && !baseUrl.isEmpty() ) {
							qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", baseUrl);
							if ( currentUrl != qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_DEFAULT_URL).toString() )
								QTimer::singleShot(0, qmc2MainWindow->actionClearMAWSCache, SLOT(trigger()));
						}
					}
				}
				break;
#endif

#if QMC2_YOUTUBE_ENABLED
				case QMC2_YOUTUBE_INDEX: {
					QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
					QString oldCacheDirectory = qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", userScopePath + "/youtube/").toString();
					QString youTubeCacheDirectory = QFileDialog::getExistingDirectory(this, tr("Choose the YouTube cache directory"), oldCacheDirectory, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
					if ( !youTubeCacheDirectory.isNull() ) {
						if ( !youTubeCacheDirectory.endsWith("/") )
							youTubeCacheDirectory += "/";
						if ( youTubeCacheDirectory != oldCacheDirectory ) {
							QDir youTubeCacheDir(youTubeCacheDirectory);
							if ( !youTubeCacheDir.exists() ) {
								if ( youTubeCacheDir.mkdir(youTubeCacheDirectory) )
									qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", youTubeCacheDirectory);
								else
									qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create new YouTube cache directory, path = %1").arg(youTubeCacheDirectory));
							}
						}
					}
				}
#endif

				default:
					break;
			}
		}
	}
}

void DetailSetup::on_pushButtonDeactivateDetails_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonDeactivateDetails_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetActiveDetails->selectedItems()) {
		if ( item ) {
			if ( item->text() != longTitleMap[QMC2_CONFIG_INDEX] ) {
				int row = listWidgetActiveDetails->row(item);
				QListWidgetItem *takenItem = listWidgetActiveDetails->takeItem(row);
				if ( takenItem )
					delete takenItem;
				if ( row >= 0 && row < activeDetailList.count() )
					activeDetailList.removeAt(row);
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFO: the configuration tab can't be removed"));
		}
	}
}

void DetailSetup::on_pushButtonDetailsUp_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonDetailsUp_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetActiveDetails->selectedItems()) {
		if ( item ) {
			int row = listWidgetActiveDetails->row(item);
			if ( row > 0 ) {
				activeDetailList.move(row, row - 1);
				QListWidgetItem *takenItem = listWidgetActiveDetails->takeItem(row);
				if ( takenItem ) {
					listWidgetActiveDetails->insertItem(row - 1, takenItem);
					takenItem->setSelected(true);
				}
			}
		}
	}
}

void DetailSetup::on_pushButtonDetailsDown_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonDetailsDown_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetActiveDetails->selectedItems()) {
		if ( item ) {
			int row = listWidgetActiveDetails->row(item);
			if ( row < listWidgetActiveDetails->count() - 1 ) {
				activeDetailList.move(row, row + 1);
				QListWidgetItem *takenItem = listWidgetActiveDetails->takeItem(row);
				if ( takenItem ) {
					listWidgetActiveDetails->insertItem(row + 1, takenItem);
					takenItem->setSelected(true);
				}
			}
		}
	}
}

void DetailSetup::on_pushButtonOk_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonOk_clicked()");
#endif

	saveDetail();
}

void DetailSetup::on_pushButtonApply_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonApply_clicked()");
#endif

	saveDetail();
}

void DetailSetup::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_pushButtonCancel_clicked()");
#endif

	loadDetail();
}
