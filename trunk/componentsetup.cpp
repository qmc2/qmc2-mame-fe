#include <QtGui>
#include <QFileDialog>
#include <QInputDialog>
#include <QHashIterator>

#include "componentsetup.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;

ComponentSetup::ComponentSetup(QWidget *parent)
	: QDialog(parent)
{
	// init components / hashes
	m_components << "Component1" << "Component2" << "Component3";
	componentInfoHash().insert("Component1", initComponent1());
	componentInfoHash().insert("Component2", initComponent2());
	componentInfoHash().insert("Component3", initComponent3());

	setupUi(this);
	hide();
	adjustSize();

	// FIXME
	comboBoxComponents->setCurrentIndex(1);
	//comboBoxComponents->setEnabled(false);
}

ComponentSetup::~ComponentSetup()
{
	// NOP
}

ComponentInfo *ComponentSetup::initComponent1()
{
	ComponentInfo *componentInfo = new ComponentInfo();
#if defined(QMC2_EMUTYPE_MESS)
	componentInfo->setShortTitle(QMC2_GAMELIST_INDEX, tr("M&achine list"));
	componentInfo->setLongTitle(QMC2_GAMELIST_INDEX, tr("Machine list"));
#else
	componentInfo->setShortTitle(QMC2_GAMELIST_INDEX, tr("&Game list"));
	componentInfo->setLongTitle(QMC2_GAMELIST_INDEX, tr("Game list"));
#endif
	componentInfo->setIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/flat.png")));
	componentInfo->setWidget(QMC2_GAMELIST_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_GAMELIST_INDEX));
	componentInfo->setShortTitle(QMC2_SEARCH_INDEX, tr("&Search"));
	componentInfo->setLongTitle(QMC2_SEARCH_INDEX, tr("Search systems"));
	componentInfo->setIcon(QMC2_SEARCH_INDEX, QIcon(QString::fromUtf8(":/data/img/hint.png")));
	componentInfo->setWidget(QMC2_SEARCH_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_SEARCH_INDEX));
	componentInfo->setShortTitle(QMC2_FAVORITES_INDEX, tr("Favo&rites"));
	componentInfo->setLongTitle(QMC2_FAVORITES_INDEX, tr("Favorite list"));
	componentInfo->setIcon(QMC2_FAVORITES_INDEX, QIcon(QString::fromUtf8(":/data/img/favorites.png")));
	componentInfo->setWidget(QMC2_FAVORITES_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_FAVORITES_INDEX));
	componentInfo->setShortTitle(QMC2_PLAYED_INDEX, tr("Pl&ayed"));
	componentInfo->setLongTitle(QMC2_PLAYED_INDEX, tr("Played list"));
	componentInfo->setIcon(QMC2_PLAYED_INDEX, QIcon(QString::fromUtf8(":/data/img/time.png")));
	componentInfo->setWidget(QMC2_PLAYED_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_PLAYED_INDEX));
	componentInfo->setShortTitle(QMC2_FOREIGN_INDEX, tr("&Foreign emulators"));
	componentInfo->setLongTitle(QMC2_FOREIGN_INDEX, tr("Foreign emulator list"));
	componentInfo->setIcon(QMC2_FOREIGN_INDEX, QIcon(QString::fromUtf8(":/data/img/alien.png")));
	componentInfo->setWidget(QMC2_FOREIGN_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_FOREIGN_INDEX));
	componentInfo->setShortTitle(QMC2_EMBED_INDEX, tr("Embedded emulators"));
	componentInfo->setLongTitle(QMC2_EMBED_INDEX, tr("Embedded emulators"));
	componentInfo->setIcon(QMC2_EMBED_INDEX, QIcon(QString::fromUtf8(":/data/img/embed.png")));
	componentInfo->setWidget(QMC2_EMBED_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_EMBED_INDEX));
	componentInfo->availableFeatureList() << QMC2_GAMELIST_INDEX << QMC2_SEARCH_INDEX << QMC2_FAVORITES_INDEX << QMC2_PLAYED_INDEX << QMC2_FOREIGN_INDEX << QMC2_EMBED_INDEX;
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/Component1/ActiveFeatures") ) {
		foreach (int index, componentInfo->availableFeatureList())
			componentInfo->activeFeatureList() << index;
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Component1/ActiveFeatures").toStringList();
		foreach (QString sIndex, activeIndexList) {
			int index = sIndex.toInt();
			if ( componentInfo->availableFeatureList().contains(index) )
				componentInfo->activeFeatureList() << index;
		}
	}
	m_componentToWidgetHash["Component1"] = qmc2MainWindow->tabWidgetGamelist;
	return componentInfo;
}

