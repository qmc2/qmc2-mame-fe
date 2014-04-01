#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QMessageBox>
#include <QFileDialog>
#endif

#include "swlistexport.h"
#include "macros.h"
#include "options.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;

SoftwareListExporter::SoftwareListExporter(QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::SoftwareListExporter(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	softwareList = (SoftwareList *)parent;

	exportListAutoCorrected = false;

	columnNames << tr("Title") << tr("Name") << tr("Publisher") << tr("Year") << tr("Part") << tr("Interface") << tr("List") << tr("Supported");
	columnNamesUntranslated << "Title" << "Name" << "Publisher" << "Year" << "Part" << "Interface" << "List" << "Supported";
	defaultColumnActivation << "true" << "true" << "true" << "true" << "true" << "true" << "true" << "true";

	comboBoxOutputFormat->insertSeparator(QMC2_SWLISTEXPORT_FORMAT_SEP_INDEX);
	comboBoxOutputFormat->insertItem(QMC2_SWLISTEXPORT_FORMAT_ALL_INDEX, tr("Both formats"));

	setWindowTitle(tr("Export software-list for '%1'").arg(softwareList->systemName));

	// restore settings
	comboBoxOutputFormat->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/OutputFormat", 0).toInt());
	comboBoxSortCriteria->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortCriteria", 0).toInt());
	comboBoxSortOrder->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortOrder", 0).toInt());
	checkBoxIncludeColumnHeaders->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/IncludeColumnHeaders", true).toBool());
	if ( comboBoxOutputFormat->currentIndex() <= QMC2_SWLISTEXPORT_FORMAT_CSV_INDEX )
		checkBoxExportToClipboard->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ExportToClipboard", false).toBool());
	else
		checkBoxExportToClipboard->setChecked(false);
	checkBoxOverwriteBlindly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/OverwriteBlindly", false).toBool());
	lineEditASCIIFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ASCIIFile", "data/tmp/softlist-$ID$.txt").toString());
	spinBoxASCIIColumnWidth->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ASCIIColumnWidth", 0).toInt());
	lineEditCSVFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVFile", "data/tmp/softlist-$ID$.csv").toString());
	lineEditCSVSeparator->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVSeparator", ";").toString());
	lineEditCSVDelimiter->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVDelimiter", "\"").toString());

	listWidgetColumns->clear();

	QStringList orderedColumns = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnOrder", columnNamesUntranslated).toStringList();
	QStringList columnActivation = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnActivation", defaultColumnActivation).toStringList();

	// make sure all columns are included, otherwise reset column order & activation
	if ( orderedColumns.toSet() != columnNamesUntranslated.toSet() ) {
		orderedColumns = columnNamesUntranslated;
		columnActivation = defaultColumnActivation;
	}
	for (int col = 0; col < orderedColumns.count(); col++) {
		QListWidgetItem *item = new QListWidgetItem(columnNames[columnNamesUntranslated.indexOf(orderedColumns[col])], listWidgetColumns);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);
		item->setCheckState(columnActivation[col] == "true" ? Qt::Checked : Qt::Unchecked);
	}
}

SoftwareListExporter::~SoftwareListExporter()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListExporter::~SoftwareListExporter()");
#endif

}

void SoftwareListExporter::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListExporter::adjustIconSizes()");
#endif

	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	toolButtonBrowseASCIIFile->setIconSize(iconSize);
	toolButtonBrowseCSVFile->setIconSize(iconSize);
	pushButtonClose->setIconSize(iconSize);
	pushButtonExport->setIconSize(iconSize);
}

void SoftwareListExporter::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::showEvent(QShowEvent *e = %1)").arg((qulonglong) e));
#endif

	adjustIconSizes();
	adjustSize();

	if ( e )
		e->accept();
}

void SoftwareListExporter::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::hideEvent(QHideEvent *e = %1)").arg((qulonglong) e));
#endif

	saveSettings();

	if ( e )
		e->accept();
}

