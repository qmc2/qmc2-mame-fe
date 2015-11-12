#include <QtGui>
#include <QFileDialog>
#include <QInputDialog>
#include <QHashIterator>
#include <QMessageBox>

#include "componentsetup.h"
#include "customartwork.h"
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
	componentInfoHash().insert("Component1", initComponent1());
	componentInfoHash().insert("Component2", initComponent2());
	componentInfoHash().insert("Component3", initComponent3());
	componentInfoHash().insert("Component4", initComponent4());

	// prepare component arrangement
	splitter0 = qmc2MainWindow->hSplitter;
	splitter1 = qmc2MainWindow->vSplitter;
	widget00 = splitter0->widget(0);
	widget01 = splitter0->widget(1);
	widget10 = splitter1->widget(0);
	widget11 = splitter1->widget(1);

	// init UI
	setupUi(this);
	hide();
	adjustSize();
	connect(this, SIGNAL(rejected()), this, SLOT(on_pushButtonCancel_clicked()));

	// load current component arrangement
	loadArrangement();
}

ComponentSetup::~ComponentSetup()
{
	// NOP
}

ComponentInfo *ComponentSetup::initComponent1()
{
	ComponentInfo *componentInfo = new ComponentInfo();
	componentInfo->setShortTitle(QMC2_MACHINELIST_INDEX, tr("Machine &list"));
	componentInfo->setLongTitle(QMC2_MACHINELIST_INDEX, tr("Machine list"));
	componentInfo->setIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/flat.png")));
	componentInfo->setWidget(QMC2_MACHINELIST_INDEX, qmc2MainWindow->tabWidgetMachineList->widget(QMC2_MACHINELIST_INDEX));
	componentInfo->setShortTitle(QMC2_SEARCH_INDEX, tr("&Search"));
	componentInfo->setLongTitle(QMC2_SEARCH_INDEX, tr("Search systems"));
	componentInfo->setIcon(QMC2_SEARCH_INDEX, QIcon(QString::fromUtf8(":/data/img/hint.png")));
	componentInfo->setWidget(QMC2_SEARCH_INDEX, qmc2MainWindow->tabWidgetMachineList->widget(QMC2_SEARCH_INDEX));
	componentInfo->setShortTitle(QMC2_FAVORITES_INDEX, tr("Favo&rites"));
	componentInfo->setLongTitle(QMC2_FAVORITES_INDEX, tr("Favorite list"));
	componentInfo->setIcon(QMC2_FAVORITES_INDEX, QIcon(QString::fromUtf8(":/data/img/favorites.png")));
	componentInfo->setWidget(QMC2_FAVORITES_INDEX, qmc2MainWindow->tabWidgetMachineList->widget(QMC2_FAVORITES_INDEX));
	componentInfo->setShortTitle(QMC2_PLAYED_INDEX, tr("Pl&ayed"));
	componentInfo->setLongTitle(QMC2_PLAYED_INDEX, tr("Played list"));
	componentInfo->setIcon(QMC2_PLAYED_INDEX, QIcon(QString::fromUtf8(":/data/img/time.png")));
	componentInfo->setWidget(QMC2_PLAYED_INDEX, qmc2MainWindow->tabWidgetMachineList->widget(QMC2_PLAYED_INDEX));
	componentInfo->setShortTitle(QMC2_FOREIGN_INDEX, tr("&Foreign emulators"));
	componentInfo->setLongTitle(QMC2_FOREIGN_INDEX, tr("Foreign emulator list"));
	componentInfo->setIcon(QMC2_FOREIGN_INDEX, QIcon(QString::fromUtf8(":/data/img/alien.png")));
	componentInfo->setWidget(QMC2_FOREIGN_INDEX, qmc2MainWindow->tabWidgetMachineList->widget(QMC2_FOREIGN_INDEX));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	componentInfo->setShortTitle(QMC2_EMBED_INDEX, tr("Embedded emulators"));
	componentInfo->setLongTitle(QMC2_EMBED_INDEX, tr("Embedded emulators"));
	componentInfo->setIcon(QMC2_EMBED_INDEX, QIcon(QString::fromUtf8(":/data/img/embed.png")));
	componentInfo->setWidget(QMC2_EMBED_INDEX, qmc2MainWindow->tabWidgetMachineList->widget(QMC2_EMBED_INDEX));
	componentInfo->availableFeatureList() << QMC2_MACHINELIST_INDEX << QMC2_SEARCH_INDEX << QMC2_FAVORITES_INDEX << QMC2_PLAYED_INDEX << QMC2_FOREIGN_INDEX << QMC2_EMBED_INDEX;
#else
	componentInfo->availableFeatureList() << QMC2_MACHINELIST_INDEX << QMC2_SEARCH_INDEX << QMC2_FAVORITES_INDEX << QMC2_PLAYED_INDEX << QMC2_FOREIGN_INDEX;
#endif
	components() << "Component1";
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures") ) {
		foreach (int index, componentInfo->availableFeatureList())
			componentInfo->activeFeatureList() << index;
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures").toStringList();
		foreach (QString sIndex, activeIndexList) {
			int index = sIndex.toInt();
			if ( componentInfo->availableFeatureList().contains(index) )
				componentInfo->activeFeatureList() << index;
		}
	}
	m_componentToWidgetHash[components().last()] = qmc2MainWindow->tabWidgetMachineList;
	m_componentToSplitterHash[components().last()] = qmc2MainWindow->hSplitter;
	m_componentToSplitterIndexHash[components().last()] = 0;
	return componentInfo;
}

