#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QHash>
#include <QQuickView>
#include <QWindow>
#include <QByteArray>

#include "processmanager.h"
#include "imageprovider.h"
#include "infoprovider.h"
#include "keysequencemap.h"
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
#include "joyfunctionmap.h"
#include "joystickmanager.h"
#endif

#define QMC2_ARCADE_PARAM_THEME     0
#define QMC2_ARCADE_PARAM_CONSOLE   1
#define QMC2_ARCADE_PARAM_LANGUAGE  2
#define QMC2_ARCADE_PARAM_VIDEO     3

class TweakedQmlApplicationViewer : public QQuickView
{
	Q_OBJECT

public:
	int numFrames;
	QTimer frameCheckTimer;
	QByteArray savedGeometry;
	bool savedMaximized;
	QList<QObject *> gameList;
	ProcessManager *processManager;
	ImageProvider *imageProvider;
	InfoProvider *infoProvider;
	bool windowModeSwitching;
	QMap<QString, QStringList> cliAllowedParameterValues;
	QMap<QString, QString> cliParameterDescriptions;
	QStringList cliParams;
	QStringList infoClasses;
	QStringList videoSnapAllowedFormatExtensions;
	KeySequenceMap *keySequenceMap;
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
	JoyFunctionMap *joyFunctionMap;
	JoystickManager *joystickManager;
#endif
	static int consoleMode;

	explicit TweakedQmlApplicationViewer(QWindow *parent = 0);
	virtual ~TweakedQmlApplicationViewer();

	bool isFullScreen()
	{
		return windowState() & Qt::WindowFullScreen;
	}
	bool isMaximized()
	{
		return windowState() & Qt::WindowMaximized;
	}
	QByteArray saveGeometry()
	{
		// FIXME
		return QByteArray();
	}
	void restoreGeometry(const QByteArray &geom)
	{
		// FIXME
	}

	int themeIndex();
	void setVideoEnabled(bool enable) { m_videoEnabled = enable; }
	bool videoEnabled() { return m_videoEnabled; }
	void setInitialFullScreen(bool enable) { m_initialFullScreen = enable; }
	bool initialFullScreen() { return m_initialFullScreen; }

	// logging functions
	static void logString(const QString &s);
	static void logStringNoTime(const QString &s);
	static void logCString(const char *s);
	static void logCStringNoTime(const char *s);

signals:
	void emulatorStarted(int);
	void emulatorFinished(int);
	void imageDataUpdated(const QString & cachePrefix);

public slots:
	void displayInit();
	void fpsReady();
	void loadSettings();
	void saveSettings();
	void switchToFullScreen(bool initially = false);
	void switchToWindowed(bool initially = false);
	QString romStateText(int);
	int romStateCharToInt(char);
	void loadMachineList();
	void launchEmulator(QString);
	QString loadImage(const QString &);
	QString requestInfo(const QString &, const QString &);
	QString videoSnapUrl(const QString &);
	int findIndex(QString, int startIndex = 0);
	void log(QString);
	QStringList cliParamNames();
	QString cliParamDescription(QString);
	QString cliParamValue(QString);
	QStringList cliParamAllowedValues(QString);
	void setCliParamValue(QString, QString);
	void linkActivated(QString);
	QString emuMode();
	void frameBufferSwapped() { numFrames++; }
	void handleQuit();
	int runningEmulators() { return processManager->runningProcesses(); }
	void imageDataUpdate(const QString &cachePrefix) { emit imageDataUpdated(cachePrefix); }
	bool isSevenZippedImageType(const QString & type) { return imageProvider->isSevenZippedImageType(type); }
	bool isZippedImageType(const QString & type) { return imageProvider->isZippedImageType(type); }
	bool iconCacheDatabaseEnabled();
	QString parentId(QString id);
	QStringList customSystemArtwork();
	QStringList customSoftwareArtwork();
	QString nextCustomSytemArtwork();
	QString previousCustomSytemArtwork();
	QString nextCustomSoftwareArtwork();
	QString previousCustomSoftwareArtwork();
	QString customCachePrefix(const QString &name) { return imageProvider->customCachePrefix(name); }

private:
	bool m_initialized;
	bool m_videoEnabled;
	bool m_initialFullScreen;
	QHash<QString, QString> m_parentHash;
	QHash<QString, QString> m_videoSnapUrlCache;
	int m_currentSystemArtworkIndex;
	int m_currentSoftwareArtworkIndex;
};

#endif
