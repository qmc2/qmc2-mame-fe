#include "macros.h"

#if QMC2_USE_PHONON_API == 1

#ifndef AUDIOEFFECTS_H
#define AUDIOEFFECTS_H

#include <QMap>
#include <QString>
#include <QToolButton>
#include <QCheckBox>
#include "qmc2_phonon.h"
#include "ui_audioeffects.h"

class AudioEffectDialog : public QDialog, public Ui::AudioEffectDialog
{
	Q_OBJECT

	public:
		bool ignoreHideEvent;
		QList<Phonon::EffectDescription> effectDescriptions;
		QMap<QString, Phonon::Effect *> effectMap;
		QMap<QString, Phonon::EffectWidget *> effectWidgetMap;
		QMap<QString, QCheckBox *> effectEnablerMap;
		QMap<QString, QToolButton *> effectSetupButtonMap;
		QMap<QToolButton *, QTreeWidgetItem *> toolButtonItemMap;
		QMap<QCheckBox *, QTreeWidgetItem *> checkBoxItemMap;

		AudioEffectDialog(QWidget *parent = 0);
		~AudioEffectDialog();

	public slots:
		void toolButtonClicked();
		void checkBoxToggled(bool);
		void saveEffectSettings();
		void adjustIconSizes();

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif

#endif