ComponentInfo *ComponentSetup::initComponent2()
{
	ComponentInfo *componentInfo = new ComponentInfo();
	componentInfo->setShortTitle(QMC2_PREVIEW_INDEX, tr("Pre&view"));
	componentInfo->setLongTitle(QMC2_PREVIEW_INDEX, tr("Machine preview image"));
	componentInfo->setIcon(QMC2_PREVIEW_INDEX, QIcon(QString::fromUtf8(":/data/img/camera.png")));
	componentInfo->setShortTitle(QMC2_FLYER_INDEX, tr("Fl&yer"));
	componentInfo->setLongTitle(QMC2_FLYER_INDEX, tr("Machine flyer image"));
	componentInfo->setIcon(QMC2_FLYER_INDEX, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")));
	componentInfo->setShortTitle(QMC2_GAMEINFO_INDEX, tr("Machine &info"));
	componentInfo->setLongTitle(QMC2_GAMEINFO_INDEX, tr("Machine information"));
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
	componentInfo->setWidget(QMC2_PREVIEW_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_PREVIEW_INDEX));
	componentInfo->setWidget(QMC2_FLYER_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_FLYER_INDEX));
	componentInfo->setWidget(QMC2_GAMEINFO_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_GAMEINFO_INDEX));
	componentInfo->setWidget(QMC2_EMUINFO_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_EMUINFO_INDEX));
	componentInfo->setWidget(QMC2_CONFIG_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_CONFIG_INDEX));
	componentInfo->setWidget(QMC2_DEVICE_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_DEVICE_INDEX));
	componentInfo->setWidget(QMC2_PROJECTMESS_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_PROJECTMESS_INDEX));
	componentInfo->setWidget(QMC2_CABINET_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_CABINET_INDEX));
	componentInfo->setWidget(QMC2_CONTROLLER_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_CONTROLLER_INDEX));
	componentInfo->setWidget(QMC2_MARQUEE_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_MARQUEE_INDEX));
	componentInfo->setWidget(QMC2_TITLE_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_TITLE_INDEX));
	componentInfo->setWidget(QMC2_PCB_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_PCB_INDEX));
	componentInfo->setWidget(QMC2_SOFTWARE_LIST_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_SOFTWARE_LIST_INDEX));
