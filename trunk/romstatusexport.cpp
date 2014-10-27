#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QClipboard>
#include <QTreeWidget>
#include <QLocale>
#include <QMap>
#include <QMultiMap>

#include "romstatusexport.h"
#include "macros.h"
#include "options.h"
#include "gamelist.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;
extern bool qmc2WidgetsEnabled;
extern Gamelist *qmc2Gamelist;
extern bool qmc2ExportingROMStatus;
extern bool qmc2StopParser;

ROMStatusExporter::ROMStatusExporter(QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::ROMStatusExporter(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	exportListAutoCorrected = false;

#if defined(QMC2_EMUTYPE_MESS)
	comboBoxSortCriteria->setItemText(QMC2_RSE_SORT_BY_DESCRIPTION, tr("Machine description"));
	comboBoxSortCriteria->setItemText(QMC2_RSE_SORT_BY_NAME, tr("Machine name"));
#endif

	pushButtonExport->setEnabled(qmc2WidgetsEnabled);

	comboBoxOutputFormat->insertSeparator(QMC2_ROMSTATUSEXPORT_FORMAT_SEP_INDEX);
	comboBoxOutputFormat->insertItem(QMC2_ROMSTATUSEXPORT_FORMAT_ALL_INDEX, tr("All formats"));

	// restore settings
	comboBoxOutputFormat->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/OutputFormat", 0).toInt());
	toolButtonExportC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportC", true).toBool());
	toolButtonExportM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportM", true).toBool());
	toolButtonExportI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportI", true).toBool());
	toolButtonExportN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportN", true).toBool());
	toolButtonExportU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportU", true).toBool());
	comboBoxSortCriteria->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/SortCriteria", 0).toInt());
	comboBoxSortOrder->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/SortOrder", 0).toInt());
	checkBoxIncludeHeader->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/IncludeHeader", true).toBool());
	checkBoxIncludeStatistics->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/IncludeStatistics", true).toBool());
	if ( comboBoxOutputFormat->currentIndex() <= QMC2_ROMSTATUSEXPORT_FORMAT_HTML_INDEX )
		checkBoxExportToClipboard->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportToClipboard", false).toBool());
	else
		checkBoxExportToClipboard->setChecked(false);
	checkBoxOverwriteBlindly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/OverwriteBlindly", false).toBool());
	lineEditASCIIFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ASCIIFile", "data/tmp/rom-status.txt").toString());
	spinBoxASCIIColumnWidth->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ASCIIColumnWidth", 0).toInt());
	lineEditCSVFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/CSVFile", "data/tmp/rom-status.csv").toString());
	lineEditCSVSeparator->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/CSVSeparator", ";").toString());
	lineEditCSVDelimiter->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/CSVDelimiter", "\"").toString());
	lineEditHTMLFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/HTMLFile", "data/tmp/rom-status.html").toString());
	spinBoxHTMLBorderWidth->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/HTMLBorderWidth", 1).toInt());

	progressBarExport->reset();
}

ROMStatusExporter::~ROMStatusExporter()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMStatusExporter::~ROMStatusExporter()");
#endif

}

void ROMStatusExporter::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMStatusExporter::adjustIconSizes()");
#endif

	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	toolButtonBrowseASCIIFile->setIconSize(iconSize);
	toolButtonBrowseCSVFile->setIconSize(iconSize);
	toolButtonBrowseHTMLFile->setIconSize(iconSize);
	toolButtonExportC->setIconSize(iconSize);
	toolButtonExportM->setIconSize(iconSize);
	toolButtonExportI->setIconSize(iconSize);
	toolButtonExportN->setIconSize(iconSize);
	toolButtonExportU->setIconSize(iconSize);
	pushButtonClose->setIconSize(iconSize);
	pushButtonExport->setIconSize(iconSize);
	comboBoxSortOrder->setIconSize(iconSize);
}

void ROMStatusExporter::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::closeEvent(QCloseEvent *e = %1)").arg((qulonglong) e));
#endif

	// save settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/OutputFormat", comboBoxOutputFormat->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportC", toolButtonExportC->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportM", toolButtonExportM->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportI", toolButtonExportI->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportN", toolButtonExportN->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportU", toolButtonExportU->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/SortCriteria", comboBoxSortCriteria->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/SortOrder", comboBoxSortOrder->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/IncludeHeader", checkBoxIncludeHeader->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/IncludeStatistics", checkBoxIncludeStatistics->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ExportToClipboard", checkBoxExportToClipboard->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/OverwriteBlindly", checkBoxOverwriteBlindly->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ASCIIFile", lineEditASCIIFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/ASCIIColumnWidth", spinBoxASCIIColumnWidth->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/CSVFile", lineEditCSVFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/CSVSeparator", lineEditCSVSeparator->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/CSVDelimiter", lineEditCSVDelimiter->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/HTMLFile", lineEditHTMLFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMStatusExporter/HTMLBorderWidth", spinBoxHTMLBorderWidth->value());

	e->accept();
}