ComponentInfo *ComponentSetup::initComponent2()
{
	ComponentInfo *componentInfo = new ComponentInfo();
#if defined(QMC2_EMUTYPE_MAME)
	componentInfo->setShortTitle(QMC2_PREVIEW_INDEX, tr("Pre&view"));
	componentInfo->setLongTitle(QMC2_PREVIEW_INDEX, tr("Game preview image"));
	componentInfo->setIcon(QMC2_PREVIEW_INDEX, QIcon(QString::fromUtf8(":/data/img/camera.png")));
	componentInfo->setShortTitle(QMC2_FLYER_INDEX, tr("Fl&yer"));
	componentInfo->setLongTitle(QMC2_FLYER_INDEX, tr("Game flyer image"));
	componentInfo->setIcon(QMC2_FLYER_INDEX, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")));
	componentInfo->setShortTitle(QMC2_GAMEINFO_INDEX, tr("Game &info"));
	componentInfo->setLongTitle(QMC2_GAMEINFO_INDEX, tr("Game information"));
	componentInfo->setIcon(QMC2_GAMEINFO_INDEX, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setShortTitle(QMC2_EMUINFO_INDEX, tr("Em&ulator info"));
	componentInfo->setLongTitle(QMC2_EMUINFO_INDEX, tr("Emulator information"));
	componentInfo->setIcon(QMC2_EMUINFO_INDEX, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setShortTitle(QMC2_CONFIG_INDEX, tr("&Configuration"));
	componentInfo->setLongTitle(QMC2_CONFIG_INDEX, tr("Emulator configuration"));
	componentInfo->setIcon(QMC2_CONFIG_INDEX, QIcon(QString::fromUtf8(":/data/img/work.png")));
	componentInfo->setRemovable(QMC2_CONFIG_INDEX, false);
	componentInfo->setShortTitle(QMC2_CABINET_INDEX, tr("Ca&binet"));
	componentInfo->setLongTitle(QMC2_CABINET_INDEX, tr("Arcade cabinet image"));
	componentInfo->setIcon(QMC2_CABINET_INDEX, QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")));
	componentInfo->setShortTitle(QMC2_CONTROLLER_INDEX, tr("C&ontroller"));
	componentInfo->setLongTitle(QMC2_CONTROLLER_INDEX, tr("Control panel image"));
	componentInfo->setIcon(QMC2_CONTROLLER_INDEX, QIcon(QString::fromUtf8(":/data/img/joystick.png")));
	componentInfo->setShortTitle(QMC2_MARQUEE_INDEX, tr("Mar&quee"));
	componentInfo->setLongTitle(QMC2_MARQUEE_INDEX, tr("Marquee image"));
	componentInfo->setIcon(QMC2_MARQUEE_INDEX, QIcon(QString::fromUtf8(":/data/img/marquee.png")));
	componentInfo->setShortTitle(QMC2_TITLE_INDEX, tr("Titl&e"));
	componentInfo->setLongTitle(QMC2_TITLE_INDEX, tr("Title screen image"));
	componentInfo->setIcon(QMC2_TITLE_INDEX, QIcon(QString::fromUtf8(":/data/img/arcademode.png")));
	componentInfo->setShortTitle(QMC2_MAWS_INDEX, tr("MA&WS"));
	componentInfo->setLongTitle(QMC2_MAWS_INDEX, tr("MAWS page (web lookup)"));
	componentInfo->setIcon(QMC2_MAWS_INDEX, QIcon(QString::fromUtf8(":/data/img/internet.png")));
	componentInfo->setShortTitle(QMC2_PCB_INDEX, tr("&PCB"));
	componentInfo->setLongTitle(QMC2_PCB_INDEX, tr("PCB image"));
	componentInfo->setIcon(QMC2_PCB_INDEX, QIcon(QString::fromUtf8(":/data/img/circuit.png")));
	componentInfo->setShortTitle(QMC2_SOFTWARE_LIST_INDEX, tr("Softwar&e list"));
	componentInfo->setLongTitle(QMC2_SOFTWARE_LIST_INDEX, tr("Software list"));
	componentInfo->setIcon(QMC2_SOFTWARE_LIST_INDEX, QIcon(QString::fromUtf8(":/data/img/pacman.png")));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setShortTitle(QMC2_YOUTUBE_INDEX, tr("&YouTube"));
	componentInfo->setLongTitle(QMC2_YOUTUBE_INDEX, tr("YouTube videos"));
	componentInfo->setIcon(QMC2_YOUTUBE_INDEX, QIcon(QString::fromUtf8(":/data/img/youtube.png")));
#endif
	componentInfo->setShortTitle(QMC2_SYSTEM_NOTES_INDEX, tr("&Notes"));
	componentInfo->setLongTitle(QMC2_SYSTEM_NOTES_INDEX, tr("System notes"));
	componentInfo->setIcon(QMC2_SYSTEM_NOTES_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->availableFeatureList() << QMC2_PREVIEW_INDEX << QMC2_FLYER_INDEX << QMC2_GAMEINFO_INDEX << QMC2_EMUINFO_INDEX << QMC2_CONFIG_INDEX << QMC2_CABINET_INDEX << QMC2_CONTROLLER_INDEX << QMC2_MARQUEE_INDEX << QMC2_TITLE_INDEX << QMC2_MAWS_INDEX << QMC2_PCB_INDEX << QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
	componentInfo->availableFeatureList() << QMC2_YOUTUBE_INDEX << QMC2_SYSTEM_NOTES_INDEX;
#else
	componentInfo->availableFeatureList() << QMC2_SYSTEM_NOTES_INDEX;
#endif
	componentInfo->setWidget(QMC2_PREVIEW_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX));
	componentInfo->setWidget(QMC2_FLYER_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX));
	componentInfo->setWidget(QMC2_GAMEINFO_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_GAMEINFO_INDEX));
	componentInfo->setWidget(QMC2_EMUINFO_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX));
	componentInfo->setWidget(QMC2_CONFIG_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX));
	componentInfo->setWidget(QMC2_CABINET_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX));
	componentInfo->setWidget(QMC2_CONTROLLER_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX));
	componentInfo->setWidget(QMC2_MARQUEE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX));
	componentInfo->setWidget(QMC2_TITLE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_TITLE_INDEX));
	componentInfo->setWidget(QMC2_MAWS_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MAWS_INDEX));
	componentInfo->setWidget(QMC2_PCB_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PCB_INDEX));
	componentInfo->setWidget(QMC2_SOFTWARE_LIST_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SOFTWARE_LIST_INDEX));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setWidget(QMC2_YOUTUBE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_YOUTUBE_INDEX));