void SoftwareListExporter::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::closeEvent(QCloseEvent *e = %1)").arg((qulonglong) e));
#endif

	// save settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/OutputFormat", comboBoxOutputFormat->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortCriteria", comboBoxSortCriteria->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortOrder", comboBoxSortOrder->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/IncludeColumnHeaders", checkBoxIncludeColumnHeaders->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ExportToClipboard", checkBoxExportToClipboard->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/OverwriteBlindly", checkBoxOverwriteBlindly->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ASCIIFile", lineEditASCIIFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ASCIIColumnWidth", spinBoxASCIIColumnWidth->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVFile", lineEditCSVFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVSeparator", lineEditCSVSeparator->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVDelimiter", lineEditCSVDelimiter->text());

	QStringList orderedColumns;
	QStringList columnActivation;
	for (int col = 0; col < listWidgetColumns->count(); col++) {
		QListWidgetItem *item = listWidgetColumns->item(col);
		orderedColumns << columnNamesUntranslated[columnNames.indexOf(item->text())];
		if ( item->checkState() == Qt::Checked )
			columnActivation << "true";
		else
			columnActivation << "false";
	}
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnOrder", orderedColumns);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnActivation", columnActivation);

	if ( e )
		e->accept();
}

void SoftwareListExporter::exportToASCII()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::exportToASCII()"));
#endif

	QTreeWidget *treeWidget = NULL;
	switch ( softwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidget = softwareList->treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = softwareList->treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = softwareList->treeWidgetSearchResults;
			break;
		default:
			break;
	}

	if ( !treeWidget )
		return;

	QFile exportFile(lineEditASCIIFile->text().replace("$ID$", softwareList->systemName));
	QString clipboardBuffer;
	QTextStream ts;
	if ( !checkBoxExportToClipboard->isChecked() ) {
		if ( !checkBoxOverwriteBlindly->isChecked() && exportFile.exists() ) {
			switch ( QMessageBox::question(this, tr("Confirm"), tr("Overwrite existing file?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
				case QMessageBox::Yes:
					break;
				case QMessageBox::No:
				default:
				       	return;
			}
		}

		if ( exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			ts.setDevice(&exportFile);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting current software-list in ASCII format to '%1'").arg(QFileInfo(exportFile).filePath()));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open ASCII export file '%1' for writing, please check permissions").arg(QFileInfo(exportFile).filePath()));
			return;
		}
	} else {
		ts.setString(&clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting current software-list in ASCII format to clipboard"));
	}

	setEnabled(false);

	progressBarExport->setRange(0, treeWidget->topLevelItemCount() * 2);
	progressBarExport->reset();
	progressBarExport->setValue(0);

	QMap<QString, int> maxColumnWidth;
	int maxLength = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ASCIIColumnWidth", 0).toInt();
	bool columnWidthLimited = maxLength > 0;
	QStringList orderedColumns = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnOrder", columnNamesUntranslated).toStringList();
	QStringList columnActivation = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnActivation", defaultColumnActivation).toStringList();
	QStringList headerNames;
	QList<int> columnIndexes;
	for (int i = 0; i < orderedColumns.count(); i++) {
		QString headerName = columnNames[columnNamesUntranslated.indexOf(orderedColumns[i])];
		if ( columnActivation[i] == "true" ) {
			maxColumnWidth[headerName] = columnWidthLimited ? QMC2_MIN(headerName.length(), maxLength) : headerName.length();
			headerNames << headerName;
			columnIndexes << columnNamesUntranslated.indexOf(orderedColumns[i]);
		}
	}

	int sortCriteria = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortCriteria", 0).toInt();
	QMultiMap<QString, QTreeWidgetItem *> exportMap;
	int count = 0;
	for (; count < treeWidget->topLevelItemCount(); count++) {
		progressBarExport->setValue(count);
		QTreeWidgetItem *item = treeWidget->topLevelItem(count);
		if ( item ) {
			exportMap.insert(item->text(sortCriteria), item);
			for (int j = 0; j < columnIndexes.count(); j++) {
				QString text = item->text(columnIndexes[j]);
				QString headerName = headerNames[j];
				if ( text.length() > maxColumnWidth[headerName] )
					maxColumnWidth[headerName] = columnWidthLimited ? QMC2_MIN(text.length(), maxLength) : text.length();
			}
		}
	}

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/IncludeColumnHeaders", true).toBool() ) {
		for (int i = 0; i < headerNames.count(); i++)
			ts << headerNames[i].leftJustified(maxColumnWidth[headerNames[i]], ' ', true) << "  ";
		ts << "\n";
		for (int i = 0; i < headerNames.count(); i++)
			ts << QString("-").leftJustified(maxColumnWidth[headerNames[i]], '-', true) << "  ";
		ts << "\n";
	}

	QMapIterator<QString, QTreeWidgetItem *> itExport(exportMap);
	bool ascendingOrder = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortOrder", 0).toInt() == 0;
	bool showMoreChars = maxLength > 3;
	while ( ascendingOrder ? itExport.hasNext() : itExport.hasPrevious() ) {
		if  ( ascendingOrder )
			itExport.next();
		else
			itExport.previous();
		QTreeWidgetItem *item = itExport.value();
		progressBarExport->setValue(++count);
		if ( item ) {
			for (int j = 0; j < columnIndexes.count(); j++) {
				QString s = item->text(columnIndexes[j]);
				int w = maxColumnWidth[headerNames[j]];
				if ( showMoreChars && s.length() > w )
					ts << s.left(w - 3).leftJustified(w, '.', true) << "  ";
				else
					ts << s.leftJustified(w, ' ', true) << "  ";
			}
			ts << "\n";
		}
	}

	progressBarExport->reset();

	if ( checkBoxExportToClipboard->isChecked() ) {
		qApp->clipboard()->setText(clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting current software-list in ASCII format to clipboard)"));
	} else {
		exportFile.close();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting current software-list in ASCII format to '%1')").arg(QFileInfo(exportFile).filePath()));
	}

	setEnabled(true);
}