void ROMStatusExporter::exportToASCII()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::exportToASCII()"));
#endif

	QFile exportFile(lineEditASCIIFile->text());
	QString clipboardBuffer;
	QTextStream ts;
	if ( !checkBoxExportToClipboard->isChecked() ) {
		if ( !checkBoxOverwriteBlindly->isChecked() && exportFile.exists() ) {
			switch ( QMessageBox::question(this, tr("Confirm"),
						tr("Overwrite existing file?"),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
				case QMessageBox::Yes:
					break;

				case QMessageBox::No:
				default:
					return;
			}
		}

		if ( exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			qmc2ExportingROMStatus = true;
			ts.setDevice(&exportFile);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting ROM status in ASCII format to '%1'").arg(QFileInfo(exportFile).filePath()));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open ASCII export file '%1' for writing, please check permissions").arg(QFileInfo(exportFile).filePath()));
			return;
		}
	} else {
		qmc2ExportingROMStatus = true;
		ts.setString(&clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting ROM status in ASCII format to clipboard"));
	}

	frameExportParams->setEnabled(false);

	progressBarExport->setRange(0, qmc2Gamelist->numGames);
	progressBarExport->reset();
	progressBarExport->setValue(0);

	int maxLength = 0;
	QStringList headerStrings;
	headerStrings << tr("Emulator") << tr("Date") << tr("Time") << tr("Total sets") << tr("Correct") << tr("Mostly correct") << tr("Incorrect") << tr("Not found") << tr("Unknown");
	foreach (QString s, headerStrings)
		if ( s.length() > maxLength )
			maxLength = s.length();
	maxLength += 2;
    
	if ( checkBoxIncludeHeader->isChecked() ) {
		QString qmc2Version(XSTR(QMC2_VERSION));
		ts << tr("ROM Status Export - created by QMC2 %1").arg(qmc2Version) << "\n" << QString().leftJustified(tr("ROM Status Export - created by QMC2 %1").arg(qmc2Version).length(), '-', true) << "\n\n";
#if defined(QMC2_SDLMAME)
		QString emulatorTarget = tr("SDLMAME");
#elif defined(QMC2_SDLMESS)
		QString emulatorTarget = tr("SDLMESS");
#elif defined(QMC2_SDLUME)
		QString emulatorTarget = tr("SDLUME");
#elif defined(QMC2_MAME)
		QString emulatorTarget = tr("MAME");
#elif defined(QMC2_MESS)
		QString emulatorTarget = tr("MESS");
#elif defined(QMC2_UME)
		QString emulatorTarget = tr("UME");
#else
		QString emulatorTarget = tr("unknown");
#endif
		ts << tr("Emulator") << " " << QString().leftJustified(maxLength - tr("Emulator").length(), '.', true) << " " << emulatorTarget << " " << qmc2Gamelist->emulatorVersion << "\n";
		ts << tr("Date") << " " << QString().leftJustified(maxLength - tr("Date").length(), '.', true) << " " << QDate::currentDate().toString() << "\n";
		ts << tr("Time") << " " << QString().leftJustified(maxLength - tr("Time").length(), '.', true) << " " << QTime::currentTime().toString() << "\n\n";
	}

	QLocale locale;
	if ( checkBoxIncludeStatistics->isChecked() ) {
		ts << tr("Overall ROM Status") << "\n" << QString().leftJustified(tr("Overall ROM Status").length(), '-', true) << "\n\n";
		ts << tr("Total sets") << " " << QString().leftJustified(maxLength - tr("Total sets").length(), '.', true) << " " << locale.toString(qmc2Gamelist->numGames) << "\n";
		ts << tr("Correct") << " " << QString().leftJustified(maxLength - tr("Correct").length(), '.', true) << " " << locale.toString(qmc2Gamelist->numCorrectGames) << "\n";
		ts << tr("Mostly correct") << " " << QString().leftJustified(maxLength - tr("Mostly correct").length(), '.', true) << " " << locale.toString(qmc2Gamelist->numMostlyCorrectGames) << "\n";
		ts << tr("Incorrect") << " " << QString().leftJustified(maxLength - tr("Incorrect").length(), '.', true) << " " << locale.toString(qmc2Gamelist->numIncorrectGames) << "\n";
		ts << tr("Not found") << " " << QString().leftJustified(maxLength - tr("Not found").length(), '.', true) << " " << locale.toString(qmc2Gamelist->numNotFoundGames) << "\n";
		ts << tr("Unknown") << " " << QString().leftJustified(maxLength - tr("Unknown").length(), '.', true) << " " << locale.toString(qmc2Gamelist->numUnknownGames) << "\n\n";
	}

	ts << tr("Detailed ROM Status") << "\n" << QString().leftJustified(tr("Detailed ROM Status").length(), '-', true) << "\n\n";

	// create a sorted multi-map as export data...
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting, filtering and analyzing export data"));

	int maxDescriptionColumnWidth = tr("Description").length();
	int maxNameColumnWidth = tr("Name").length();
	int maxStateColumnWidth = tr("Status").length();
	int maxManufacturerColumnWidth = tr("Manufacturer").length();
	int maxYearColumnWidth = tr("Year").length();
	int maxRomTypesColumnWidth = tr("ROM types").length();

	QMultiMap<QString, QTreeWidgetItem *> exportMap;
	for (int i = 0; i < qmc2MainWindow->treeWidgetGamelist->topLevelItemCount(); i++) {
		progressBarExport->setValue(i + 1);
		qApp->processEvents();
		QTreeWidgetItem *item = qmc2MainWindow->treeWidgetGamelist->topLevelItem(i);
		QString translatedState;
		switch ( qmc2Gamelist->romState(item->text(QMC2_GAMELIST_COLUMN_NAME)) ) {
			case 'C':
				if ( !toolButtonExportC->isChecked() )
					continue;
				else
					translatedState = tr("correct");
				break;

			case 'M':
				if ( !toolButtonExportM->isChecked() )
					continue;
				else
					translatedState = tr("mostly correct");
				break;

			case 'I':
				if ( !toolButtonExportI->isChecked() )
					continue;
				else
					translatedState = tr("incorrect");
				break;

			case 'N':
				if ( !toolButtonExportN->isChecked() )
					continue;
				else
					translatedState = tr("not found");
				break;

			case 'U':
			default:
				if ( !toolButtonExportU->isChecked() )
					continue;
				else
					translatedState = tr("unknown");
				break;
		}

		switch ( comboBoxSortCriteria->currentIndex() ) {
			case QMC2_RSE_SORT_BY_DESCRIPTION:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_GAME), item);
				break;

			case QMC2_RSE_SORT_BY_ROM_STATE:
				exportMap.insert(QString(qmc2Gamelist->romState(item->text(QMC2_GAMELIST_COLUMN_NAME))), item);
				break;

			case QMC2_RSE_SORT_BY_YEAR:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_YEAR), item);
				break;

			case QMC2_RSE_SORT_BY_MANUFACTURER:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_MANU), item);
				break;

			case QMC2_RSE_SORT_BY_NAME:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_NAME), item);
				break;

			case QMC2_RSE_SORT_BY_ROMTYPES:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_RTYPES), item);
				break;

			default:
				break;
		}

		if ( item->text(QMC2_GAMELIST_COLUMN_GAME).length() > maxDescriptionColumnWidth )
			maxDescriptionColumnWidth = item->text(QMC2_GAMELIST_COLUMN_GAME).length();
		if ( item->text(QMC2_GAMELIST_COLUMN_NAME).length() > maxNameColumnWidth )
			maxNameColumnWidth = item->text(QMC2_GAMELIST_COLUMN_NAME).length();
		if ( item->text(QMC2_GAMELIST_COLUMN_MANU).length() > maxManufacturerColumnWidth )
			maxManufacturerColumnWidth = item->text(QMC2_GAMELIST_COLUMN_MANU).length();
		if ( item->text(QMC2_GAMELIST_COLUMN_YEAR).length() > maxYearColumnWidth )
			maxYearColumnWidth = item->text(QMC2_GAMELIST_COLUMN_YEAR).length();
		if ( translatedState.length() > maxStateColumnWidth )
			maxStateColumnWidth = translatedState.length();
		if ( item->text(QMC2_GAMELIST_COLUMN_RTYPES).length() > maxRomTypesColumnWidth )
			maxRomTypesColumnWidth = item->text(QMC2_GAMELIST_COLUMN_RTYPES).length();
	}

	// truncate column widths if applicable...
	if ( spinBoxASCIIColumnWidth->value() > 0 ) {
		maxDescriptionColumnWidth = QMC2_MIN(maxDescriptionColumnWidth, spinBoxASCIIColumnWidth->value());
		maxNameColumnWidth = QMC2_MIN(maxNameColumnWidth, spinBoxASCIIColumnWidth->value());
		maxManufacturerColumnWidth = QMC2_MIN(maxManufacturerColumnWidth, spinBoxASCIIColumnWidth->value());
		maxYearColumnWidth = QMC2_MIN(maxYearColumnWidth, spinBoxASCIIColumnWidth->value());
		maxStateColumnWidth = QMC2_MIN(maxStateColumnWidth, spinBoxASCIIColumnWidth->value());
		maxRomTypesColumnWidth = QMC2_MIN(maxRomTypesColumnWidth, spinBoxASCIIColumnWidth->value());
	}

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (sorting, filtering and analyzing export data)"));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("writing export data"));

	ts << tr("Name").leftJustified(maxNameColumnWidth, ' ', true) << "  "
		<< tr("Status").leftJustified(maxStateColumnWidth, ' ', true) << "  "
		<< tr("Description").leftJustified(maxDescriptionColumnWidth, ' ', true) << "  "
		<< tr("Year").leftJustified(maxYearColumnWidth, ' ', true) << "  "
		<< tr("Manufacturer").leftJustified(maxManufacturerColumnWidth, ' ', true) << "  "
		<< tr("ROM types").leftJustified(maxRomTypesColumnWidth, ' ', true)
		<< "\n";
	ts << QString("-").leftJustified(maxNameColumnWidth, '-', true) << "  "
		<< QString("-").leftJustified(maxStateColumnWidth, '-', true) << "  "
		<< QString("-").leftJustified(maxDescriptionColumnWidth, '-', true) << "  "
		<< QString("-").leftJustified(maxYearColumnWidth, '-', true) << "  "
		<< QString("-").leftJustified(maxManufacturerColumnWidth, '-', true) << "  "
		<< QString("-").leftJustified(maxRomTypesColumnWidth, '-', true)
		<< "\n";

	QMapIterator<QString, QTreeWidgetItem *> itExport(exportMap);
	int i = 0;
	bool ascendingOrder = (comboBoxSortOrder->currentIndex() == 0);
	bool showMoreChars = ( spinBoxASCIIColumnWidth->value() > 3 );
	if ( !ascendingOrder )
		itExport.toBack();
	while ( ( ( ascendingOrder && itExport.hasNext() ) || ( !ascendingOrder && itExport.hasPrevious() ) ) && !qmc2StopParser ) {
		progressBarExport->setValue(++i);
		qApp->processEvents();

		if ( ascendingOrder )
			itExport.next();
		else
			itExport.previous();

		QString s = itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME);
		if ( showMoreChars && s.length() > maxNameColumnWidth )
			ts << s.left(maxNameColumnWidth - 3).leftJustified(maxNameColumnWidth, '.', true) << "  ";
		else
			ts << s.leftJustified(maxNameColumnWidth, ' ', true) << "  ";
		switch ( qmc2Gamelist->romState(s) ) {
			case 'C':
				s = tr("correct");
				break;

			case 'M':
				s = tr("mostly correct");
				break;

			case 'I':
				s = tr("incorrect");
				break;

			case 'N':
				s = tr("not found");
				break;

			case 'U':
			default:
				s = tr("unknown");
				break;
		}
		if ( showMoreChars && s.length() > maxStateColumnWidth )
			ts << s.left(maxStateColumnWidth - 3).leftJustified(maxStateColumnWidth, '.', true) << "  ";
		else
			ts << s.leftJustified(maxStateColumnWidth, ' ', true) << "  ";
		s = itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME);
		if ( showMoreChars && s.length() > maxDescriptionColumnWidth )
			ts << s.left(maxDescriptionColumnWidth - 3).leftJustified(maxDescriptionColumnWidth, '.', true) << "  ";
		else
			ts << s.leftJustified(maxDescriptionColumnWidth, ' ', true) << "  ";
		s = itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR);
		if ( showMoreChars && s.length() > maxYearColumnWidth )
			ts << s.left(maxYearColumnWidth - 3).leftJustified(maxYearColumnWidth, '.', true) << "  ";
		else
			ts << s.leftJustified(maxYearColumnWidth, ' ', true) << "  ";
		s = itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU);
		if ( showMoreChars && s.length() > maxManufacturerColumnWidth )
			ts << s.left(maxManufacturerColumnWidth - 3).leftJustified(maxManufacturerColumnWidth, '.', true) << "  ";
		else
			ts << s.leftJustified(maxManufacturerColumnWidth, ' ', true) << "  ";
		s = itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES);
		if ( showMoreChars && s.length() > maxRomTypesColumnWidth )
			ts << s.left(maxRomTypesColumnWidth - 3).leftJustified(maxRomTypesColumnWidth, '.', true) << "\n";
		else
			ts << s.leftJustified(maxRomTypesColumnWidth, ' ', true) << "\n";
	}

	progressBarExport->reset();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (writing export data)"));

	if ( checkBoxExportToClipboard->isChecked() ) {
		qApp->clipboard()->setText(clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting ROM status in ASCII format to clipboard)"));
	} else {
		exportFile.close();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting ROM status in ASCII format to '%1')").arg(QFileInfo(exportFile).filePath()));
	}

	frameExportParams->setEnabled(true);
	qmc2ExportingROMStatus = false;
}