#endif
	componentInfo->setWidget(QMC2_SYSTEM_NOTES_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SYSTEM_NOTES_INDEX));
	componentInfo->configurableFeatureList() << QMC2_MAWS_INDEX;
#if QMC2_YOUTUBE_ENABLED
	componentInfo->configurableFeatureList() << QMC2_YOUTUBE_INDEX;
#endif
#elif defined(QMC2_EMUTYPE_MESS)
	componentInfo->setShortTitle(QMC2_PREVIEW_INDEX, tr("Pre&view"));
	componentInfo->setLongTitle(QMC2_PREVIEW_INDEX, tr("Machine preview image"));
	componentInfo->setIcon(QMC2_PREVIEW_INDEX, QIcon(QString::fromUtf8(":/data/img/camera.png")));
	componentInfo->setShortTitle(QMC2_FLYER_INDEX, tr("Fl&yer"));
	componentInfo->setLongTitle(QMC2_FLYER_INDEX, tr("Machine flyer image"));
	componentInfo->setIcon(QMC2_FLYER_INDEX, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")));
	componentInfo->setShortTitle(QMC2_MACHINEINFO_INDEX, tr("Machine &info"));
	componentInfo->setLongTitle(QMC2_MACHINEINFO_INDEX, tr("Machine information"));
	componentInfo->setIcon(QMC2_MACHINEINFO_INDEX, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setShortTitle(QMC2_EMUINFO_INDEX, tr("Em&ulator info"));
	componentInfo->setLongTitle(QMC2_EMUINFO_INDEX, tr("Emulator information"));
	componentInfo->setIcon(QMC2_EMUINFO_INDEX, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setShortTitle(QMC2_CONFIG_INDEX, tr("&Configuration"));
	componentInfo->setLongTitle(QMC2_CONFIG_INDEX, tr("Emulator configuration"));
	componentInfo->setIcon(QMC2_CONFIG_INDEX, QIcon(QString::fromUtf8(":/data/img/work.png")));
	componentInfo->setRemovable(QMC2_CONFIG_INDEX, false);
	componentInfo->setShortTitle(QMC2_DEVICE_INDEX, tr("De&vices"));
	componentInfo->setLongTitle(QMC2_DEVICE_INDEX, tr("Device configuration"));
	componentInfo->setIcon(QMC2_DEVICE_INDEX, QIcon(QString::fromUtf8(":/data/img/tape.png")));
	componentInfo->setShortTitle(QMC2_PROJECTMESS_INDEX, tr("Pr&ojectMESS"));
	componentInfo->setLongTitle(QMC2_PROJECTMESS_INDEX, tr("ProjectMESS (web lookup)"));
	componentInfo->setIcon(QMC2_PROJECTMESS_INDEX, QIcon(QString::fromUtf8(":/data/img/project_mess.png")));
	componentInfo->setShortTitle(QMC2_PCB_INDEX, tr("&PCB"));
	componentInfo->setLongTitle(QMC2_PCB_INDEX, tr("PCB image"));
	componentInfo->setIcon(QMC2_PCB_INDEX, QIcon(QString::fromUtf8(":/data/img/circuit.png")));
	componentInfo->setShortTitle(QMC2_CABINET_INDEX, tr("Ca&binet"));
	componentInfo->setLongTitle(QMC2_CABINET_INDEX, tr("Machine cabinet image"));
	componentInfo->setIcon(QMC2_CABINET_INDEX, QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")));
	componentInfo->setShortTitle(QMC2_CONTROLLER_INDEX, tr("C&ontroller"));
	componentInfo->setLongTitle(QMC2_CONTROLLER_INDEX, tr("Control panel image"));
	componentInfo->setIcon(QMC2_CONTROLLER_INDEX, QIcon(QString::fromUtf8(":/data/img/joystick.png")));
	componentInfo->setShortTitle(QMC2_LOGO_INDEX, tr("Lo&go"));
	componentInfo->setLongTitle(QMC2_LOGO_INDEX, tr("Logo image"));
	componentInfo->setIcon(QMC2_LOGO_INDEX, QIcon(QString::fromUtf8(":/data/img/marquee.png")));
	componentInfo->setShortTitle(QMC2_SOFTWARE_LIST_INDEX, tr("Softwar&e list"));
	componentInfo->setLongTitle(QMC2_SOFTWARE_LIST_INDEX, tr("Software list"));
	componentInfo->setIcon(QMC2_SOFTWARE_LIST_INDEX, QIcon(QString::fromUtf8(":/data/img/pacman.png")));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setShortTitle(QMC2_YOUTUBE_INDEX, tr("&YouTube"));
	componentInfo->setLongTitle(QMC2_YOUTUBE_INDEX, tr("YouTube videos"));
	componentInfo->setIcon(QMC2_YOUTUBE_INDEX, QIcon(QString::fromUtf8(":/data/img/youtube.png")));
#endif
	componentInfo->setShortTitle(QMC2_SYSTEM_NOTES_INDEX, tr("&Notes"));
	componentInfo->setLongTitle(QMC2_SYSTEM_NOTES_INDEX, tr("System notes"));
	componentInfo->setIcon(QMC2_SYSTEM_NOTES_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->availableFeatureList() << QMC2_PREVIEW_INDEX << QMC2_FLYER_INDEX << QMC2_MACHINEINFO_INDEX << QMC2_EMUINFO_INDEX << QMC2_CONFIG_INDEX << QMC2_DEVICE_INDEX << QMC2_PROJECTMESS_INDEX << QMC2_PCB_INDEX << QMC2_CABINET_INDEX << QMC2_CONTROLLER_INDEX << QMC2_MARQUEE_INDEX << QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
	componentInfo->availableFeatureList() << QMC2_YOUTUBE_INDEX << QMC2_SYSTEM_NOTES_INDEX;
#else
	componentInfo->availableFeatureList() << QMC2_SYSTEM_NOTES_INDEX;
#endif
	componentInfo->setWidget(QMC2_PREVIEW_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX));
	componentInfo->setWidget(QMC2_FLYER_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX));
	componentInfo->setWidget(QMC2_MACHINEINFO_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MACHINEINFO_INDEX));
	componentInfo->setWidget(QMC2_EMUINFO_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX));
	componentInfo->setWidget(QMC2_CONFIG_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX));
	componentInfo->setWidget(QMC2_DEVICE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_DEVICE_INDEX));
	componentInfo->setWidget(QMC2_PROJECTMESS_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PROJECTMESS_INDEX));
	componentInfo->setWidget(QMC2_PCB_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PCB_INDEX));
	componentInfo->setWidget(QMC2_CABINET_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX));
	componentInfo->setWidget(QMC2_CONTROLLER_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX));
	componentInfo->setWidget(QMC2_MARQUEE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX));
	componentInfo->setWidget(QMC2_SOFTWARE_LIST_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SOFTWARE_LIST_INDEX));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setWidget(QMC2_YOUTUBE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_YOUTUBE_INDEX));
	componentInfo->configurableFeatureList() << QMC2_YOUTUBE_INDEX;
