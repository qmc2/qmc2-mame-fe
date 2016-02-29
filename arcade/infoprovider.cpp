#include <qglobal.h>

#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QTextStream>
#include <QTextCodec>

#include "infoprovider.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;
extern int emulatorMode;

InfoProvider::InfoProvider()
{
	// URL replacement regexp
	urlSectionRegExp = QString("[%1]+").arg(QLatin1String("\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*"));

	// DAT-info database
	m_datInfoDb = new DatInfoDatabaseManager(0);

	// load (import) DAT-infos if required
	loadGameInfo();
	loadEmuInfo();
	loadSoftwareInfo();
}

InfoProvider::~InfoProvider()
{
	delete datInfoDb();
}

QString InfoProvider::requestInfo(const QString &id, InfoClass infoClass)
{
	QString infoText;

	switch ( infoClass ) {
	case InfoProvider::InfoClassGame:
		if ( datInfoDb()->existsMachineInfo(id) ) {
			QString newGameInfo = datInfoDb()->machineInfo(id);
			if ( !newGameInfo.isEmpty() ) {
				switch ( emulatorMode ) {
				case QMC2_ARCADE_EMUMODE_MAME:
				default:
					infoText = newGameInfo;
					if ( isMessGameInfo(id) )
						infoText = messWikiToHtml(infoText);
					else
						infoText.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
					break;
				}
			}
		}
		break;
	case InfoProvider::InfoClassEmu:
		if ( datInfoDb()->existsEmuInfo(id) ) {
			QString newEmuInfo = datInfoDb()->emuInfo(id);
			if ( !newEmuInfo.isEmpty() )
				infoText = newEmuInfo.replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
		}
		break;
	case InfoProvider::InfoClassSoft:
		// FIXME
		break;
	}

	return infoText;
}