void ROMStatusExporter::exportToCSV()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::exportToCSV()"));
#endif

	QFile exportFile(lineEditCSVFile->text());
	QString clipboardBuffer;
	QTextStream ts;
	if ( !checkBoxExportToClipboard->isChecked() ) {
		if ( !checkBoxOverwriteBlindly->isChecked() && exportFile.exists() ) {
			switch ( QMessageBox::question(this, tr("Confirm"),
						tr("Overwrite existing file?"),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
				case QMessageBox::Yes:
					break;

				case QMessageBox::No:
				default:
					return;
			}
		}

		if ( exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			qmc2ExportingROMStatus = true;
			ts.setDevice(&exportFile);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting ROM status in CSV format to '%1'").arg(QFileInfo(exportFile).filePath()));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open CSV export file '%1' for writing, please check permissions").arg(QFileInfo(exportFile).filePath()));
			return;
		}
	} else {
		qmc2ExportingROMStatus = true;
		ts.setString(&clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting ROM status in CSV format to clipboard"));
	}

	frameExportParams->setEnabled(false);

	progressBarExport->setRange(0, qmc2Gamelist->numGames);
	progressBarExport->reset();
	progressBarExport->setValue(0);

	QString sep = lineEditCSVSeparator->text();
	QString del = lineEditCSVDelimiter->text();

	if ( checkBoxIncludeHeader->isChecked() ) {
		QString qmc2Version(QString(XSTR(QMC2_VERSION)));
		ts << del << tr("ROM Status Export - created by QMC2 %1").arg(qmc2Version) << del << "\n" << del << del << "\n";
#if defined(QMC2_SDLMAME)
		QString emulatorTarget = tr("SDLMAME");
#elif defined(QMC2_SDLMESS)
		QString emulatorTarget = tr("SDLMESS");
#elif defined(QMC2_SDLUME)
		QString emulatorTarget = tr("SDLUME");
#elif defined(QMC2_MAME)
		QString emulatorTarget = tr("MAME");
#elif defined(QMC2_MESS)
		QString emulatorTarget = tr("MESS");
#elif defined(QMC2_UME)
		QString emulatorTarget = tr("UME");
#else
		QString emulatorTarget = tr("unknown");
#endif
		ts << del << tr("Emulator") << del << sep << del << emulatorTarget << " " << qmc2Gamelist->emulatorVersion << del << "\n";
		ts << del << tr("Date") << del << sep << del << QDate::currentDate().toString() << del << "\n";
		ts << del << tr("Time") << del << sep << del << QTime::currentTime().toString() << del << "\n" << del << del << "\n";
	}

	if ( checkBoxIncludeStatistics->isChecked() ) {
		QLocale locale;
		ts << del << tr("Overall ROM Status") << del << "\n" << del << del << "\n";
		ts << del << tr("Total sets") << del << sep << del << locale.toString(qmc2Gamelist->numGames) << del << "\n";
		ts << del << tr("Correct") << del << sep << del << locale.toString(qmc2Gamelist->numCorrectGames) << del << "\n";
		ts << del << tr("Mostly correct") << del << sep << del << locale.toString(qmc2Gamelist->numMostlyCorrectGames) << del << "\n";
		ts << del << tr("Incorrect") << del << sep << del << locale.toString(qmc2Gamelist->numIncorrectGames) << del << "\n";
		ts << del << tr("Not found") << del << sep << del << locale.toString(qmc2Gamelist->numNotFoundGames) << del << "\n";
		ts << del << tr("Unknown") << del << sep << del << locale.toString(qmc2Gamelist->numUnknownGames) << del << "\n" << del << del << "\n";
	}

	ts << del << tr("Detailed ROM Status") << del << "\n" << del << del << "\n";

	// create a sorted multi-map as export data...
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting and filtering export data"));

	QMultiMap<QString, QTreeWidgetItem *> exportMap;
	for (int i = 0; i < qmc2MainWindow->treeWidgetGamelist->topLevelItemCount(); i++) {
		progressBarExport->setValue(i + 1);
		qApp->processEvents();
		QTreeWidgetItem *item = qmc2MainWindow->treeWidgetGamelist->topLevelItem(i);
		switch ( comboBoxSortCriteria->currentIndex() ) {
			case QMC2_RSE_SORT_BY_DESCRIPTION:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_GAME), item);
				break;

			case QMC2_RSE_SORT_BY_ROM_STATE:
				exportMap.insert(QString(qmc2Gamelist->romState(item->text(QMC2_GAMELIST_COLUMN_NAME))), item);
				break;

			case QMC2_RSE_SORT_BY_YEAR:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_YEAR), item);
				break;

			case QMC2_RSE_SORT_BY_MANUFACTURER:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_MANU), item);
				break;

			case QMC2_RSE_SORT_BY_NAME:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_NAME), item);
				break;

			case QMC2_RSE_SORT_BY_ROMTYPES:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_RTYPES), item);
				break;

			default:
				break;
		}
	}

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (sorting and filtering export data)"));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("writing export data"));

	ts << del << tr("Name") << del << sep
		<< del << tr("Status") << del << sep
		<< del << tr("Description") << del << sep
		<< del << tr("Year") << del << sep
		<< del << tr("Manufacturer") << del << sep
		<< del << tr("ROM types") << del << "\n" << del << del << "\n";

	QMapIterator<QString, QTreeWidgetItem *> itExport(exportMap);
	int i = 0;
	bool ascendingOrder = (comboBoxSortOrder->currentIndex() == 0);
	if ( !ascendingOrder )
		itExport.toBack();
	while ( ( ( ascendingOrder && itExport.hasNext() ) || ( !ascendingOrder && itExport.hasPrevious() ) ) && !qmc2StopParser ) {
		progressBarExport->setValue(++i);
		qApp->processEvents();

		if ( ascendingOrder )
			itExport.next();
		else
			itExport.previous();

		switch ( qmc2Gamelist->romState(itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME)) ) {
			case 'C':
				if ( toolButtonExportC->isChecked() ) {
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << del << sep;
					ts << del << tr("correct") << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << del << "\n";
				}
				break;

			case 'M':
				if ( toolButtonExportM->isChecked() ) {
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << del << sep;
					ts << del << tr("mostly correct") << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << del << "\n";
				}
				break;

			case 'I':
				if ( toolButtonExportI->isChecked() ) {
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << del << sep;
					ts << del << tr("incorrect") << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << del << "\n";
				}
				break;

			case 'N':
				if ( toolButtonExportN->isChecked() ) {
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << del << sep;
					ts << del << tr("not found") << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << del << "\n";
				}
				break;

			case 'U':
			default:
				if ( toolButtonExportU->isChecked() ) {
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << del << sep;
					ts << del << tr("unknown") << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU) << del << sep;
					ts << del << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << del << "\n";
				}
				break;
		}
	}

	progressBarExport->reset();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (writing export data)"));

	if ( checkBoxExportToClipboard->isChecked() ) {
		qApp->clipboard()->setText(clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting ROM status in CSV format to clipboard)"));
	} else {
		exportFile.close();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting ROM status in CSV format to '%1')").arg(QFileInfo(exportFile).filePath()));
	}

	frameExportParams->setEnabled(true);
	qmc2ExportingROMStatus = false;
}

