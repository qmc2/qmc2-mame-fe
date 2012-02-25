#include "customidsetup.h"
#include "macros.h"

CustomIDSetup::CustomIDSetup(QString foreignEmulatorName, QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: CustomIDSetup::CustomIDSetup(QString foreignEmulatorName = %1, QWidget *parent = %2)").arg(foreignEmulatorName).arg((qulonglong) parent));
#endif

	setupUi(this);

}

CustomIDSetup::~CustomIDSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: CustomIDSetup::~CustomIDSetup()");
#endif

}
