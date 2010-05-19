#include "macros.h"

#if QMC2_USE_PHONON_API == 1

#include "audioeffects.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;

#define QMC2_DEBUG

AudioEffectDialog::AudioEffectDialog(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: AudioEffectDialog::AudioEffectDialog(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);
}

AudioEffectDialog::~AudioEffectDialog()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: AudioEffectDialog::~AudioEffectDialog()");
#endif

}

#endif