#if QMC2_YOUTUBE_ENABLED
	componentInfo->setWidget(QMC2_YOUTUBE_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_YOUTUBE_INDEX));
	componentInfo->configurableFeatureList() << QMC2_YOUTUBE_INDEX;
#endif
	componentInfo->setWidget(QMC2_SYSTEM_NOTES_INDEX, qmc2MainWindow->tabWidgetMachineDetail->widget(QMC2_SYSTEM_NOTES_INDEX));

	int num = 0;
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups()) {
		if ( qmc2Config->value(QString("%1/Target").arg(name), 0).toInt() == QMC2_AW_INDEX_TARGET_SYSTEM ) {
			int featureIndex = QMC2_USEROFFSET_INDEX + num;
			QString nameCopy = name;
			componentInfo->setShortTitle(featureIndex, name);
			componentInfo->setLongTitle(featureIndex, nameCopy.replace("&", QString()));
			componentInfo->setIcon(featureIndex, QIcon(qmc2Config->value(QString("%1/Icon").arg(name), QString()).toString()));
			componentInfo->availableFeatureList() << featureIndex;
			qmc2MainWindow->tabWidgetMachineDetail->insertTab(featureIndex, new CustomArtwork(qmc2MainWindow->tabWidgetMachineDetail, name, num), QIcon(qmc2Config->value(QString("%1/Icon").arg(name), QString()).toString()), name);
			componentInfo->setWidget(featureIndex, qmc2MainWindow->tabWidgetMachineDetail->widget(featureIndex));
			num++;
		}
	}
	qmc2Config->endGroup();

	components() << "Component2";
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures") ) {
		foreach (int index, componentInfo->availableFeatureList())
			componentInfo->activeFeatureList() << index;
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures").toStringList();
		foreach (QString sIndex, activeIndexList) {
			int index = sIndex.toInt();
			if ( componentInfo->availableFeatureList().contains(index) )
				componentInfo->activeFeatureList() << index;
		}
	}
	m_componentToWidgetHash[components().last()] = qmc2MainWindow->tabWidgetMachineDetail;
	m_componentToSplitterHash[components().last()] = qmc2MainWindow->vSplitter;
	m_componentToSplitterIndexHash[components().last()] = 0;
	return componentInfo;
}