#endif
	componentInfo->setWidget(QMC2_SYSTEM_NOTES_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SYSTEM_NOTES_INDEX));
#elif defined(QMC2_EMUTYPE_UME)
	componentInfo->setShortTitle(QMC2_PREVIEW_INDEX, tr("Pre&view"));
	componentInfo->setLongTitle(QMC2_PREVIEW_INDEX, tr("Game preview image"));
	componentInfo->setIcon(QMC2_PREVIEW_INDEX, QIcon(QString::fromUtf8(":/data/img/camera.png")));
	componentInfo->setShortTitle(QMC2_FLYER_INDEX, tr("Fl&yer"));
	componentInfo->setLongTitle(QMC2_FLYER_INDEX, tr("Game flyer image"));
	componentInfo->setIcon(QMC2_FLYER_INDEX, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")));
	componentInfo->setShortTitle(QMC2_GAMEINFO_INDEX, tr("Game &info"));
	componentInfo->setLongTitle(QMC2_GAMEINFO_INDEX, tr("Game information"));
	componentInfo->setIcon(QMC2_GAMEINFO_INDEX, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setShortTitle(QMC2_EMUINFO_INDEX, tr("Em&ulator info"));
	componentInfo->setLongTitle(QMC2_EMUINFO_INDEX, tr("Emulator information"));
	componentInfo->setIcon(QMC2_EMUINFO_INDEX, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setShortTitle(QMC2_CONFIG_INDEX, tr("&Configuration"));
	componentInfo->setLongTitle(QMC2_CONFIG_INDEX, tr("Emulator configuration"));
	componentInfo->setIcon(QMC2_CONFIG_INDEX, QIcon(QString::fromUtf8(":/data/img/work.png")));
	componentInfo->setRemovable(QMC2_CONFIG_INDEX, false);
	componentInfo->setShortTitle(QMC2_DEVICE_INDEX, tr("De&vices"));
	componentInfo->setLongTitle(QMC2_DEVICE_INDEX, tr("Device configuration"));
	componentInfo->setIcon(QMC2_DEVICE_INDEX, QIcon(QString::fromUtf8(":/data/img/tape.png")));
	componentInfo->setShortTitle(QMC2_PROJECTMESS_INDEX, tr("Pr&ojectMESS"));
	componentInfo->setLongTitle(QMC2_PROJECTMESS_INDEX, tr("ProjectMESS (web lookup)"));
	componentInfo->setIcon(QMC2_PROJECTMESS_INDEX, QIcon(QString::fromUtf8(":/data/img/project_mess.png")));
	componentInfo->setShortTitle(QMC2_CABINET_INDEX, tr("Ca&binet"));
	componentInfo->setLongTitle(QMC2_CABINET_INDEX, tr("Arcade cabinet image"));
	componentInfo->setIcon(QMC2_CABINET_INDEX, QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")));
	componentInfo->setShortTitle(QMC2_CONTROLLER_INDEX, tr("C&ontroller"));
	componentInfo->setLongTitle(QMC2_CONTROLLER_INDEX, tr("Control panel image"));
	componentInfo->setIcon(QMC2_CONTROLLER_INDEX, QIcon(QString::fromUtf8(":/data/img/joystick.png")));
	componentInfo->setShortTitle(QMC2_MARQUEE_INDEX, tr("Mar&quee"));
	componentInfo->setLongTitle(QMC2_MARQUEE_INDEX, tr("Marquee image"));
	componentInfo->setIcon(QMC2_MARQUEE_INDEX, QIcon(QString::fromUtf8(":/data/img/marquee.png")));
	componentInfo->setShortTitle(QMC2_TITLE_INDEX, tr("Titl&e"));
	componentInfo->setLongTitle(QMC2_TITLE_INDEX, tr("Title screen image"));
	componentInfo->setIcon(QMC2_TITLE_INDEX, QIcon(QString::fromUtf8(":/data/img/arcademode.png")));
	componentInfo->setShortTitle(QMC2_PCB_INDEX, tr("&PCB"));
	componentInfo->setLongTitle(QMC2_PCB_INDEX, tr("PCB image"));
	componentInfo->setIcon(QMC2_PCB_INDEX, QIcon(QString::fromUtf8(":/data/img/circuit.png")));
	componentInfo->setShortTitle(QMC2_SOFTWARE_LIST_INDEX, tr("Softwar&e list"));
	componentInfo->setLongTitle(QMC2_SOFTWARE_LIST_INDEX, tr("Software list"));
	componentInfo->setIcon(QMC2_SOFTWARE_LIST_INDEX, QIcon(QString::fromUtf8(":/data/img/pacman.png")));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setShortTitle(QMC2_YOUTUBE_INDEX, tr("&YouTube"));
	componentInfo->setLongTitle(QMC2_YOUTUBE_INDEX, tr("YouTube videos"));
	componentInfo->setIcon(QMC2_YOUTUBE_INDEX, QIcon(QString::fromUtf8(":/data/img/youtube.png")));
#endif
	componentInfo->setShortTitle(QMC2_SYSTEM_NOTES_INDEX, tr("&Notes"));
	componentInfo->setLongTitle(QMC2_SYSTEM_NOTES_INDEX, tr("System notes"));
	componentInfo->setIcon(QMC2_SYSTEM_NOTES_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->availableFeatureList() << QMC2_PREVIEW_INDEX << QMC2_FLYER_INDEX << QMC2_GAMEINFO_INDEX << QMC2_EMUINFO_INDEX << QMC2_CONFIG_INDEX << QMC2_DEVICE_INDEX << QMC2_PROJECTMESS_INDEX << QMC2_CABINET_INDEX << QMC2_CONTROLLER_INDEX << QMC2_MARQUEE_INDEX << QMC2_TITLE_INDEX << QMC2_PCB_INDEX << QMC2_SOFTWARE_LIST_INDEX;
#if QMC2_YOUTUBE_ENABLED
	componentInfo->availableFeatureList() << QMC2_YOUTUBE_INDEX << QMC2_SYSTEM_NOTES_INDEX;
#else
	componentInfo->availableFeatureList() << QMC2_SYSTEM_NOTES_INDEX;
#endif
	componentInfo->setWidget(QMC2_PREVIEW_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PREVIEW_INDEX));
	componentInfo->setWidget(QMC2_FLYER_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_FLYER_INDEX));
	componentInfo->setWidget(QMC2_GAMEINFO_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_GAMEINFO_INDEX));
	componentInfo->setWidget(QMC2_EMUINFO_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_EMUINFO_INDEX));
	componentInfo->setWidget(QMC2_CONFIG_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONFIG_INDEX));
	componentInfo->setWidget(QMC2_DEVICE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_DEVICE_INDEX));
	componentInfo->setWidget(QMC2_PROJECTMESS_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PROJECTMESS_INDEX));
	componentInfo->setWidget(QMC2_CABINET_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CABINET_INDEX));
	componentInfo->setWidget(QMC2_CONTROLLER_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_CONTROLLER_INDEX));
	componentInfo->setWidget(QMC2_MARQUEE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_MARQUEE_INDEX));
	componentInfo->setWidget(QMC2_TITLE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_TITLE_INDEX));
	componentInfo->setWidget(QMC2_PCB_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_PCB_INDEX));
	componentInfo->setWidget(QMC2_SOFTWARE_LIST_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SOFTWARE_LIST_INDEX));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setWidget(QMC2_YOUTUBE_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_YOUTUBE_INDEX));
	componentInfo->configurableFeatureList() << QMC2_YOUTUBE_INDEX;
