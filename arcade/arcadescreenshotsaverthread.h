#ifndef _ARCADESCREENSHOTSAVERTHREAD_H_
#define _ARCADESCREENSHOTSAVERTHREAD_H_

#include <QThread>
#include <QMutex>

class ArcadeScreenshotSaverThread : public QThread
{
  Q_OBJECT

  public:
    QMutex screenshotSaverMutex;

    ArcadeScreenshotSaverThread(QObject *parent = 0);
    ~ArcadeScreenshotSaverThread();

  protected:
    void run();

  signals:
    void log(const QString &);
};

#endif
