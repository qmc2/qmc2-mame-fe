#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include <qglobal.h>
#include <QTimer>
#include <QStringList>
#include <QMap>
#include <QHash>

#if QT_VERSION < 0x050000
#include "qmlapplicationviewer/qmlapplicationviewer.h"
#else
#include <QQuickView>
#include <QWindow>
#include <QByteArray>
#endif

#include "processmanager.h"
#include "imageprovider.h"
#include "infoprovider.h"
#include "keysequencemap.h"
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
#include "joyfunctionmap.h"
#include "joystickmanager.h"
#endif

#define QMC2_ARCADE_PARAM_THEME     0
#if QT_VERSION < 0x050000
#define QMC2_ARCADE_PARAM_GRASYS    1
#define QMC2_ARCADE_PARAM_CONSOLE   2
#define QMC2_ARCADE_PARAM_LANGUAGE  3
#define QMC2_ARCADE_PARAM_VIDEO     4
#else
#define QMC2_ARCADE_PARAM_CONSOLE   1
#define QMC2_ARCADE_PARAM_LANGUAGE  2
#define QMC2_ARCADE_PARAM_VIDEO     3
#endif

#if QT_VERSION < 0x050000
class TweakedQmlApplicationViewer : public QmlApplicationViewer
#else
class TweakedQmlApplicationViewer : public QQuickView
#endif
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

#if QT_VERSION < 0x050000
    explicit TweakedQmlApplicationViewer(QWidget *parent = 0);
#else
    explicit TweakedQmlApplicationViewer(QWindow *parent = 0);
#endif
    virtual ~TweakedQmlApplicationViewer();

#if QT_VERSION >= 0x050000
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
#endif

    int themeIndex();
    void setVideoEnabled(bool enable) { m_videoEnabled = enable; }
    bool videoEnabled() { return m_videoEnabled; }
    void setInitialFullScreen(bool enable) { m_initialFullScreen = enable; }
    bool initialFullScreen() { return m_initialFullScreen; }

signals:
    void emulatorStarted(int);
    void emulatorFinished(int);
    void imageDataUpdated(QString cachePrefix);

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
#if QT_VERSION >= 0x050000
    void frameBufferSwapped() { numFrames++; }
    void handleQuit();
#endif
    int runningEmulators() { return processManager->runningProcesses(); }
    void imageDataUpdate(QString cachePrefix) { emit imageDataUpdated(cachePrefix); }
    bool isSevenZippedImageType(QString type) { return imageProvider->isSevenZippedImageType(type); }
    bool isZippedImageType(QString type) { return imageProvider->isZippedImageType(type); }
    QString parentId(QString id);

private:
    bool m_initialized;
    bool m_videoEnabled;
    bool m_initialFullScreen;
    QHash<QString, QString> m_parentHash;
    QHash<QString, QString> m_videoSnapUrlCache;

#if QT_VERSION < 0x050000
protected:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
#endif
};

#endif
