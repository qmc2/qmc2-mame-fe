#ifndef _ARCADEVIEW_H_
#define _ARCADEVIEW_H_

#include <QtGui>
#include "macros.h"
#include "arcade/arcademenuscene.h"
#include "arcade/arcadescreenshotsaverthread.h"

#define QMC2_MAIN_SIZEADJUST_TIMEOUT	100

class ArcadeView : public QGraphicsView
{
  Q_OBJECT

  public:
    ArcadeMenuScene *menuScene;
    ArcadeScreenshotSaverThread *arcadeScreenshotSaverThread;
    QSize sizeAdjusted;

    ArcadeView(QWidget *parent = 0);
    ~ArcadeView();

  public slots:
    void setupGraphicsMode();
    void setupWindowSize(QSize);
    void setupWindowPosition(QPoint);
    void toggleFullscreen();
    void takeScreenshot();
    void rescale(QSize);
    void sizeAdjust();
    void delayedInit();
    void reconnectSceneSignals();
    void logMsg(const QString &);

  signals:
    void resized(QSize);
    void viewportAdjusted(QSize);
    void switchedToFullscreenMode();
    void switchedToWindowedMode();

  protected:
    void keyPressEvent(QKeyEvent *);
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *);
};


#endif