ComponentInfo *ComponentSetup::initComponent3()
{
	ComponentInfo *componentInfo = new ComponentInfo();

	componentInfo->setShortTitle(QMC2_FRONTENDLOG_INDEX, tr("&Front end log"));
	componentInfo->setLongTitle(QMC2_FRONTENDLOG_INDEX, tr("Front end log"));
	componentInfo->setIcon(QMC2_FRONTENDLOG_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->setWidget(QMC2_FRONTENDLOG_INDEX, qmc2MainWindow->tabWidgetLogsAndEmulators->widget(QMC2_FRONTENDLOG_INDEX));
	componentInfo->setShortTitle(QMC2_EMULATORLOG_INDEX, tr("Emulator &log"));
	componentInfo->setLongTitle(QMC2_EMULATORLOG_INDEX, tr("Emulator log"));
	componentInfo->setIcon(QMC2_EMULATORLOG_INDEX, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->setWidget(QMC2_EMULATORLOG_INDEX, qmc2MainWindow->tabWidgetLogsAndEmulators->widget(QMC2_EMULATORLOG_INDEX));
	componentInfo->setShortTitle(QMC2_EMULATORCONTROL_INDEX, tr("E&mulator control"));
	componentInfo->setLongTitle(QMC2_EMULATORCONTROL_INDEX, tr("Emulator control panel"));
	componentInfo->setIcon(QMC2_EMULATORCONTROL_INDEX, QIcon(QString::fromUtf8(":/data/img/process.png")));
	componentInfo->setWidget(QMC2_EMULATORCONTROL_INDEX, qmc2MainWindow->tabWidgetLogsAndEmulators->widget(QMC2_EMULATORCONTROL_INDEX));
#if QMC2_USE_PHONON_API
	componentInfo->setShortTitle(QMC2_AUDIOPLAYER_INDEX, tr("&Audio player"));
	componentInfo->setLongTitle(QMC2_AUDIOPLAYER_INDEX, tr("Audio player"));
	componentInfo->setIcon(QMC2_AUDIOPLAYER_INDEX, QIcon(QString::fromUtf8(":/data/img/music.png")));
	componentInfo->setWidget(QMC2_AUDIOPLAYER_INDEX, qmc2MainWindow->tabWidgetLogsAndEmulators->widget(QMC2_AUDIOPLAYER_INDEX));
#endif
	componentInfo->setShortTitle(QMC2_DOWNLOADS_INDEX, tr("Do&wnloads"));
	componentInfo->setLongTitle(QMC2_DOWNLOADS_INDEX, tr("Downloads"));
	componentInfo->setIcon(QMC2_DOWNLOADS_INDEX, QIcon(QString::fromUtf8(":/data/img/download.png")));
	componentInfo->setWidget(QMC2_DOWNLOADS_INDEX, qmc2MainWindow->tabWidgetLogsAndEmulators->widget(QMC2_DOWNLOADS_INDEX));
#if QMC2_USE_PHONON_API
	componentInfo->availableFeatureList() << QMC2_FRONTENDLOG_INDEX << QMC2_EMULATORLOG_INDEX << QMC2_EMULATORCONTROL_INDEX << QMC2_AUDIOPLAYER_INDEX << QMC2_DOWNLOADS_INDEX;
#else
	componentInfo->availableFeatureList() << QMC2_FRONTENDLOG_INDEX << QMC2_EMULATORLOG_INDEX << QMC2_EMULATORCONTROL_INDEX << QMC2_DOWNLOADS_INDEX;
#endif
	components() << "Component3";
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures") ) {
		foreach (int index, componentInfo->availableFeatureList())
			componentInfo->activeFeatureList() << index;
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures").toStringList();
		foreach (QString sIndex, activeIndexList) {
			int index = sIndex.toInt();
			if ( componentInfo->availableFeatureList().contains(index) )
				componentInfo->activeFeatureList() << index;
		}
	}
	m_componentToWidgetHash[components().last()] = qmc2MainWindow->tabWidgetLogsAndEmulators;
	m_componentToSplitterHash[components().last()] = qmc2MainWindow->vSplitter;
	m_componentToSplitterIndexHash[components().last()] = 1;
	return componentInfo;
}

ComponentInfo *ComponentSetup::initComponent4()
{
	ComponentInfo *componentInfo = new ComponentInfo();

	componentInfo->setShortTitle(QMC2_SWINFO_SNAPSHOT_PAGE, tr("Snapshot"));
	componentInfo->setLongTitle(QMC2_SWINFO_SNAPSHOT_PAGE, tr("Software snapshot"));
	componentInfo->setIcon(QMC2_SWINFO_SNAPSHOT_PAGE, QIcon(QString::fromUtf8(":/data/img/camera.png")));
	componentInfo->setWidget(QMC2_SWINFO_SNAPSHOT_PAGE, qmc2MainWindow->tabWidgetSoftwareDetail->widget(QMC2_SWINFO_SNAPSHOT_PAGE));

	componentInfo->setShortTitle(QMC2_SWINFO_PROJECTMESS_PAGE, tr("ProjectMESS"));
	componentInfo->setLongTitle(QMC2_SWINFO_PROJECTMESS_PAGE, tr("ProjectMESS web lookup"));
	componentInfo->setIcon(QMC2_SWINFO_PROJECTMESS_PAGE, QIcon(QString::fromUtf8(":/data/img/project_mess.png")));
	componentInfo->setWidget(QMC2_SWINFO_PROJECTMESS_PAGE, qmc2MainWindow->tabWidgetSoftwareDetail->widget(QMC2_SWINFO_PROJECTMESS_PAGE));

	componentInfo->setShortTitle(QMC2_SWINFO_NOTES_PAGE, tr("Notes"));
	componentInfo->setLongTitle(QMC2_SWINFO_NOTES_PAGE, tr("Software notes"));
	componentInfo->setIcon(QMC2_SWINFO_NOTES_PAGE, QIcon(QString::fromUtf8(":/data/img/notes.png")));
	componentInfo->setWidget(QMC2_SWINFO_NOTES_PAGE, qmc2MainWindow->tabWidgetSoftwareDetail->widget(QMC2_SWINFO_NOTES_PAGE));

	componentInfo->setShortTitle(QMC2_SWINFO_INFO_PAGE, tr("Software info"));
	componentInfo->setLongTitle(QMC2_SWINFO_INFO_PAGE, tr("Software info entry"));
	componentInfo->setIcon(QMC2_SWINFO_INFO_PAGE, QIcon(QString::fromUtf8(":/data/img/info.png")));
	componentInfo->setWidget(QMC2_SWINFO_INFO_PAGE, qmc2MainWindow->tabWidgetSoftwareDetail->widget(QMC2_SWINFO_INFO_PAGE));

	componentInfo->availableFeatureList() << QMC2_SWINFO_SNAPSHOT_PAGE << QMC2_SWINFO_PROJECTMESS_PAGE << QMC2_SWINFO_NOTES_PAGE << QMC2_SWINFO_INFO_PAGE;
	components() << "Component4";
	if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures") ) {
		foreach (int index, componentInfo->availableFeatureList())
			componentInfo->activeFeatureList() << index;
	} else {
		QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + components().last() + "/ActiveFeatures").toStringList();
		foreach (QString sIndex, activeIndexList) {
			int index = sIndex.toInt();
			if ( componentInfo->availableFeatureList().contains(index) )
				componentInfo->activeFeatureList() << index;
		}
	}
	m_componentToWidgetHash[components().last()] = qmc2MainWindow->tabWidgetSoftwareDetail;
	m_componentToSplitterHash[components().last()] = qmc2MainWindow->vSplitter;
	m_componentToSplitterIndexHash[components().last()] = 1;
	return componentInfo;
}