#endif
	componentInfo->setWidget(QMC2_SYSTEM_NOTES_INDEX, qmc2MainWindow->tabWidgetGameDetail->widget(QMC2_SYSTEM_NOTES_INDEX));
#endif
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/Component2/ActiveFeatures") ) {
		foreach (int index, componentInfo->availableFeatureList())
			componentInfo->activeFeatureList() << index;
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Component2/ActiveFeatures").toStringList();
		foreach (QString sIndex, activeIndexList) {
			int index = sIndex.toInt();
			if ( componentInfo->availableFeatureList().contains(index) )
				componentInfo->activeFeatureList() << index;
		}
	}
	m_componentToWidgetHash["Component2"] = qmc2MainWindow->tabWidgetGameDetail;
	return componentInfo;
}

ComponentInfo *ComponentSetup::initComponent3()
{
	ComponentInfo *componentInfo = new ComponentInfo();

	componentInfo->setShortTitle(QMC2_FRONTENDLOG_INDEX, tr("&Front end log"));
	componentInfo->setLongTitle(QMC2_FRONTENDLOG_INDEX, tr("Front end log"));
	componentInfo->setIcon(QMC2_FRONTENDLOG_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->setWidget(QMC2_FRONTENDLOG_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_FRONTENDLOG_INDEX));
	componentInfo->setShortTitle(QMC2_EMULATORLOG_INDEX, tr("Emulator &log"));
	componentInfo->setLongTitle(QMC2_EMULATORLOG_INDEX, tr("Emulator log"));
	componentInfo->setIcon(QMC2_EMULATORLOG_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->setWidget(QMC2_EMULATORLOG_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_EMULATORLOG_INDEX));
	componentInfo->setShortTitle(QMC2_EMULATORCONTROL_INDEX, tr("E&mulator control"));
	componentInfo->setLongTitle(QMC2_EMULATORCONTROL_INDEX, tr("Emulator control panel"));
	componentInfo->setIcon(QMC2_EMULATORCONTROL_INDEX, QIcon(QString::fromUtf8(":/data/img/process.png")));
	componentInfo->setWidget(QMC2_EMULATORCONTROL_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_EMULATORCONTROL_INDEX));
