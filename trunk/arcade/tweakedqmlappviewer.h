#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include <QTimer>
#include <QMap>

#include "qmlapplicationviewer.h"
#include "processmanager.h"
#include "imageprovider.h"
#include "infoprovider.h"

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

private:
    bool initialised;

protected:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
};

#endif