void ComponentSetup::loadComponent(QString name, bool fromSettings)
{
	listWidgetAvailableFeatures->clear();
	listWidgetActiveFeatures->clear();
	if ( name.isEmpty() )
		name = m_components[comboBoxComponents->currentIndex()];
	ComponentInfo *componentInfo = componentInfoHash()[name];
	if ( !componentInfo )
		return;
	if ( fromSettings ) {
		componentInfo->activeFeatureList().clear();
		if ( !qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/" + name + "/ActiveFeatures") ) {
			foreach (int index, componentInfo->availableFeatureList())
				componentInfo->activeFeatureList() << index;
		} else {
			QStringList activeIndexList = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + name + "/ActiveFeatures").toStringList();
			foreach (QString sIndex, activeIndexList) {
				int index = sIndex.toInt();
				if ( componentInfo->availableFeatureList().contains(index) )
					componentInfo->activeFeatureList() << index;
			}
		}
	}
	if ( name == "Component1" ) {
		switch ( qmc2MainWindow->comboBoxViewSelect->currentIndex() ) {
			case QMC2_VIEWMACHINELIST_INDEX:
				componentInfo->setIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/flat.png")));
				break;
			case QMC2_VIEWHIERARCHY_INDEX:
				componentInfo->setIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/clone.png")));
				break;
			case QMC2_VIEWCATEGORY_INDEX:
				componentInfo->setIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
				break;
			case QMC2_VIEWVERSION_INDEX:
				componentInfo->setIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/version.png")));
				break;
	       }
	}
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
	QTabWidget *tabWidget = m_componentToWidgetHash[name];
	QSplitter *splitter = m_componentToSplitterHash[name];
	int widgetIndex = m_componentToSplitterIndexHash[name];
	QWidget *oldWidget = tabWidget->currentWidget();
	tabWidget->clear();
	componentInfo->appliedFeatureList().clear();
	QStringList activeIndexList;
	foreach (int index, componentInfo->activeFeatureList()) {
		if ( componentInfo->availableFeatureList().contains(index) ) {
			QIcon icon = componentInfo->icon(index);
			bool doAddTab = true;
			if ( name == "Component1" ) {
				switch ( index ) {
					case QMC2_MACHINELIST_INDEX:
						switch ( qmc2MainWindow->comboBoxViewSelect->currentIndex() ) {
							case QMC2_VIEWMACHINELIST_INDEX:
								icon = QIcon(QString::fromUtf8(":/data/img/flat.png"));
								break;
							case QMC2_VIEWHIERARCHY_INDEX:
								icon = QIcon(QString::fromUtf8(":/data/img/clone.png"));
								break;
							case QMC2_VIEWCATEGORY_INDEX:
								icon = QIcon(QString::fromUtf8(":/data/img/category.png"));
								break;
							case QMC2_VIEWVERSION_INDEX:
								icon = QIcon(QString::fromUtf8(":/data/img/version.png"));
								break;
						}
						break;
					case QMC2_FOREIGN_INDEX:
						doAddTab &= (qmc2MainWindow->treeWidgetForeignIDs->topLevelItemCount() > 0);
						break;
				}
			}
			componentInfo->appliedFeatureList() << index;
			activeIndexList << QString::number(index);
			if ( doAddTab )
				tabWidget->addTab(componentInfo->widget(index), icon, componentInfo->shortTitle(index));
		}
	}
	if ( tabWidget->count() > 0 ) {
		splitter->handle(1)->setEnabled(true);
		if ( splitter->orientation() == Qt::Horizontal ) {
			splitter->handle(1)->setFixedWidth(4);
			splitter->handle(1)->setMinimumHeight(0);
			splitter->handle(1)->setMaximumHeight(QWIDGETSIZE_MAX);
		} else {
			splitter->handle(1)->setMinimumWidth(0);
			splitter->handle(1)->setMaximumWidth(QWIDGETSIZE_MAX);
			splitter->handle(1)->setFixedHeight(4);
		}
		splitter->setHandleWidth(4);
		int index = tabWidget->indexOf(oldWidget);
		if ( index >= 0 )
			tabWidget->setCurrentIndex(index);
	} else {
		splitter->handle(1)->setEnabled(false);
		QList<int> maximizedSizes;
		if ( splitter->orientation() == Qt::Horizontal ) {
			if ( widgetIndex == 0 )
				maximizedSizes << 0 << qmc2MainWindow->desktopGeometry.width();
			else
				maximizedSizes << qmc2MainWindow->desktopGeometry.width() << 0;
			qmc2MainWindow->hSplitterSizes = maximizedSizes;
			splitter->handle(1)->setFixedWidth(0);
		} else {
			if ( widgetIndex == 0 )
				maximizedSizes << 0 << qmc2MainWindow->desktopGeometry.height();
			else
				maximizedSizes << qmc2MainWindow->desktopGeometry.height() << 0;
			qmc2MainWindow->vSplitterSizes = maximizedSizes;
			splitter->handle(1)->setFixedHeight(0);
		}
		splitter->setSizes(maximizedSizes);
		splitter->setHandleWidth(1);
	}
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + name + "/ActiveFeatures", activeIndexList);
}