#if QMC2_USE_PHONON_API
	componentInfo->setShortTitle(QMC2_AUDIOPLAYER_INDEX, tr("&Audio player"));
	componentInfo->setLongTitle(QMC2_AUDIOPLAYER_INDEX, tr("Audio player"));
	componentInfo->setIcon(QMC2_AUDIOPLAYER_INDEX, QIcon(QString::fromUtf8(":/data/img/music.png")));
	componentInfo->setWidget(QMC2_AUDIOPLAYER_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_AUDIOPLAYER_INDEX));
#endif
	componentInfo->setShortTitle(QMC2_DOWNLOADS_INDEX, tr("Do&wnloads"));
	componentInfo->setLongTitle(QMC2_DOWNLOADS_INDEX, tr("Downloads"));
	componentInfo->setIcon(QMC2_DOWNLOADS_INDEX, QIcon(QString::fromUtf8(":/data/img/download.png")));
	componentInfo->setWidget(QMC2_DOWNLOADS_INDEX, qmc2MainWindow->tabWidgetGamelist->widget(QMC2_DOWNLOADS_INDEX));
#if QMC2_USE_PHONON_API
	componentInfo->availableFeatureList() << QMC2_FRONTENDLOG_INDEX << QMC2_EMULATORLOG_INDEX << QMC2_EMULATORCONTROL_INDEX << QMC2_AUDIOPLAYER_INDEX << QMC2_DOWNLOADS_INDEX;
