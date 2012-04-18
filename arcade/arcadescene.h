#ifndef _ARCADESCENE_H
#define _ARCADESCENE_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QProgressBar>
#endif

#define QMC2_SCENE_FRAMECOUNTER_TIMEOUT		2000
#define QMC2_SCENE_BOTTOM_Z			1
#define QMC2_SCENE_TOP_Z			100
#define QMC2_SCENE_FOREGROUND_Z			200
#define QMC2_SCENE_LOG_Z			300
#define QMC2_SCENE_CONSOLE_Z			400

class ArcadeScene : public QGraphicsScene
{
  Q_OBJECT

  public:
    QGraphicsSimpleTextItem *fpsTextItem;
    QGraphicsRectItem *fpsBackgroundItem;
    QGraphicsSimpleTextItem *statusTextItem;
    QGraphicsRectItem *statusBackgroundItem;
    QProgressBar *progressBar;
    QGraphicsProxyWidget *progressBarProxy;
    QList<QString> messageList;
    QTimer frameCounterTimer;
    ulong frames;
    QTime frameTime;
    QFont sceneFont;
    double scaleFactorX;
    double scaleFactorY;
    double centerX;
    double centerY;
    bool animationPaused;
    bool fpsShown;
    bool consoleShown;
    QRectF sceneClipRect;

    ArcadeScene(QObject *parent = 0);
    ~ArcadeScene();

    virtual void toggleConsole();
    virtual void startAnimation() { animationPaused = false; }
    virtual void stopAnimation() { animationPaused = true; }
    virtual void toggleAnimation();
    virtual void showFps();
    virtual void hideFps();
    virtual void toggleFps();
    virtual void rescaleContent();

    QPointF scalePoint(QPointF p) { return QPointF(p.x() * scaleFactorX, p.y() * scaleFactorY); }
    double scaleX(double x) { return scaleFactorX * x; }
    double scaleY(double y) { return scaleFactorY * y; }

  public slots:
    void updateFrameCounters();
    void hasChanged(const QList<QRectF> &);
    void updateScene();
    void setStatus(QString);
    void clearStatus(QString message = QString());
    void setProgressRange(int from, int to) { progressBar->setRange(from, to); }
    void setProgress(int);
    void clearProgress() { setProgress(-1); }
    void resizeScene(QSize);
    void setupWindowedMode();
    void setupFullscreenMode();

  protected:
    virtual void drawBackground(QPainter *, const QRectF &);
    virtual void drawForeground(QPainter *, const QRectF &);
    virtual void drawItems(QPainter *, int, QGraphicsItem *[], const QStyleOptionGraphicsItem [], QWidget *);
};

#endif
