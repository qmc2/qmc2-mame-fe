#include <QMutex>
#include <QImage>
#include <QPainter>
#include <QDir>
#include <QString>
#include "arcade/arcadeview.h"
#include "arcade/arcadescene.h"
#include "arcade/arcadesettings.h"
#include "arcade/arcadescreenshotsaverthread.h"
#include "qmc2main.h"
#include "macros.h"

// extern global variables
extern MainWindow *qmc2MainWindow;
extern bool exitArcade;
extern QWaitCondition arcadeScreenshotSaverWaitCondition;
extern ArcadeView *arcadeView;
extern ArcadeSettings *arcadeSettings;
extern QImage *arcadeScreenshotImage;
extern QMutex arcadeScreenshotMutex;

ArcadeScreenshotSaverThread::ArcadeScreenshotSaverThread(QObject *parent)
  : QThread(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScreenshotSaverThread::ArcadeScreenshotSaverThread(QObject *parent = %1)").arg((qulonglong) parent));
#endif

  start();
}

ArcadeScreenshotSaverThread::~ArcadeScreenshotSaverThread()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScreenshotSaverThread::~ArcadeScreenshotSaverThread()");
#endif

  quit();
  wait();
}

void ArcadeScreenshotSaverThread::run()
{
#ifdef QMC2_DEBUG
  emit log("DEBUG: ArcadeScreenshotSaverThread::run()");
#endif

  emit log(tr("ArcadeScreenshotSaverThread: Started"));

  while ( !exitArcade ) {
#ifdef QMC2_DEBUG
    emit log("DEBUG: ArcadeScreenshotSaverThread: Going to sleep");
#endif

    screenshotSaverMutex.lock();
    arcadeScreenshotSaverWaitCondition.wait(&screenshotSaverMutex);
    screenshotSaverMutex.unlock();

#ifdef QMC2_DEBUG
    emit log("DEBUG: ArcadeScreenshotSaverThread: Woke up");
#endif

    arcadeScreenshotMutex.lock();
    if ( arcadeScreenshotImage ) {
      if ( !exitArcade ) {
        emit log(tr("ArcadeScreenshotSaverThread: Saving screen shot"));

        QDir screenshotDir(arcadeSettings->pathsScreenshotDir());
        if ( !screenshotDir.exists() ) {
          if ( !screenshotDir.mkpath(screenshotDir.absolutePath()) ) {
            emit log(tr("ArcadeScreenshotSaverThread: Failed to create screen shot directory '%1' - aborting screen shot creation").arg(screenshotDir.absolutePath()));
            arcadeScreenshotMutex.unlock();
            continue;
          }
        }

        QString screenShotfile(arcadeSettings->pathsScreenshotDir() + "/screenshot-" + QDate::currentDate().toString("yyyyMMdd-") + QTime::currentTime().toString("hhmmsszzz") + ".png");

        if ( arcadeScreenshotImage->save(screenShotfile) )
          emit log(tr("ArcadeScreenshotSaverThread: Screen shot successfully saved as '%1'").arg(screenShotfile));
        else
          emit log(tr("ArcadeScreenshotSaverThread: Failed to save screen shot as '%1'").arg(screenShotfile));
        ((ArcadeScene *)(arcadeView->scene()))->clearStatus(tr("Saving screen shot"));
      }

      delete arcadeScreenshotImage;
      arcadeScreenshotImage = NULL;
    }
    arcadeScreenshotMutex.unlock();
  }

  emit log(tr("ArcadeScreenshotSaverThread: Ended"));
}