QString &InfoProvider::messWikiToHtml(QString &wikiText)
{
	int ulLevel = 0;
	int olLevel = 0;
	bool tableOpen = false;
	bool preOn = false;
	bool codeOn = false;
	int preCounter = 0;
	QStringList wikiLines = wikiText.split("<p>");
	wikiText.clear();
	foreach (QString wikiLine, wikiLines) {
		QString wikiLineTrimmed = wikiLine.trimmed();
		if ( wikiLine.indexOf(QRegExp("\\s*<code>")) == 0 ) {
			codeOn = true;
			continue;
		}
		if ( wikiLine.indexOf(QRegExp("\\s*</code>")) == 0 )
			codeOn = false;
		bool listDetected = ( (wikiLineTrimmed.startsWith("* ") && wikiLine[wikiLine.indexOf("*") + 2] != ' ') || wikiLineTrimmed.startsWith("- ") );
		if ( wikiLine == "  * " || wikiLine == "  - " || wikiLine == "  *" || wikiLine == "  -" ) continue; // this is an "artifact"... ignore :)
		if ( !listDetected && (wikiLine.startsWith("  ") || codeOn) ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( wikiLine == "  "  && preCounter == 0 ) {
				preCounter++;
				wikiText += "\n";
				continue;
			}
			if ( !preOn ) {
				wikiText += "<p><table border=\"1\"><tr><td><pre>";
				preOn = true;
			}
			if ( wikiLine == "  " ) {
				wikiText += "\n";
				continue;
			}
		} else if ( preOn ) {
			preCounter++;
			wikiText += "</pre></td></tr></table><p>";
			preOn = codeOn = false;
		}
		int listDepth = 0;
		if ( listDetected ) listDepth = wikiLine.indexOf(QRegExp("[\\-\\*]")) / 2;
		if ( !preOn ) {
			wikiLine = wikiLineTrimmed;
			preCounter = 0;
		}
		if ( wikiLine.isEmpty() )
			continue;
		wikiLine.replace("<", "&lt;").replace(">", "&gt;");
		if ( wikiLine.startsWith("//") && wikiLine.endsWith("//") ) {
			wikiLine.replace(0, 2, "<i>");
			wikiLine.replace(wikiLine.length() - 2, 2, "</i>");
		}
		foreach (QString snippet, wikiLine.split("//")) {
			if ( snippet.indexOf(QRegExp("^.*(http:|https:|ftp:)$")) < 0 )
				wikiLine.replace(QString("//%1//").arg(snippet), QString("<i>%1</i>").arg(snippet));
		}
		wikiLine.replace(QRegExp("\\*\\*(.*)\\*\\*"), "<b>\\1</b>");
		wikiLine.replace(QRegExp("__(.*)__"), "<u>\\1</u>");
		wikiLine.replace(QRegExp("\\[\\[wp>([^\\]]*)\\]\\]"), QLatin1String("\\1 -- http://en.wikipedia.org/wiki/\\1"));
		foreach (QString snippet, wikiLine.split("[[")) {
			if ( snippet.indexOf(QRegExp("\\]\\]|\\|")) > 0 ) {
				QStringList subSnippets = snippet.split(QRegExp("\\]\\]|\\|"));
				wikiLine.replace(QString("[[%1]]").arg(snippet), subSnippets[0]);
			}
		}
		wikiLine.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
		if ( wikiLine.startsWith("&lt;h2&gt;======") && wikiLine.endsWith("======&lt;/h2&gt;") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h2>" + wikiLine.mid(16, wikiLine.length() - 33) + "</h2>";
		} else if ( wikiLine.startsWith("=====") && wikiLine.endsWith("=====") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }

			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h3>" + wikiLine.mid(6, wikiLine.length() - 12) + "</h3>";
		} else if ( wikiLine.startsWith("====") && wikiLine.endsWith("====") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h4>" + wikiLine.mid(5, wikiLine.length() - 10) + "</h4>";
		} else if ( wikiLine.startsWith("===") && wikiLine.endsWith("===") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<b>" + wikiLine.mid(4, wikiLine.length() - 8) + "</b>";
		} else if ( wikiLine.startsWith("==") && wikiLine.endsWith("==") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<b>" + wikiLine.mid(3, wikiLine.length() - 6) + "</b>";
		} else if ( wikiLine.indexOf(QRegExp("\\* \\S")) == 0 ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( listDepth > ulLevel ) {
				wikiText += "<ul style=\"list-style-type:square;\">";
				ulLevel++;
			} else if ( listDepth < ulLevel ) {
				wikiText += "</ul>";
				ulLevel--;
			}
			wikiText += "<li>" + wikiLine.mid(2) + "</li>";
		} else if ( wikiLine.indexOf(QRegExp("\\- \\S")) == 0 ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( listDepth > olLevel ) {
				wikiText += "<ol style=\"list-style-type:decimal;\">";
				olLevel++;
			} else if ( listDepth < olLevel ) {
				wikiText += "</ol>";
				olLevel--;
			}
			wikiText += "<li>" + wikiLine.mid(2) + "</li>";
		} else if ( ( wikiLine.startsWith("| ") && wikiLine.endsWith(" |") ) || ( wikiLine.startsWith("^ ") && wikiLine.endsWith(" |") ) ) {
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( !tableOpen ) { wikiText += "<p><table border=\"1\">"; tableOpen = true; }
			wikiText += "<tr>";
			foreach (QString cell, wikiLine.split(QRegExp("\\^|\\|"), QString::SkipEmptyParts)) wikiText += "<td>" + cell + "</td>";
			wikiText += "</tr>";
		} else if ( wikiLine.startsWith("^ ") && wikiLine.endsWith(" ^") ) {
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( !tableOpen ) { wikiText += "<p><table border=\"1\">"; tableOpen = true; }
			wikiText += "<tr>";
			foreach (QString cell, wikiLine.split("^", QString::SkipEmptyParts)) wikiText += "<td><b>" + cell + "</b></td>";
			wikiText += "</tr>";
		} else {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( preOn ) {
				wikiText += wikiLine.mid(2) + "\n";
			} else if ( codeOn ) {
				wikiText += wikiLine + "\n";
			} else
				wikiText += "<p>" + wikiLine + "</p>";
		}
	}

	if ( ulLevel > 0 )
		for (int i = 0; i < ulLevel; i++) wikiText += "</ul>";
	else if ( olLevel > 0 )
		for (int i = 0; i < olLevel; i++) wikiText += "</ol>";

	return wikiText;
}

void InfoProvider::loadGameInfo()
{
	QStringList pathList, emulatorList;

	switch ( emulatorMode ) {
	case QMC2_ARCADE_EMUMODE_MAME:
	default:
		pathList << QStringList() << globalConfig->mameHistoryDat() << globalConfig->messSysinfoDat();
		emulatorList << QStringList() << "MAME" << "MESS";
		break;
	}

	if ( datInfoDb()->machineInfoImportRequired(pathList) )
		datInfoDb()->importMachineInfo(pathList, emulatorList);
}

void InfoProvider::loadEmuInfo()
{
	QStringList pathList;

	switch ( emulatorMode ) {
	case QMC2_ARCADE_EMUMODE_MAME:
	default:
		pathList = QStringList() << globalConfig->mameInfoDat() << globalConfig->messInfoDat();
		break;
	}

	if ( datInfoDb()->emuInfoImportRequired(pathList) )
		datInfoDb()->importEmuInfo(pathList);
}

void InfoProvider::loadSoftwareInfo()
{
	QStringList pathList = QStringList() << globalConfig->softwareInfoDat();

	if ( datInfoDb()->softwareInfoImportRequired(pathList) )
		datInfoDb()->importSoftwareInfo(pathList);
}