void ComponentSetup::loadArrangement()
{
	int index = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ComponentArrangement", 0).toInt();
	if ( index < 0 || index > 13 )
		index = 0;
	comboBoxArrangements->setCurrentIndex(index);
}

void ComponentSetup::saveArrangement()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ComponentArrangement", comboBoxArrangements->currentIndex());
	setArrangement(comboBoxArrangements->currentIndex());
}

void ComponentSetup::setArrangement(int index)
{
	switch ( index ) {
		case 0:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget00);
			splitter0->insertWidget(1, widget01);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 1:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget00);
			splitter0->insertWidget(1, widget01);
			splitter1->insertWidget(0, widget11);
			splitter1->insertWidget(1, widget10);
			break;
		case 2:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 3:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget11);
			splitter1->insertWidget(1, widget10);
			break;
		case 4:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget00);
			splitter0->insertWidget(1, widget01);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 5:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget00);
			splitter0->insertWidget(1, widget01);
			splitter1->insertWidget(0, widget11);
			splitter1->insertWidget(1, widget10);
			break;
		case 6:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 7:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget11);
			splitter1->insertWidget(1, widget10);
			break;
		case 8:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget00);
			splitter0->insertWidget(1, widget01);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 9:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget00);
			splitter0->insertWidget(1, widget01);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 10:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 11:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget10);
			splitter1->insertWidget(1, widget11);
			break;
		case 12:
			splitter0->setOrientation(Qt::Vertical);
			splitter1->setOrientation(Qt::Vertical);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget11);
			splitter1->insertWidget(1, widget10);
			break;
		case 13:
			splitter0->setOrientation(Qt::Horizontal);
			splitter1->setOrientation(Qt::Horizontal);
			splitter0->insertWidget(0, widget01);
			splitter0->insertWidget(1, widget00);
			splitter1->insertWidget(0, widget11);
			splitter1->insertWidget(1, widget10);
			break;
		default:
			break;
	}
	if ( splitter0->orientation() == Qt::Horizontal ) {
		splitter0->handle(1)->setFixedWidth(4);
		splitter0->handle(1)->setMinimumHeight(0);
		splitter0->handle(1)->setMaximumHeight(QWIDGETSIZE_MAX);
	} else {
		splitter0->handle(1)->setMinimumWidth(0);
		splitter0->handle(1)->setMaximumWidth(QWIDGETSIZE_MAX);
		splitter0->handle(1)->setFixedHeight(4);
	}
	if ( splitter1->orientation() == Qt::Horizontal ) {
		splitter1->handle(1)->setFixedWidth(4);
		splitter1->handle(1)->setMinimumHeight(0);
		splitter1->handle(1)->setMaximumHeight(QWIDGETSIZE_MAX);
	} else {
		splitter1->handle(1)->setMinimumWidth(0);
		splitter1->handle(1)->setMaximumWidth(QWIDGETSIZE_MAX);
		splitter1->handle(1)->setFixedHeight(4);
	}
}

