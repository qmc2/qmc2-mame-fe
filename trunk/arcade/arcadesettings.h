#ifndef _ARCADESETTINGS_H_
#define _ARCADESETTINGS_H_

#if defined(QT_NO_OPENGL)
#undef QMC2_ARCADE_OPENGL
#define QMC2_ARCADE_OPENGL	0
#endif

#include <QSettings>
#include <QStringList>
#include <QApplication>
#include <QFont>
#include <QDir>
#include <QPoint>
#include <QSize>
#include "macros.h"

extern QSettings *qmc2Config;

class ArcadeSettings : public QObject
{
  Q_OBJECT

  public:
    ArcadeSettings(QObject *parent = 0);
    ~ArcadeSettings();

    // miscellaneous settings
    bool miscellaneousShowFPS() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Miscellaneous/ShowFPS", false).toBool(); }
    void setMiscellaneousShowFPS(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Miscellaneous/ShowFPS", enable); }
    int miscellaneousConsoleBufferSize() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Miscellaneous/ConsoleBufferSize", 1000).toInt(); }
    void setMiscellaneousConsoleBufferSize(int size) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Miscellaneous/ConsoleBufferSize", size); }
    QString miscellaneousFont() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Miscellaneous/Font", qApp->font().toString()).toString(); }
    void setMiscellaneousFont(QString font) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Miscellaneous/Font", font); }
    
    // file/path settings
    QString pathsScreenshotDir() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Paths/ScreenshotDir", QMC2_DOT_PATH + "/arcadeshots").toString(); }
    void setPathsScreenshotDir(QString path) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Paths/ScreenshotDir", path); }

    // general graphics settings
#if QMC2_ARCADE_OPENGL == 1
    bool graphicsUseOpenGL() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/UseOpenGL", false).toBool(); }
#else
    bool graphicsUseOpenGL() { return false; }
#endif
    void setGraphicsUseOpenGL(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/UseOpenGL", enable); }
    bool graphicsFullscreen() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/Fullscreen", false).toBool(); }
    void setGraphicsFullscreen(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/Fullscreen", enable); }
    QSize graphicsWindowSize() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/WindowSize", QSize(640, 480)).toSize(); }
    void setGraphicsWindowSize(QSize size) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/WindowSize", size); }
    QPoint graphicsWindowPosition() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/WindowPosition", QPoint(0, 0)).toPoint(); }
    void setGraphicsWindowPosition(QPoint point) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/WindowPosition", point); }
    bool graphicsSwitchResolution() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/SwitchResolution", false).toBool(); }
    void setGraphicsSwitchResolution(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/SwitchResolution", enable); }
    bool graphicsCenterOnScreen() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/CenterOnScreen", true).toBool(); }
    void setGraphicsCenterOnScreen(bool center) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/CenterOnScreen", center); }
    bool graphicsPrimitiveAntiAliasing() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/PrimitiveAntiAliasing", true).toBool(); }
    void setGraphicsPrimitiveAntiAliasing(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/PrimitiveAntiAliasing", enable); }
    bool graphicsKeepAspect() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/KeepAspect", true).toBool(); }
    void setGraphicsKeepAspect(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/KeepAspect", enable); }
    QSize graphicsAspectRatio() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/AspectRatio", QSize(4, 3)).toSize(); }
    void setGraphicsAspectRatio(QSize ratio) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/AspectRatio", ratio); }
    QSize graphicsVirtualResolution() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/VirtualResolution", QSize(1600, 1200)).toSize(); }
    void setGraphicsVirtualResolution(QSize resolution) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/VirtualResolution", resolution); }
    bool graphicsUseWindowResolutionInFullScreenMode() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/UseWindowResolutionInFullScreenMode", false).toBool(); }
    void setGraphicsUseWindowResolutionInFullScreenMode(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/UseWindowResolutionInFullScreenMode", enable); }
    double graphicsRotationAngle() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Graphics/RotationAngle", 0).toDouble(); }
    void setGraphicsRotationAngle(double angle) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/RotationAngle", angle); }

    // quality settings
    bool qualitySmoothItemScaling() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "Quality/SmoothItemScaling", false).toBool(); }
    void setQualitySmoothItemScaling(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Quality/SmoothItemScaling", enable); }

    // OpenGL settings
#if QMC2_ARCADE_OPENGL == 1
    bool openGLSyncToScreen() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/SyncToScreen", true).toBool(); }
    void setOpenGLSyncToScreen(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/SyncToScreen", enable); }
    bool openGLDoubleBuffer() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/DoubleBuffer", true).toBool(); }
    void setOpenGLDoubleBuffer(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/DoubleBuffer", enable); }
    bool openGLDepthBuffer() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/DepthBuffer", true).toBool(); }
    void setOpenGLDepthBuffer(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/DepthBuffer", enable); }
    bool openGLRGBA() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/RGBA", true).toBool(); }
    void setOpenGLRGBA(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/RGBA", enable); }
    bool openGLAlphaChannel() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/AlphaChannel", false).toBool(); }
    void setOpenGLAlphaChannel(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/AlphaChannel", enable); }
    bool openGLAccumulatorBuffer() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/AccumulatorBuffer", false).toBool(); }
    void setOpenGLAccumulatorBuffer(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/AccumulatorBuffer", enable); }
    bool openGLStencilBuffer() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/StencilBuffer", false).toBool(); }
    void setOpenGLStencilBuffer(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/StencilBuffer", enable); }
    bool openGLStereo() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/Stereo", false).toBool(); }
    void setOpenGLStereo(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/Stereo", enable); }
    bool openGLDirectRendering() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/DirectRendering", true).toBool(); }
    void setOpenGLDirectRendering(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/DirectRendering", enable); }
    bool openGLOverlay() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/Overlay", false).toBool(); }
    void setOpenGLOverlay(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/Overlay", enable); }
    bool openGLMultiSample() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/MultiSample", false).toBool(); }
    void setOpenGLMultiSample(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/MultiSample", enable); }
    bool openGLAntiAliasing() { return qmc2Config->value(QMC2_ARCADE_PREFIX + "OpenGL/AntiAliasing", false).toBool(); }
    void setOpenGLAntiAliasing(bool enable) { qmc2Config->setValue(QMC2_ARCADE_PREFIX + "OpenGL/AntiAliasing", enable); }
#endif
};


#endif
