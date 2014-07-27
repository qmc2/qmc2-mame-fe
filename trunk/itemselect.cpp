#include "itemselect.h"
#include "macros.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

ItemSelector::ItemSelector(QWidget *parent, QStringList &items)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemSelector::ItemSelector(QWidget *parent = %1, QStringList &items = ...)").arg((qulonglong)parent));
#endif

	setupUi(this);
	listWidgetItems->addItems(items);
}
