#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QByteArray>
#include <QBuffer>
#include <QXmlQuery>
#include <QMultiMap>

#include "missingdumpsviewer.h"
#include "romalyzer.h"
#include "settings.h"
#include "options.h"
#include "macros.h"

extern Settings *qmc2Config;
extern Options *qmc2Options;

MissingDumpsViewer::MissingDumpsViewer(QString settingsKey, QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	m_settingsKey = settingsKey;
	setVisible(false);
	setDefaultEmulator(false);
	setupUi(this);
}

void MissingDumpsViewer::on_toolButtonExportToDataFile_clicked()
{
	QString storedPath;
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath") )
		storedPath = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath").toString();
	QString dataFilePath = QFileDialog::getSaveFileName(this, tr("Choose data file to export to"), storedPath, tr("Data files (*.dat)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !dataFilePath.isNull() ) {
		QFile dataFile(dataFilePath);
		QFileInfo fi(dataFilePath);
		if ( dataFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			QTextStream ts(&dataFile);
			ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			ts << "<!DOCTYPE datafile PUBLIC \"-//Logiqx//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\">\n\n";
			ts << "<datafile>\n";
			ts << "\t<header>\n";
			ts << "\t\t<name>" << fi.completeBaseName() << "</name>\n";
			ts << "\t\t<description>" << fi.completeBaseName() << "</description>\n";
			ts << "\t\t<category>FIXDATFILE</category>\n";
			ts << "\t\t<version>" << QDateTime::currentDateTime().toString("MM/dd/yy hh:mm:ss") << "</version>\n";
			ts << "\t\t<date>" << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "</date>\n";
			ts << "\t\t<author>auto-create</author>\n";
			ts << "\t\t<email></email>\n";
			ts << "\t\t<homepage></homepage>\n";
			ts << "\t\t<url></url>\n";
			ts << "\t\t<comment>" << tr("Created by QMC2 v%1").arg(XSTR(QMC2_VERSION)) << "</comment>\n";
			ts << "\t</header>\n";
			QString mainEntityName = "machine";
			QMultiMap<QString, DumpRecord *> dumpMap;
			for (int i = 0; i < treeWidget->topLevelItemCount(); i++) {
				QTreeWidgetItem *item = treeWidget->topLevelItem(i);
				if ( !checkBoxSelectedDumpsOnly->isChecked() || item->isSelected() )
					dumpMap.insertMulti(item->text(QMC2_MDV_COLUMN_ID), new DumpRecord(item->text(QMC2_MDV_COLUMN_NAME), item->text(QMC2_MDV_COLUMN_TYPE), item->text(QMC2_MDV_COLUMN_SIZE), item->text(QMC2_MDV_COLUMN_CRC), item->text(QMC2_MDV_COLUMN_SHA1)));
			}
			foreach (QString id, dumpMap.uniqueKeys()) {
				if ( defaultEmulator() ) {
					QString sourcefile, isbios, cloneof, romof, sampleof, description, year, manufacturer, merge;
					QByteArray xmlDocument(ROMAlyzer::getXmlData(id, true).toLocal8Bit());
					QBuffer xmlQueryBuffer(&xmlDocument);
					xmlQueryBuffer.open(QIODevice::ReadOnly);
					QXmlQuery xmlQuery(QXmlQuery::XQuery10);
					xmlQuery.bindVariable("xmlDocument", &xmlQueryBuffer);
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@sourcefile/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&sourcefile);
					sourcefile = sourcefile.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@isbios/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&isbios);
					isbios = isbios.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@cloneof/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&cloneof);
					cloneof = cloneof.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@romof/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&romof);
					romof = romof.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@sampleof/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&sampleof);
					sampleof = sampleof.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/description/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&description);
					description = description.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/year/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&year);
					year = year.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/manufacturer/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&manufacturer);
					manufacturer = manufacturer.trimmed();
					ts << "\t<machine name=\"" << id << "\"";
					if ( !sourcefile.isEmpty() )
						ts << " sourcefile=\"" << sourcefile << "\"";
					if ( !isbios.isEmpty() && isbios != "no" )
						ts << " isbios=\"" << isbios << "\"";
					if ( !cloneof.isEmpty() )
						ts << " cloneof=\"" << cloneof << "\"";
					if ( !romof.isEmpty() )
						ts << " romof=\"" << romof << "\"";
					if ( !sampleof.isEmpty() )
						ts << " sampleof=\"" << sampleof << "\"";
					ts << ">\n";
					if ( !description.isEmpty() )
						ts << "\t\t<description>" << description << "</description>\n";
					if ( !year.isEmpty() )
						ts << "\t\t<year>" << year << "</year>\n";
					if ( !manufacturer.isEmpty() )
						ts << "\t\t<manufacturer>" << manufacturer << "</manufacturer>\n";
					foreach (DumpRecord *dr, dumpMap.values(id)) {
						if ( dr->type() == "ROM" ) {
							ts << "\t\t<rom name=\"" << dr->name() << "\"";
							xmlQuery.setQuery(QString("doc($xmlDocument)//%1/rom[@name='%2']/@merge/string()").arg(mainEntityName).arg(dr->name()));
							xmlQuery.evaluateTo(&merge);
							merge = merge.trimmed();
							if ( !merge.isEmpty() )
								ts << " merge=\"" << merge << "\"";
							if ( !dr->size().isEmpty() )
								ts << " size=\"" << dr->size() << "\"";
							if ( !dr->crc().isEmpty() )
								ts << " crc=\"" << dr->crc() << "\"";
							if ( !dr->sha1().isEmpty() )
								ts << " sha1=\"" << dr->sha1() << "\"";
							ts << "/>\n";
						} else {
							ts << "\t\t<disk name=\"" << dr->name() << "\"";
							xmlQuery.setQuery(QString("doc($xmlDocument)//%1/disk[@name='%2']/@merge/string()").arg(mainEntityName).arg(dr->name()));
							xmlQuery.evaluateTo(&merge);
							merge = merge.trimmed();
							if ( !merge.isEmpty() )
								ts << " merge=\"" << merge << "\"";
							if ( !dr->sha1().isEmpty() )
								ts << " sha1=\"" << dr->sha1() << "\"";;
							ts << "/>\n";
						}
						delete dr;
					}
					ts << "\t</machine>\n";
				} else {
					// FIXME "non-default emulator"
				}
			}
			dumpMap.clear();
			ts << "</datafile>\n";
			dataFile.close();
		} else {
			// FIXME "file error"
		}
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath", dataFilePath);
	}
}

void MissingDumpsViewer::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Geometry", QByteArray()).toByteArray());
	treeWidget->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/HeaderState", QByteArray()).toByteArray());
	checkBoxSelectedDumpsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SelectedDumpsOnly", false).toBool());
	if ( e )
		QDialog::showEvent(e);
}

void MissingDumpsViewer::hideEvent(QHideEvent *e)
{
	closeEvent(0);
	if ( e )
		QDialog::hideEvent(e);
}

void MissingDumpsViewer::closeEvent(QCloseEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/HeaderState", treeWidget->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SelectedDumpsOnly", checkBoxSelectedDumpsOnly->isChecked());
	if ( e )
		QDialog::closeEvent(e);
}