#else
	componentInfo->availableFeatureList() << QMC2_FRONTENDLOG_INDEX << QMC2_EMULATORLOG_INDEX << QMC2_EMULATORCONTROL_INDEX << QMC2_DOWNLOADS_INDEX;
#endif
	m_componentToWidgetHash["Component3"] = qmc2MainWindow->tabWidgetLogsAndEmulators;
	return componentInfo;
}

void ComponentSetup::loadComponent(QString name)
{
	listWidgetAvailableFeatures->clear();
	listWidgetActiveFeatures->clear();

	if ( name.isEmpty() )
		name = m_components[comboBoxComponents->currentIndex()];

	ComponentInfo *componentInfo = componentInfoHash()[name];

	if ( !componentInfo )
		return;

	foreach (int index, componentInfo->availableFeatureList()) {
		QListWidgetItem *item = new QListWidgetItem(componentInfo->icon(index), componentInfo->longTitle(index));
		item->setData(Qt::UserRole, index);
		listWidgetAvailableFeatures->addItem(item);
	}

	foreach (int index, componentInfo->activeFeatureList()) {
		QListWidgetItem *item = new QListWidgetItem(componentInfo->icon(index), componentInfo->longTitle(index));
		item->setData(Qt::UserRole, index);
		listWidgetActiveFeatures->addItem(item);
	}
}

void ComponentSetup::saveComponent(QString name)
{
	if ( name.isEmpty() )
		name = m_components[comboBoxComponents->currentIndex()];

	ComponentInfo *componentInfo = componentInfoHash()[name];

	if ( !componentInfo )
		return;

	QTabWidget *tabWidget = m_componentToWidgetHash[name];

	if ( !tabWidget )
		return;

	int oldIndex = componentInfo->appliedFeatureList()[tabWidget->currentIndex()];
	tabWidget->clear();
	componentInfo->appliedFeatureList().clear();

	QStringList activeIndexList;
	foreach (int index, componentInfo->activeFeatureList()) {
		if ( componentInfo->availableFeatureList().contains(index) ) {
			componentInfo->appliedFeatureList() << index;
			activeIndexList << QString::number(index);
			tabWidget->addTab(componentInfo->widget(index), componentInfo->icon(index), componentInfo->shortTitle(index));
		}
	}

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + name + "/ActiveFeatures", activeIndexList);

	if ( componentInfo->appliedFeatureList().contains(oldIndex) )
		tabWidget->setCurrentIndex(componentInfo->appliedFeatureList().indexOf(oldIndex));
}

void ComponentSetup::adjustIconSizes()
{
	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	pushButtonConfigureFeature->setIconSize(iconSize);
	pushButtonActivateFeatures->setIconSize(iconSize);
	pushButtonDeactivateFeatures->setIconSize(iconSize);
	pushButtonFeatureUp->setIconSize(iconSize);
	pushButtonFeatureDown->setIconSize(iconSize);
	pushButtonOk->setIconSize(iconSize);
	pushButtonApply->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	listWidgetAvailableFeatures->setIconSize(iconSize);
	listWidgetActiveFeatures->setIconSize(iconSize);
}

void ComponentSetup::on_listWidgetAvailableFeatures_itemSelectionChanged()
{
	ComponentInfo *componentInfo = componentInfoHash()[m_components[comboBoxComponents->currentIndex()]];

	if ( !componentInfo )
		return;

	if ( listWidgetAvailableFeatures->selectedItems().count() > 0 ) {
		pushButtonActivateFeatures->setEnabled(true);
		if ( listWidgetAvailableFeatures->selectedItems().count() == 1 ) {
			if ( componentInfo->configurableFeatureList().contains(componentInfo->longTitleHash().key(listWidgetAvailableFeatures->selectedItems()[0]->text())) )
				pushButtonConfigureFeature->setEnabled(true);
			else
				pushButtonConfigureFeature->setEnabled(false);
		} else
			pushButtonConfigureFeature->setEnabled(false);
	} else
		pushButtonActivateFeatures->setEnabled(false);
}
 
