#include "macros.h"

#if QMC2_USE_PHONON_API == 1

#ifndef _AUDIOEFFECTS_H
#define _AUDIOEFFECTS_H

#include "ui_audioeffects.h"

class AudioEffectDialog : public QDialog, public Ui::AudioEffectDialog
{
	Q_OBJECT

	public:
		AudioEffectDialog(QWidget *parent = 0);
		~AudioEffectDialog();
};

#endif

#endif
