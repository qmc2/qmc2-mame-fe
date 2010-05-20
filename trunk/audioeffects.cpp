#include "macros.h"

#if QMC2_USE_PHONON_API == 1

#include <QSettings>
#include "audioeffects.h"
#include "qmc2main.h"

extern QSettings *qmc2Config;
extern MainWindow *qmc2MainWindow;

#define QMC2_DEBUG

AudioEffectDialog::AudioEffectDialog(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: AudioEffectDialog::AudioEffectDialog(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	ignoreHideEvent = true;
	hide();
	ignoreHideEvent = false;

	effectDescriptions = Phonon::BackendCapabilities::availableAudioEffects();
	foreach (Phonon::EffectDescription description, effectDescriptions) {
		QTreeWidgetItem *effectItem = new QTreeWidgetItem(treeWidgetAudioEffects);
		effectItem->setText(QMC2_AUDIOEFFECT_COLUMN_NAME, description.name());
		effectItem->setText(QMC2_AUDIOEFFECT_COLUMN_DESC, description.description());
		QCheckBox *effectEnabler = new QCheckBox(this);
		checkBoxItemMap[effectEnabler] = effectItem;
		effectEnablerMap[description.name()] = effectEnabler;
		connect(effectEnabler, SIGNAL(toggled(bool)), this, SLOT(checkBoxToggled(bool)));
		effectEnabler->setToolTip(tr("Enable effect '%1'").arg(description.name()));
		treeWidgetAudioEffects->setItemWidget(effectItem, QMC2_AUDIOEFFECT_COLUMN_ENABLE, effectEnabler);
		effectMap[description.name()] = new Phonon::Effect(description);
		effectWidgetMap[description.name()] = new  Phonon::EffectWidget(effectMap[description.name()]);
		if ( effectMap[description.name()]->parameters().count() > 0 ) {
			QToolButton *effectSetupButton = new QToolButton(this);
			toolButtonItemMap[effectSetupButton] = effectItem;
			connect(effectSetupButton, SIGNAL(clicked(bool)), this, SLOT(toolButtonClicked()));
			effectSetupButton->setToolTip(tr("Setup effect '%1'").arg(description.name()));
			effectSetupButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			effectSetupButton->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
			treeWidgetAudioEffects->setItemWidget(effectItem, QMC2_AUDIOEFFECT_COLUMN_SETUP, effectSetupButton);
		}
	}
	treeWidgetAudioEffects->sortItems(QMC2_AUDIOEFFECT_COLUMN_NAME, Qt::AscendingOrder);
	treeWidgetAudioEffects->resizeColumnToContents(QMC2_AUDIOEFFECT_COLUMN_ENABLE);
	treeWidgetAudioEffects->resizeColumnToContents(QMC2_AUDIOEFFECT_COLUMN_SETUP);

	QStringList enabledEffects = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EnabledEffects").toStringList();
	QStringList validatedEffects;
	foreach (QString effectName, enabledEffects) {
		QCheckBox *checkBox = effectEnablerMap[effectName];
		if ( checkBox ) {
			checkBox->setChecked(true);
			validatedEffects << effectName;
		}
	}
	if ( validatedEffects.count() > 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EnabledEffects", validatedEffects);
	else
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EnabledEffects");
}

AudioEffectDialog::~AudioEffectDialog()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: AudioEffectDialog::~AudioEffectDialog()");
#endif

	saveEffectSettings();

	foreach (Phonon::EffectWidget *widget, effectWidgetMap)
		if ( widget ) {
			widget->close();
			delete widget;
		}

	foreach (Phonon::Effect *effect, effectMap)
		if ( effect )
			delete effect;
}

void AudioEffectDialog::saveEffectSettings()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: AudioEffectDialog::saveEffectSettings()");
#endif

	QStringList enabledEffects;
	
	foreach (QCheckBox *checkBox, effectEnablerMap) {
		if ( checkBox )
			if ( checkBox->isChecked() ) {
				QTreeWidgetItem *item = checkBoxItemMap[checkBox];
				if ( item )
					enabledEffects << item->text(QMC2_AUDIOEFFECT_COLUMN_NAME);
			}
	}

	if ( enabledEffects.count() > 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EnabledEffects", enabledEffects);
	else
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EnabledEffects");
}

void AudioEffectDialog::toolButtonClicked()
{
	QToolButton *toolButton = (QToolButton *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: AudioEffectDialog::toolButtonClicked()");
#endif

	if ( !toolButton )
		return;

	QTreeWidgetItem *item = toolButtonItemMap[toolButton];

	if ( !item )
		return;

	QString effectName = item->text(QMC2_AUDIOEFFECT_COLUMN_NAME);

	if ( effectName.isEmpty() )
		return;

	Phonon::EffectWidget *widget = effectWidgetMap[effectName];

	if ( widget ) {
		widget->move(mapToGlobal(toolButton->frameGeometry().center()));
		widget->show();
	}
}

void AudioEffectDialog::checkBoxToggled(bool checked)
{
	QCheckBox *checkBox = (QCheckBox *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: AudioEffectDialog::checkBoxToggled(bool checked = %1)").arg(checked));
#endif

	if ( !checkBox )
		return;

	QTreeWidgetItem *item = checkBoxItemMap[checkBox];

	if ( !item )
		return;

	QString effectName = item->text(QMC2_AUDIOEFFECT_COLUMN_NAME);

	if ( effectName.isEmpty() )
		return;

	Phonon::Effect *effect = effectMap[effectName];

	if ( !effect )
		return;

	if ( checked ) {
		if ( qmc2MainWindow->phononAudioPath.insertEffect(effect) ) {
			checkBox->setToolTip(tr("Disable effect '%1'").arg(effectName));
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: audio player: effect '%1' successfully inserted").arg(effectName));
#endif
		} else {
			checkBox->blockSignals(true);
			checkBox->setChecked(false);
			checkBox->blockSignals(false);
			checkBox->setToolTip(tr("Enable effect '%1'").arg(effectName));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: audio player: can't insert effect '%1'").arg(effectName));
		}
	} else {
		if ( qmc2MainWindow->phononAudioPath.removeEffect(effect) ) {
			checkBox->setToolTip(tr("Enable effect '%1'").arg(effectName));
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: audio player: effect '%1' successfully removed").arg(effectName));
#endif
		} else {
			checkBox->blockSignals(true);
			checkBox->setChecked(true);
			checkBox->blockSignals(false);
			checkBox->setToolTip(tr("Disable effect '%1'").arg(effectName));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: audio player: can't remove effect '%1'").arg(effectName));
		}
	}

	saveEffectSettings();
}

void AudioEffectDialog::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: AudioEffectDialog::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	e->accept();
}

void AudioEffectDialog::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: AudioEffectDialog::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
		restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/Geometry").toByteArray());
		treeWidgetAudioEffects->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EffectListHeaderState").toByteArray());
	}

	e->accept();
}

void AudioEffectDialog::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: AudioEffectDialog::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( ignoreHideEvent ) {
		e->accept();
		return;
	}

	foreach (Phonon::EffectWidget *widget, effectWidgetMap)
		if ( widget )
			widget->hide();

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/Geometry", saveGeometry());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/AudioEffectDialog/EffectListHeaderState", treeWidgetAudioEffects->header()->saveState());
	}

	e->accept();
}

#endif
