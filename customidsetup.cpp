#include "customidsetup.h"
#include "macros.h"

extern QSettings *qmc2Config;

CustomIDSetup::CustomIDSetup(QString foreignEmulatorName, QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::CustomIDSetup(QString foreignEmulatorName = %1, QWidget *parent = %2)").arg(foreignEmulatorName).arg((qulonglong) parent));
#endif

	setupUi(this);
	adjustFontAndIconSizes();
	foreignEmulator = foreignEmulatorName;
	setWindowTitle(tr("Setup custom IDs for '%1'").arg(foreignEmulator));

	QSize widgetSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/Size").toSize();
	if ( !widgetSize.isEmpty() )
		resize(widgetSize);
}

CustomIDSetup::~CustomIDSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::~CustomIDSetup()");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/Size", size());
}

void CustomIDSetup::adjustFontAndIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::adjustFontAndIconSizes()");
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	setFont(f);
	QFontMetrics fm(f);
	QSize iconSize(fm.height() + 2, fm.height() + 2);
	toolButtonAddID->setIconSize(iconSize);
	toolButtonRemoveID->setIconSize(iconSize);
	toolButtonCopyIDs->setIconSize(iconSize);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
}