void SoftwareListExporter::exportToCSV()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::exportToCSV()"));
#endif

	QTreeWidget *treeWidget = NULL;
	switch ( softwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidget = softwareList->treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = softwareList->treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = softwareList->treeWidgetSearchResults;
			break;
		default:
			break;
	}

	if ( !treeWidget )
		return;

	QFile exportFile(lineEditCSVFile->text().replace("$ID$", softwareList->systemName));
	QString clipboardBuffer;
	QTextStream ts;
	if ( !checkBoxExportToClipboard->isChecked() ) {
		if ( !checkBoxOverwriteBlindly->isChecked() && exportFile.exists() ) {
			switch ( QMessageBox::question(this, tr("Confirm"), tr("Overwrite existing file?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
				case QMessageBox::Yes:
					break;
				case QMessageBox::No:
				default:
					return;
			}
		}

		if ( exportFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			ts.setDevice(&exportFile);
			ts.setCodec(QTextCodec::codecForName("UTF-8"));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting current software-list in CSV format to '%1'").arg(QFileInfo(exportFile).filePath()));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open CSV export file '%1' for writing, please check permissions").arg(QFileInfo(exportFile).filePath()));
			return;
		}
	} else {
		ts.setString(&clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("exporting current software-list in CSV format to clipboard"));
	}
	
	setEnabled(false);

	progressBarExport->setRange(0, treeWidget->topLevelItemCount() * 2);
	progressBarExport->reset();
	progressBarExport->setValue(0);

	QStringList orderedColumns = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnOrder", columnNamesUntranslated).toStringList();
	QStringList columnActivation = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/ColumnActivation", defaultColumnActivation).toStringList();
	QStringList headerNames;
	QList<int> columnIndexes;
	for (int i = 0; i < orderedColumns.count(); i++) {
		QString headerName = columnNames[columnNamesUntranslated.indexOf(orderedColumns[i])];
		if ( columnActivation[i] == "true" ) {
			headerNames << headerName;
			columnIndexes << columnNamesUntranslated.indexOf(orderedColumns[i]);
		}
	}

	int sortCriteria = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortCriteria", 0).toInt();
	QMultiMap<QString, QTreeWidgetItem *> exportMap;
	int count = 0;
	for (; count < treeWidget->topLevelItemCount(); count++) {
		progressBarExport->setValue(count);
		QTreeWidgetItem *item = treeWidget->topLevelItem(count);
		if ( item )
			exportMap.insert(item->text(sortCriteria), item);
	}

	QString sep = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVSeparator", ";").toString();
	QString del = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/CSVDelimiter", "\"").toString();

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/IncludeColumnHeaders", true).toBool() ) {
		for (int i = 0; i < headerNames.count(); i++) ts << del << headerNames[i] << del << sep;
		ts << "\n" << del << del << "\n";
	}

	QMapIterator<QString, QTreeWidgetItem *> itExport(exportMap);
	bool ascendingOrder = qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareListExporter/SortOrder", 0).toInt() == 0;
	while ( ascendingOrder ? itExport.hasNext() : itExport.hasPrevious() ) {
		if  ( ascendingOrder )
			itExport.next();
		else
			itExport.previous();
		QTreeWidgetItem *item = itExport.value();
		progressBarExport->setValue(++count);
		if ( item ) {
			for (int j = 0; j < columnIndexes.count(); j++) {
				QString s = item->text(columnIndexes[j]);
				ts << del << s << del << sep;
			}
			ts << "\n";
		}
	}

	progressBarExport->reset();

	if ( checkBoxExportToClipboard->isChecked() ) {
		qApp->clipboard()->setText(clipboardBuffer);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting current software-list in CSV format to clipboard)"));
	} else {
		exportFile.close();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (exporting current software-list in CSV format to '%1')").arg(QFileInfo(exportFile).filePath()));
	}

	setEnabled(true);
}

