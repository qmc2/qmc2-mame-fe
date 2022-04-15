#include <QProcess>
#include <QThread>
#include <QApplication>

#include "about.h"
#include "macros.h"
#include "options.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "machinelist.h"
#if defined(QMC2_OS_WIN)
#include "windows_tools.h"
#endif

#if QMC2_JOYSTICK == 1
#include <SDL.h>
#endif

#if QMC2_USE_PHONON_API
#include "qmc2_phonon.h"
#endif

#if defined(QMC2_OS_MAC) || defined(QMC2_OS_WIN)
#include <QSysInfo>
#endif

#include <QLibraryInfo>

// external global variables
extern MainWindow *qmc2MainWindow;
extern MachineList *qmc2MachineList;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;

About::About(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	widgetSize = QSize(-1, -1);
	widgetPosValid = false;
	ignoreResizeAndMove = true;

	adjustSize();

#if defined(QMC2_OS_MAC)
	macVersion = QString("macOS ") + QSysInfo::productVersion();
#elif defined(QMC2_OS_WIN)
	winVersion = QString("Windows ") + QSysInfo::productVersion();
#endif
}

void About::showEvent(QShowEvent *e)
{
	ignoreResizeAndMove = true;

	// get memory information
#if defined(QMC2_POSIX_MEMORY_INFO_ENABLED)
	quint64 numPages, pageSize, freePages, totalSize, totalUsed, totalFree;
	numPages = sysconf(_SC_PHYS_PAGES) / 1024;
	pageSize = sysconf(_SC_PAGESIZE) / 1024;
	freePages = sysconf( _SC_AVPHYS_PAGES) / 1024;
	totalSize = numPages * pageSize;
	totalFree = pageSize * freePages;
	totalUsed = totalSize - totalFree;
#elif defined(QMC2_WINDOWS_MEMORY_INFO_ENABLED)
	// FIXME
	quint64 totalSize = 0, totalUsed = 0, totalFree = 0;
	winGetMemoryInfo(&totalSize, &totalUsed, &totalFree);
#endif

#if QMC2_JOYSTICK == 1
#if SDL_MAJOR_VERSION == 1
	const SDL_version *sdlVersionLinked = SDL_Linked_Version();
#elif SDL_MAJOR_VERSION == 2
	SDL_version sdlVersionLinked;
	SDL_GetVersion(&sdlVersionLinked);
#endif
	SDL_version sdlVersionCompile;
	SDL_VERSION(&sdlVersionCompile);
#endif

	int numLogicalCores = QThread::idealThreadCount();

	QString titleString = "<p><font size=\"+1\"><b>QMC2 - M.A.M.E. Catalog / Launcher II</b></font><br>" +
		tr("Qt based multi-platform/multi-emulator front end") + "<br>" +
		tr("Version ") + QString(XSTR(QMC2_VERSION)) +
#if defined(QMC2_GIT_REV)
		" (" + tr("GIT %1").arg(XSTR(QMC2_GIT_REV)) + ")" +
#endif
		", " + tr("built for") + " " + QMC2_EMU_NAME_VARIANT + "<br>" + tr("Copyright") + " &copy; 2006 - 2022 R. Reucher, " + tr("Germany") + "</p>";

	labelTitle->setText(titleString);

	QString projectInfoString =
		"<p><b>" + tr("Project homepage:") + "</b><br><a href=\"https://qmc2.batcom-it.net/\">https://qmc2.batcom-it.net/</a></p>" +
		"<p><b>" + tr("Development site:") + "</b><br><a href=\"https://github.com/qmc2/qmc2-mame-fe\">https://github.com/qmc2/qmc2-mame-fe</a></p>" +
		"<p><b>" + tr("QMC2 development mailing list:") + "</b><br>qmc2-devel@lists.sourceforge.net (" + tr("subscription required") +")</p>" +
		"<p><b>" + tr("List subscription:") + "</b><br><a href=\"https://lists.sourceforge.net/lists/listinfo/qmc2-devel\">https://lists.sourceforge.net/lists/listinfo/qmc2-devel</a></p>" +
		"<p><b>" + tr("Bug tracking system:") + "</b><br><a href=\"https://tracker.batcom-it.net/view_all_bug_page.php?project_id=1\">https://tracker.batcom-it.net/view_all_bug_page.php?project_id=1</a></p>";

	labelProjectInfo->setText(projectInfoString);

	QString libPaths;
	foreach (QString path, QCoreApplication::libraryPaths())
		libPaths += "<br>" + path;

	QString sysInfoString = QString("<html><body>") +
#if defined(QMC2_OS_UNIX)
		"<p><b>" + tr("Build OS:") + "</b><br>" + QMC2_OS_NAME + " " + QMC2_OS_RELEASE + " " + QMC2_MACHINE_ARCHITECTURE + "</p>" +
#elif defined(QMC2_OS_MAC)
		"<p><b>" + tr("Running OS:") + "</b><br>" + macVersion + "</p>" +
#elif defined(QMC2_OS_WIN)
		"<p><b>" + tr("Running OS:") + "</b><br>" + winVersion + "</p>" +
#endif
		"<p><b>" + tr("Emulator version:") + "</b><br>" + qmc2MachineList->emulatorVersion + "</p>" +
		"<p><b>" + tr("Template information:") + "</b><br>" + tr("Emulator:") + " " + qmc2GlobalEmulatorOptions->templateEmulator + "<br>" + tr("Version:") + " " + qmc2GlobalEmulatorOptions->templateVersion + "<br>" + tr("Format:") + " " + qmc2GlobalEmulatorOptions->templateFormat + "</p>" +
		"<p><b>" + tr("Qt version:") + "</b><br>" + tr("Compile-time:") + " " + QT_VERSION_STR + "<br>" + tr("Run-time:") + " " + qVersion() +
#if QT_VERSION >= 0x050000
		"</p>" +
#else
		"<br>" + tr("Build key:") + " " + QLibraryInfo::buildKey() + "</p>" +
#endif
		"<p><b>" + tr("Qt library paths:") + "</b>" + libPaths + "</p>";
#if QMC2_JOYSTICK == 1
#if SDL_MAJOR_VERSION == 1
	sysInfoString += "<p><b>" + tr("SDL version:") + "</b><br>" + tr("Compile-time:") + " " + QString("%1.%2.%3").arg(sdlVersionCompile.major).arg(sdlVersionCompile.minor).arg(sdlVersionCompile.patch) + "<br>" + tr("Run-time:") + " " + QString("%1.%2.%3").arg(sdlVersionLinked->major).arg(sdlVersionLinked->minor).arg(sdlVersionLinked->patch) + "</p>";
#elif SDL_MAJOR_VERSION == 2
	sysInfoString += "<p><b>" + tr("SDL version:") + "</b><br>" + tr("Compile-time:") + " " + QString("%1.%2.%3").arg(sdlVersionCompile.major).arg(sdlVersionCompile.minor).arg(sdlVersionCompile.patch) + "<br>" + tr("Run-time:") + " " + QString("%1.%2.%3").arg(sdlVersionLinked.major).arg(sdlVersionLinked.minor).arg(sdlVersionLinked.patch) + "</p>";
#endif
#endif
#if QMC2_USE_PHONON_API
	sysInfoString += "<p><b>" + tr("Phonon version:") + "</b><br>" + tr("Run-time:") + " " + QString("%1").arg(Phonon::phononVersion()) + "</p><p><b>" + tr("Phonon backend / supported MIME types:") + "</b>";
	QStringList mimeTypes = Phonon::BackendCapabilities::availableMimeTypes();
	mimeTypes.removeDuplicates();
	mimeTypes.sort();
	foreach (QString mimeType, mimeTypes)
		sysInfoString += "<br>" + mimeType;
	sysInfoString += QString("</p>");
#endif
#if defined(QMC2_POSIX_MEMORY_INFO_ENABLED) || defined(QMC2_WINDOWS_MEMORY_INFO_ENABLED)
	sysInfoString += "<p><b>" + tr("Physical memory:") + "</b><br>" + tr("Total: %1 MB").arg(totalSize) + "<br>" + tr("Free: %1 MB").arg(totalFree) + "<br>" + tr("Used: %1 MB").arg(totalUsed) + "</p>";
#endif
	sysInfoString += "<p><b>" + tr("Number of CPUs:") + "</b><br>" + (numLogicalCores != -1 ? QString("%1").arg(numLogicalCores) : tr("unknown")) + "</p><p><b>" + tr("Environment variables:") + "</b>";
	QStringList envVars = QProcess::systemEnvironment();
	envVars.sort();
	foreach (QString ev, envVars)
		sysInfoString += "<br>" + ev;
	sysInfoString += "</p></body></html>";

	textBrowserSystemInformation->setHtml(sysInfoString);

	// ensure a black background for the logo image
	labelLogoPixmap->setPalette(QPalette(QColor(0, 0, 0)));

	if ( widgetSize.isValid() )
		resize(widgetSize);

	if ( widgetPosValid )
		move(widgetPos);

	ignoreResizeAndMove = false;

	e->accept();
}

void About::resizeEvent(QResizeEvent *e)
{
	if ( !ignoreResizeAndMove )
		widgetSize = size();
	e->accept();
}

void About::moveEvent(QMoveEvent *e)
{
	if ( !ignoreResizeAndMove ) {
		widgetPos = pos();
		widgetPosValid = true;
	}
	e->accept();
}
