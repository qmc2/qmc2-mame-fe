#include "detailsetup.h"
#include "macros.h"

#define QMC2_DEBUG

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

DetailSetup::DetailSetup(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DetailSetup::DetailSetup(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the game/machine detail setup isn't working yet!"));
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Previe&w");
  longTitleMap[QMC2_PREVIEW_INDEX] = tr("Game preview image");
  shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
  longTitleMap[QMC2_FLYER_INDEX] = tr("Game flyer image");
  shortTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game &info");
  longTitleMap[QMC2_GAMEINFO_INDEX] = tr("Game information");
  shortTitleMap[QMC2_EMUINFO_INDEX] = tr("Em&ulator info");
  longTitleMap[QMC2_EMUINFO_INDEX] = tr("Emulator information");
  shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
  longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
  shortTitleMap[QMC2_CABINET_INDEX] = tr("Ca&binet");
  longTitleMap[QMC2_CABINET_INDEX] = tr("Arcade cabinet image");
  shortTitleMap[QMC2_CONTROLLER_INDEX] = tr("C&ontroller");
  longTitleMap[QMC2_CONTROLLER_INDEX] = tr("Control panel image");
  shortTitleMap[QMC2_MARQUEE_INDEX] = tr("&Marquee");
  longTitleMap[QMC2_MARQUEE_INDEX] = tr("Marquee image");
  shortTitleMap[QMC2_TITLE_INDEX] = tr("Titl&e");
  longTitleMap[QMC2_TITLE_INDEX] = tr("Title screen image");
  activeDetailList << QMC2_PREVIEW_INDEX
                   << QMC2_FLYER_INDEX
                   << QMC2_GAMEINFO_INDEX
                   << QMC2_EMUINFO_INDEX
                   << QMC2_CONFIG_INDEX
                   << QMC2_CABINET_INDEX
                   << QMC2_CONTROLLER_INDEX
                   << QMC2_MARQUEE_INDEX
                   << QMC2_TITLE_INDEX;
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  shortTitleMap[QMC2_PREVIEW_INDEX] = tr("Previe&w");
  longTitleMap[QMC2_PREVIEW_INDEX] = tr("Machine preview image");
  shortTitleMap[QMC2_FLYER_INDEX] = tr("Fl&yer");
  longTitleMap[QMC2_FLYER_INDEX] = tr("Machine flyer image");
  shortTitleMap[QMC2_MACHINEINFO_INDEX] = tr("Machine &info");
  longTitleMap[QMC2_MACHINEINFO_INDEX] = tr("Machine information");
  shortTitleMap[QMC2_CONFIG_INDEX] = tr("&Configuration");
  longTitleMap[QMC2_CONFIG_INDEX] = tr("Emulator configuration");
  shortTitleMap[QMC2_DEVICE_INDEX] = tr("De&vices");
  longTitleMap[QMC2_DEVICE_INDEX] = tr("Device configuration");
  activeDetailList << QMC2_PREVIEW_INDEX
                   << QMC2_FLYER_INDEX
                   << QMC2_MACHINEINFO_INDEX
                   << QMC2_CONFIG_INDEX
                   << QMC2_DEVICE_INDEX;
#endif

  setupUi(this);

  loadDetail();

  QMapIterator<int, QString> it(longTitleMap);
  while ( it.hasNext() ) {
    it.next();
    listWidgetAvailableDetails->addItem(it.value());
  }

  int i;
  for (i = 0; i < activeDetailList.count(); i++) {
    listWidgetActiveDetails->addItem(longTitleMap[activeDetailList[i]]);
  }

  adjustSize();

  QList<int> equalSplitterSizes;
  equalSplitterSizes << 100 << 100;
//  hSplitter->setSizes(equalSplitterSizes);
}

DetailSetup::~DetailSetup()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::~DetailSetup()");
#endif

}

void DetailSetup::loadDetail()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::loadDetail()");
#endif

}

void DetailSetup::saveDetail()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::saveDetail()");
#endif

}

void DetailSetup::on_listWidgetAvailableDetails_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_listWidgetAvailableDetails_itemSelectionChanged()");
#endif

  if ( listWidgetAvailableDetails->selectedItems().count() > 0 ) {
    pushButtonActivateDetails->setEnabled(TRUE);
  } else {
    pushButtonActivateDetails->setEnabled(FALSE);
  }
}
 
void DetailSetup::on_listWidgetActiveDetails_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::on_listWidgetActiveDetails_itemSelectionChanged()");
#endif

  if ( listWidgetActiveDetails->selectedItems().count() > 0 ) {
    pushButtonDeactivateDetails->setEnabled(TRUE);
    pushButtonDetailsUp->setEnabled(TRUE);
    pushButtonDetailsDown->setEnabled(TRUE);
  } else {
    pushButtonDeactivateDetails->setEnabled(FALSE);
    pushButtonDetailsUp->setEnabled(FALSE);
    pushButtonDetailsDown->setEnabled(FALSE);
  }
}