void ROMStatusExporter::exportToHTML()
{
#ifdef QMC2_DEBUG
      	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::exportToHTML()"));
#endif

      	QFile exportFile(lineEditHTMLFile->text());
      	QString clipboardBuffer;
      	QTextStream ts;
      	if ( !checkBoxExportToClipboard->isChecked() ) {
	    	if ( !checkBoxOverwriteBlindly->isChecked() && exportFile.exists() ) {
		  	switch ( QMessageBox::question(this, tr("Confirm"),
	   					tr("Overwrite existing file?"),
	   					QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
				case QMessageBox::Yes:
			      		break;

				case QMessageBox::No:
				default:
			      		return;
		  	}
	    	}

	    	if ( exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		  	qmc2ExportingROMStatus = true;
		  	ts.setDevice(&exportFile);
		  	ts.setCodec(QTextCodec::codecForName("UTF-8"));
		  	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting ROM status in HTML format to '%1'").arg(QFileInfo(exportFile).filePath()));
	    	} else {
		  	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open HTML export file '%1' for writing, please check permissions").arg(QFileInfo(exportFile).filePath()));
		  	return;
	    	}
      	} else {
	    	qmc2ExportingROMStatus = true;
	    	ts.setString(&clipboardBuffer);
	    	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting ROM status in HTML format to clipboard"));
      	}

      	frameExportParams->setEnabled(false);

      	progressBarExport->setRange(0, qmc2Gamelist->numGames);
      	progressBarExport->reset();
      	progressBarExport->setValue(0);

      	QString qmc2Version(XSTR(QMC2_VERSION));

      	ts << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
	   	<< "<html>\n"
	   	<< "<head>\n"
	   	<< "<meta http-equiv=\"CONTENT-TYPE\" content=\"text/html; charset=utf-8\">\n"
	   	<< "<title>" << tr("ROM Status Export - created by QMC2 %1").arg(qmc2Version) << "</title>\n"
	   	<< "</head>\n"
	   	<< "<body style=\"color: black; background-color: lightyellow;\" dir=\"ltr\" alink=\"#111111\" link=\"#111111\" vlink=\"#999999\">\n";

      	if ( checkBoxIncludeHeader->isChecked() ) {
#if defined(QMC2_SDLMAME)
	    	QString emulatorTarget = tr("SDLMAME");
#elif defined(QMC2_SDLMESS)
	    	QString emulatorTarget = tr("SDLMESS");
#elif defined(QMC2_SDLUME)
	    	QString emulatorTarget = tr("SDLUME");
#elif defined(QMC2_MAME)
	    	QString emulatorTarget = tr("MAME");
#elif defined(QMC2_MESS)
	    	QString emulatorTarget = tr("MESS");
#elif defined(QMC2_UME)
	    	QString emulatorTarget = tr("UME");
#else
	    	QString emulatorTarget = tr("unknown");
#endif
	    	ts << "<h3>" << tr("ROM Status Export - created by QMC2 %1").arg(qmc2Version) << "</h3>\n"
		 	<< "<p>\n"
		 	<< "<table border=\"" << spinBoxHTMLBorderWidth->value() << "\">\n"
		 	<< "<tr>\n"
		 	<< "<td nowrap>" << tr("Emulator") << "</td><td nowrap>" << emulatorTarget << " " << qmc2Gamelist->emulatorVersion << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Date") << "</td><td nowrap>" << QDate::currentDate().toString() << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Time") << "</td><td nowrap>" << QTime::currentTime().toString() << "</td>\n"
		 	<< "</tr>\n"
		 	<< "</table>\n"
		 	<< "</p>\n";
      	}

      	if ( checkBoxIncludeStatistics->isChecked() ) {
	    	QLocale locale;
	    	ts << "<h3>" << tr("Overall ROM Status") << "</h3>\n"
		 	<< "<p>\n"
		 	<< "<table border=\"" << spinBoxHTMLBorderWidth->value() << "\">\n"
		 	<< "<tr>\n"
		 	<< "<td nowrap>" << tr("Total sets") << "</td><td nowrap align=\"right\">" << locale.toString(qmc2Gamelist->numGames) << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Correct") << "</td><td nowrap align=\"right\">" << locale.toString(qmc2Gamelist->numCorrectGames) << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Mostly correct") << "</td><td nowrap align=\"right\">" << locale.toString(qmc2Gamelist->numMostlyCorrectGames) << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Incorrect") << "</td><td nowrap align=\"right\">" << locale.toString(qmc2Gamelist->numIncorrectGames) << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Not found") << "</td><td nowrap align=\"right\">" << locale.toString(qmc2Gamelist->numNotFoundGames) << "</td>\n"
		 	<< "</tr>\n<tr>\n"
		 	<< "<td nowrap>" << tr("Unknown") << "</td><td nowrap align=\"right\">" << locale.toString(qmc2Gamelist->numUnknownGames) << "</td>\n"
		 	<< "</tr>\n"
		 	<< "</table>\n"
		 	<< "</p>\n";
      	}

      	ts << "<h3>" << tr("Detailed ROM Status") << "</h3>\n"
	   	<< "<p>\n";

      	// create a sorted multi-map as export data...
      	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting and filtering export data"));

      	QMultiMap<QString, QTreeWidgetItem *> exportMap;
      	for (int i = 0; i < qmc2MainWindow->treeWidgetGamelist->topLevelItemCount(); i++) {
	    	progressBarExport->setValue(i + 1);
	    	qApp->processEvents();
	    	QTreeWidgetItem *item = qmc2MainWindow->treeWidgetGamelist->topLevelItem(i);
	    	switch ( comboBoxSortCriteria->currentIndex() ) {
		  	case QMC2_RSE_SORT_BY_DESCRIPTION:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_GAME), item);
				break;

		  	case QMC2_RSE_SORT_BY_ROM_STATE:
				exportMap.insert(QString(qmc2Gamelist->romState(item->text(QMC2_GAMELIST_COLUMN_NAME))), item);
				break;

		  	case QMC2_RSE_SORT_BY_YEAR:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_YEAR), item);
				break;

		  	case QMC2_RSE_SORT_BY_MANUFACTURER:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_MANU), item);
				break;

		  	case QMC2_RSE_SORT_BY_NAME:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_NAME), item);
				break;

		  	case QMC2_RSE_SORT_BY_ROMTYPES:
				exportMap.insert(item->text(QMC2_GAMELIST_COLUMN_RTYPES), item);
				break;

		  	default:
				break;
	    	}
      	}

      	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (sorting and filtering export data)"));
      	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("writing export data"));

      	ts << "<table border=\"" << spinBoxHTMLBorderWidth->value() << "\">\n"
	   	<< "<tr>\n"
	   	<< "<td nowrap><b>" << tr("Name") << "</b></td><td nowrap><b>" << tr("Status") << "</b></td><td nowrap><b>" << tr("Description") << "</b></td><td nowrap><b>" << tr("Year") << "</b></td><td nowrap><b>" << tr("Manufacturer") << "</b></td><td nowrap><b>" << tr("ROM types") << "</td></b>\n"
	   	<< "</tr>\n";

      	QMapIterator<QString, QTreeWidgetItem *> itExport(exportMap);
      	int i = 0;
      	bool ascendingOrder = (comboBoxSortOrder->currentIndex() == 0);
      	if ( !ascendingOrder )
	    	itExport.toBack();
      	while ( ( ( ascendingOrder && itExport.hasNext() ) || ( !ascendingOrder && itExport.hasPrevious() ) ) && !qmc2StopParser ) {
	    	progressBarExport->setValue(++i);
	    	qApp->processEvents();

	    	if ( ascendingOrder )
		  	itExport.next();
	    	else
		  	itExport.previous();

	    	switch ( qmc2Gamelist->romState(itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME)) ) {
		  	case 'C':
				if ( toolButtonExportC->isChecked() ) {
					ts << "<tr>\n<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << "</td>";
					ts << "<td valign=\"top\">" << tr("correct") << "</td>";
					ts << "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << "</td>\n"
						<< "</tr>\n";
				}
				break;

			case 'M':
				if ( toolButtonExportM->isChecked() ) {
					ts << "<tr>\n<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << "</td>";
					ts << "<td valign=\"top\">" << tr("mostly correct") << "</td>";
					ts << "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << "</td>\n"
						<< "</tr>\n";
				}
				break;

			case 'I':
				if ( toolButtonExportI->isChecked() ) {
					ts << "<tr>\n<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << "</td>";
					ts << "<td valign=\"top\">" << tr("incorrect") << "</td>";
					ts << "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << "</td>\n"
						<< "</tr>\n";
				}
				break;

			case 'N':
				if ( toolButtonExportN->isChecked() ) {
					ts << "<tr>\n<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << "</td>";
					ts << "<td valign=\"top\">" << tr("not found") << "</td>";
					ts << "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << "</td>\n"
						<< "</tr>\n";
				}
				break;

			case 'U':
			default:
				if ( toolButtonExportU->isChecked() ) {
					ts << "<tr>\n<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_NAME) << "</td>";
					ts << "<td valign=\"top\">" << tr("unknown") << "</td>";
					ts << "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_GAME).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_YEAR) << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_MANU).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;").replace("'", "&apos;") << "</td>"
						<< "<td valign=\"top\">" << itExport.value()->text(QMC2_GAMELIST_COLUMN_RTYPES) << "</td>\n"
						<< "</tr>\n";
				}
				break;
		}
	}

	ts << "</table>\n</p>\n</html>";

	progressBarExport->reset();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (writing export data)"));

	if ( checkBoxExportToClipboard->isChecked() ) {
		qApp->clipboard()->setText(clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting ROM status in HTML format to clipboard)"));
	} else {
		exportFile.close();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting ROM status in HTML format to '%1')").arg(QFileInfo(exportFile).filePath()));
	}

	frameExportParams->setEnabled(true);
	qmc2ExportingROMStatus = false;
}

