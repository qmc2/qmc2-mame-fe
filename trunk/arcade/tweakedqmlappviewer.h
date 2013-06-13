#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include <QTimer>
#include <QStringList>
#include <QMap>

#include "qmlapplicationviewer.h"
#include "processmanager.h"
#include "imageprovider.h"
#include "infoprovider.h"

#define QMC2_ARCADE_PARAM_THEME     0
#define QMC2_ARCADE_PARAM_GRASYS    1
#define QMC2_ARCADE_PARAM_CONSOLE   2
#define QMC2_ARCADE_PARAM_LANGUAGE  3

class TweakedQmlApplicationViewer : public QmlApplicationViewer
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

    explicit TweakedQmlApplicationViewer(QWidget *parent = 0);
    virtual ~TweakedQmlApplicationViewer();

signals:
    void emulatorStarted(int);
    void emulatorFinished(int);

public slots:
    void fpsReady();
    void loadSettings();
    void saveSettings();
    void goFullScreen();
    void switchToFullScreen(bool initially = false);
    void switchToWindowed(bool initially = false);
    QString romStateText(int);
    int romStateCharToInt(char);
    void loadGamelist();
    void launchEmulator(QString);
    QString loadImage(const QString &);
    QString requestInfo(const QString&, const QString&);
    int findIndex(QString, int startIndex = 0);
    void log(QString);
    QStringList cliParamNames();
    QString cliParamDescription(QString);
    QString cliParamValue(QString);
    QStringList cliParamAllowedValues(QString);
    void setCliParamValue(QString, QString);

private:
    bool initialised;

protected:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
};

#endif