void ComponentSetup::on_listWidgetActiveFeatures_itemSelectionChanged()
{
	if ( listWidgetActiveFeatures->selectedItems().count() > 0 ) {
		pushButtonDeactivateFeatures->setEnabled(true);
		if ( listWidgetActiveFeatures->selectedItems().count() == 1 ) {
			pushButtonFeatureUp->setEnabled(true);
			pushButtonFeatureDown->setEnabled(true);
		} else {
			pushButtonFeatureUp->setEnabled(false);
			pushButtonFeatureDown->setEnabled(false);
		}
	} else {
		pushButtonDeactivateFeatures->setEnabled(false);
		pushButtonFeatureUp->setEnabled(false);
		pushButtonFeatureDown->setEnabled(false);
	}
}

void ComponentSetup::on_pushButtonActivateFeatures_clicked()
{
	ComponentInfo *componentInfo = componentInfoHash()[m_components[comboBoxComponents->currentIndex()]];

	if ( !componentInfo )
		return;

	foreach (QListWidgetItem *item, listWidgetAvailableFeatures->selectedItems()) {
		if ( item ) {
			QList<QListWidgetItem *> il = listWidgetActiveFeatures->findItems(item->text(), Qt::MatchExactly); 
			if ( il.count() == 0 ) {
				int pageIndex = componentInfo->longTitleHash().key(item->text());
				if ( componentInfo->availableFeatureList().contains(pageIndex) ) {
					QListWidgetItem *newItem = new QListWidgetItem(componentInfo->icon(pageIndex), item->text());
					newItem->setData(Qt::UserRole, pageIndex);
					listWidgetActiveFeatures->addItem(newItem);
					componentInfo->activeFeatureList() << pageIndex;
				}
			}
		}
	}
}

void ComponentSetup::on_pushButtonConfigureFeature_clicked()
{
	ComponentInfo *componentInfo = componentInfoHash()[m_components[comboBoxComponents->currentIndex()]];

	if ( !componentInfo )
		return;

	// FIXME (supports only component 2 ATM)
	if ( listWidgetAvailableFeatures->selectedItems().count() == 1 ) {
		int pageIndex = componentInfo->longTitleHash().key(listWidgetAvailableFeatures->selectedItems()[0]->text());
		if ( componentInfo->configurableFeatureList().contains(pageIndex) ) {
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

void ComponentSetup::on_pushButtonDeactivateFeatures_clicked()
{
	ComponentInfo *componentInfo = componentInfoHash()[m_components[comboBoxComponents->currentIndex()]];

	if ( !componentInfo )
		return;

	foreach (QListWidgetItem *item, listWidgetActiveFeatures->selectedItems()) {
		if ( item ) {
			if ( !componentInfo->removable(item->data(Qt::UserRole).toInt()) )
				continue;
			int row = listWidgetActiveFeatures->row(item);
			QListWidgetItem *takenItem = listWidgetActiveFeatures->takeItem(row);
			if ( takenItem )
				delete takenItem;
			if ( row >= 0 && row < componentInfo->activeFeatureList().count() )
				componentInfo->activeFeatureList().removeAt(row);
		}
	}
}

void ComponentSetup::on_pushButtonFeatureUp_clicked()
{
	ComponentInfo *componentInfo = componentInfoHash()[m_components[comboBoxComponents->currentIndex()]];

	if ( !componentInfo )
		return;

	foreach (QListWidgetItem *item, listWidgetActiveFeatures->selectedItems()) {
		if ( item ) {
			int row = listWidgetActiveFeatures->row(item);
			if ( row > 0 ) {
				componentInfo->activeFeatureList().move(row, row - 1);
				QListWidgetItem *takenItem = listWidgetActiveFeatures->takeItem(row);
				if ( takenItem ) {
					listWidgetActiveFeatures->insertItem(row - 1, takenItem);
					takenItem->setSelected(true);
				}
			}
		}
	}
}

void ComponentSetup::on_pushButtonFeatureDown_clicked()
{
	ComponentInfo *componentInfo = componentInfoHash()[m_components[comboBoxComponents->currentIndex()]];

	if ( !componentInfo )
		return;

	foreach (QListWidgetItem *item, listWidgetActiveFeatures->selectedItems()) {
		if ( item ) {
			int row = listWidgetActiveFeatures->row(item);
			if ( row < listWidgetActiveFeatures->count() - 1 ) {
				componentInfo->activeFeatureList().move(row, row + 1);
				QListWidgetItem *takenItem = listWidgetActiveFeatures->takeItem(row);
				if ( takenItem ) {
					listWidgetActiveFeatures->insertItem(row + 1, takenItem);
					takenItem->setSelected(true);
				}
			}
		}
	}
}

void ComponentSetup::on_comboBoxComponents_currentIndexChanged(int index)
{
	switch ( index ) {
		case 0:
			loadComponent("Component1");
			break;
		case 1:
			loadComponent("Component2");
			break;
		case 2:
			loadComponent("Component3");
			break;
		default:
			break;
	}
}

void ComponentSetup::showEvent(QShowEvent *)
{
	loadComponent();
}