void ROMStatusExporter::on_toolButtonBrowseASCIIFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMStatusExporter::on_toolButtonBrowseASCIIFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose ASCII export file"), lineEditASCIIFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditASCIIFile->setText(s);
	raise();
}

void ROMStatusExporter::on_toolButtonBrowseCSVFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMStatusExporter::on_toolButtonBrowseCSVFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose CSV export file"), lineEditCSVFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCSVFile->setText(s);
	raise();
}

void ROMStatusExporter::on_toolButtonBrowseHTMLFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMStatusExporter::on_toolButtonBrowseHTMLFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose HTML export file"), lineEditHTMLFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditHTMLFile->setText(s);
	raise();
}

void ROMStatusExporter::on_pushButtonExport_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMStatusExporter::on_pushButtonExport_clicked()");
#endif

	if ( !qmc2Gamelist ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("gamelist is not ready, please wait"));
		return;
	}

	switch ( comboBoxOutputFormat->currentIndex() ) {
		case QMC2_ROMSTATUSEXPORT_FORMAT_ASCII_INDEX:
			exportToASCII();
			break;

		case QMC2_ROMSTATUSEXPORT_FORMAT_CSV_INDEX:
			exportToCSV();
			break;

		case QMC2_ROMSTATUSEXPORT_FORMAT_HTML_INDEX:
			exportToHTML();
			break;

		case QMC2_ROMSTATUSEXPORT_FORMAT_ALL_INDEX:
			exportToASCII();
			exportToCSV();
			exportToHTML();
			break;

		default:
			break;
	}
}

void ROMStatusExporter::on_comboBoxOutputFormat_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::on_comboBoxOutputFormat_currentIndexChanged(int index = %1)").arg(index));
#endif

	stackedWidgetOutputParams->setCurrentIndex(index);

	if ( index > QMC2_ROMSTATUSEXPORT_FORMAT_HTML_INDEX ) {
		exportListAutoCorrected = true;
		checkBoxExportToClipboard->setChecked(false);
	}

	exportListAutoCorrected = false;
}

void ROMStatusExporter::on_checkBoxExportToClipboard_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMStatusExporter::on_checkBoxExportToClipboard_toggled(bool enable = %1)").arg(enable));
#endif

	if ( enable ) {
		if ( comboBoxOutputFormat->currentIndex() > QMC2_ROMSTATUSEXPORT_FORMAT_HTML_INDEX && !exportListAutoCorrected ) {
			exportListAutoCorrected = true;
			comboBoxOutputFormat->setCurrentIndex(0);
		}
	}

	exportListAutoCorrected = false;
}