void SoftwareListExporter::on_toolButtonBrowseASCIIFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListExporter::on_toolButtonBrowseASCIIFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose ASCII export file"), lineEditASCIIFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !s.isNull() )
		lineEditASCIIFile->setText(s);

	raise();
}

void SoftwareListExporter::on_toolButtonBrowseCSVFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListExporter::on_toolButtonBrowseCSVFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose CSV export file"), lineEditCSVFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !s.isNull() )
		lineEditCSVFile->setText(s);

	raise();
}

void SoftwareListExporter::on_pushButtonExport_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListExporter::on_pushButtonExport_clicked()");
#endif

	saveSettings();

	switch ( comboBoxOutputFormat->currentIndex() ) {
		case QMC2_SWLISTEXPORT_FORMAT_ASCII_INDEX:
			exportToASCII();
			break;

		case QMC2_SWLISTEXPORT_FORMAT_CSV_INDEX:
			exportToCSV();
			break;

		case QMC2_SWLISTEXPORT_FORMAT_ALL_INDEX:
			exportToASCII();
			exportToCSV();
			break;

		default:
			break;
	}
}

void SoftwareListExporter::on_comboBoxOutputFormat_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::on_comboBoxOutputFormat_currentIndexChanged(int index = %1)").arg(index));
#endif

	stackedWidgetOutputParams->setCurrentIndex(index);

	if ( index > QMC2_SWLISTEXPORT_FORMAT_CSV_INDEX && !exportListAutoCorrected ) {
		exportListAutoCorrected = true;
		checkBoxExportToClipboard->setChecked(false);
	}
	exportListAutoCorrected = false;
}

void SoftwareListExporter::on_checkBoxExportToClipboard_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListExporter::on_checkBoxExportToClipboard_toggled(bool enable = %1)").arg(enable));
#endif

	if ( enable ) {
		if ( comboBoxOutputFormat->currentIndex() > QMC2_SWLISTEXPORT_FORMAT_CSV_INDEX && !exportListAutoCorrected ) {
			exportListAutoCorrected = true;
			comboBoxOutputFormat->setCurrentIndex(0);
		}
	}

	exportListAutoCorrected = false;
}