void ComponentSetup::adjustIconSizes()
{
	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeBig(QMC2_MAX(fm.height() + 2, 28), QMC2_MAX(fm.height() + 2, 28));
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
	comboBoxComponents->setIconSize(iconSizeBig);
	comboBoxArrangements->setIconSize(iconSizeBig);
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
#if QMC2_YOUTUBE_ENABLED
				case QMC2_YOUTUBE_INDEX: {
					QString userScopePath = Options::configPath();
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
	static int lastIndex = -1;
	if ( lastIndex > -1 ) {
		ComponentInfo *componentInfo = componentInfoHash()[m_components[lastIndex]];
		bool changed = (componentInfo->activeFeatureList().count() != componentInfo->appliedFeatureList().count());
		for (int i = 0; !changed && i < componentInfo->activeFeatureList().count(); i++)
			 changed = (componentInfo->activeFeatureList()[i] != componentInfo->appliedFeatureList()[i]);
		if ( changed ) {
			switch ( QMessageBox::question(this, tr("Question"), tr("The current component's setup hasn't been applied yet. Apply it now?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes) ) {
				case QMessageBox::Yes:
					saveComponent(m_components[lastIndex]);
					break;
				case QMessageBox::No:
					break;
				case QMessageBox::Cancel:
				default:
					comboBoxComponents->blockSignals(true);
					comboBoxComponents->setCurrentIndex(lastIndex);
					comboBoxComponents->blockSignals(false);
					loadComponent(m_components[lastIndex], false);
					return;
			}
		}
	}
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
		case 3:
			loadComponent("Component4");
			break;
		default:
			break;
	}
	lastIndex = index;
}
