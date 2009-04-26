#include "arcade/arcadeview.h"
#include "arcade/arcadesettings.h"
#include "qmc2main.h"
#include "options.h"

extern MainWindow *qmc2MainWindow;

ArcadeSettings::ArcadeSettings(QObject *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeSettings::ArcadeSettings(QObject *parent = %1)").arg((qulonglong)parent));
#endif

  // initialize all settings

  // miscellaneous settings
  setMiscellaneousShowFPS(miscellaneousShowFPS());
  setMiscellaneousConsoleBufferSize(miscellaneousConsoleBufferSize());
  setMiscellaneousFont(miscellaneousFont());

  // file/path settings
  setPathsScreenshotDir(pathsScreenshotDir());

  // general graphics settings
  setGraphicsUseOpenGL(graphicsUseOpenGL());
  setGraphicsFullscreen(graphicsFullscreen());
  setGraphicsWindowSize(graphicsWindowSize());
  setGraphicsWindowPosition(graphicsWindowPosition());
  setGraphicsCenterOnScreen(graphicsCenterOnScreen());
  setGraphicsSwitchResolution(graphicsSwitchResolution());
  setGraphicsPrimitiveAntiAliasing(graphicsPrimitiveAntiAliasing());
  setGraphicsKeepAspect(graphicsKeepAspect());
  setGraphicsAspectRatio(graphicsAspectRatio());
  setGraphicsVirtualResolution(graphicsVirtualResolution());
  setGraphicsUseWindowResolutionInFullScreenMode(graphicsUseWindowResolutionInFullScreenMode());
  setGraphicsRotationAngle(graphicsRotationAngle());

  // quality settings
  setQualitySmoothItemScaling(qualitySmoothItemScaling());

  // OpenGL settings
#if QMC2_ARCADE_OPENGL == 1
  setOpenGLSyncToScreen(openGLSyncToScreen());
  setOpenGLDoubleBuffer(openGLDoubleBuffer());
  setOpenGLDepthBuffer(openGLDepthBuffer());
  setOpenGLRGBA(openGLRGBA());
  setOpenGLAlphaChannel(openGLAlphaChannel());
  setOpenGLAccumulatorBuffer(openGLAccumulatorBuffer());
  setOpenGLStencilBuffer(openGLStencilBuffer());
  setOpenGLStereo(openGLStereo());
  setOpenGLDirectRendering(openGLDirectRendering());
  setOpenGLOverlay(openGLOverlay());
  setOpenGLMultiSample(openGLMultiSample());
  setOpenGLAntiAliasing(openGLAntiAliasing());
#endif
}

ArcadeSettings::~ArcadeSettings()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeSettings::~ArcadeSettings()");
#endif

}
