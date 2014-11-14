#include <QTextStream>
#include <QTextCodec>
#include <QApplication>

#include "infoprovider.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;
extern int emulatorMode;

InfoProvider::InfoProvider()
{
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

    // URL replacement regexp
    urlSectionRegExp = QString("[%1]+").arg(QLatin1String("\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*"));

    loadGameInfoDB();
    loadEmuInfoDB();
}

InfoProvider::~InfoProvider()
{
    clearGameInfoDB();
    clearEmuInfoDB();
}

void InfoProvider::clearGameInfoDB() {
    if ( !qmc2GameInfoDB.isEmpty() ) {
        QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
        QList<QByteArray *> deletedRecords;
        while ( it.hasNext() ) {
            it.next();
            if ( !deletedRecords.contains(it.value()) ) {
                if ( it.value() )
                    delete it.value();
                deletedRecords.append(it.value());
            }
        }
        deletedRecords.clear();
        qmc2GameInfoDB.clear();
        foreach (QString key, qmc2GameInfoSourceMap)
            qmc2GameInfoSourceMap.remove(key);
        qmc2GameInfoSourceMap.clear();
    }
}

void InfoProvider::clearEmuInfoDB() {
    if ( !qmc2EmuInfoDB.isEmpty() ) {
        QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
        QList<QByteArray *> deletedRecords;
        while ( it.hasNext() ) {
            it.next();
            if ( !deletedRecords.contains(it.value()) ) {
                if ( it.value() )
                    delete it.value();
                deletedRecords.append(it.value());
            }
        }
        deletedRecords.clear();
        qmc2EmuInfoDB.clear();
    }
}

void InfoProvider::loadGameInfoDB()
{
    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
    case QMC2_ARCADE_EMUMODE_UME:
        QMC2_ARCADE_LOG_STR(QObject::tr("Loading game info DB"));
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        QMC2_ARCADE_LOG_STR(QObject::tr("Loading machine info DB"));
        break;
    default:
        return;
    }

    qmc2InfoStopParser = false;

    clearGameInfoDB();

    QStringList gameInfoPathList;
    QStringList gameInfoSourceList;

    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
        gameInfoPathList << globalConfig->mameHistoryDat();
        gameInfoSourceList << "MAME";
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        gameInfoPathList << globalConfig->messSysinfoDat();
        gameInfoSourceList << "MESS";
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        gameInfoPathList << globalConfig->mameHistoryDat() << globalConfig->messSysinfoDat();
        gameInfoSourceList << "MAME" << "MESS";
        break;
    default:
        return;
    }

    for (int index = 0; index < gameInfoPathList.count(); index++) {
        if ( index % QMC2_ARCADE_LOAD_RESPONSE == 0 )
            qApp->processEvents();
        QString pathToGameInfoDB = gameInfoPathList[index];
        QString gameInfoSource = gameInfoSourceList[index];
        QFile gameInfoDB(pathToGameInfoDB);
        gameInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);
        if ( gameInfoDB.isOpen() ) {
            QTextStream ts(&gameInfoDB);
            ts.setCodec(QTextCodec::codecForName("UTF-8"));
            while ( !ts.atEnd() && !qmc2InfoStopParser ) {
                QString singleLine = ts.readLine();
                while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() )
                    singleLine = ts.readLine();
                if ( singleLine.simplified().startsWith("$info=") ) {
                    QStringList gameWords = singleLine.simplified().mid(6).split(",");
                    while ( !singleLine.simplified().startsWith("$bio") && !ts.atEnd() )
                        singleLine = ts.readLine();
                    if ( singleLine.simplified().startsWith("$bio") ) {
                        QString gameInfoString;
                        bool firstLine = true;
                        bool lastLineWasHeader = false;
                        while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
                            singleLine = ts.readLine();
                            if ( !singleLine.simplified().startsWith("$end") ) {
                                if ( !firstLine ) {
                                    if ( !lastLineWasHeader )
                                        gameInfoString.append(singleLine.trimmed() + "<br>");
                                    lastLineWasHeader = false;
                                } else if ( !singleLine.isEmpty() ) {
                                    gameInfoString.append("<h2>" + singleLine.trimmed() + "</h2>");
                                    firstLine = false;
                                    lastLineWasHeader = true;
                                }
                            }
                        }
                        if ( singleLine.simplified().startsWith("$end") ) {
                            // reduce the number of line breaks
                            gameInfoString.replace(QRegExp("(<br>){2,}"), "<p>");
                            if ( gameInfoString.endsWith("<p>") )
                                gameInfoString.remove(gameInfoString.length() - 3, gameInfoString.length() - 1);
                            QByteArray *gameInfo;
#if QT_VERSION >= 0x050000
                            gameInfo = new QByteArray(QTextCodec::codecForLocale()->fromUnicode(gameInfoString));
#else
                            gameInfo = new QByteArray(QTextCodec::codecForCStrings()->fromUnicode(gameInfoString));
#endif
                            for (int i = 0; i < gameWords.count(); i++) {
                                QString setName = gameWords[i];
                                if ( !setName.isEmpty() ) {
                                    qmc2GameInfoDB[setName] = gameInfo;
                                    qmc2GameInfoSourceMap.insert(gameInfoSource, setName);
                                }
                            }
                        } else {
                            switch ( emulatorMode ) {
                            case QMC2_ARCADE_EMUMODE_MAME:
                            case QMC2_ARCADE_EMUMODE_UME:
                                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$end' in game info DB %1").arg(pathToGameInfoDB));
                                break;
                            case QMC2_ARCADE_EMUMODE_MESS:
                                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$end' in machine info DB %1").arg(pathToGameInfoDB));
                                break;
                            }
                        }
                    } else {
                        switch ( emulatorMode ) {
                        case QMC2_ARCADE_EMUMODE_MAME:
                        case QMC2_ARCADE_EMUMODE_UME:
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$bio' in game info DB %1").arg(pathToGameInfoDB));
                            break;
                        case QMC2_ARCADE_EMUMODE_MESS:
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$bio' in machine info DB %1").arg(pathToGameInfoDB));
                            break;
                        }
                    }
                } else if ( !ts.atEnd() ) {
                    switch ( emulatorMode ) {
                    case QMC2_ARCADE_EMUMODE_MAME:
                    case QMC2_ARCADE_EMUMODE_UME:
                        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$info' in game info DB %1").arg(pathToGameInfoDB));
                        break;
                    case QMC2_ARCADE_EMUMODE_MESS:
                        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$info' in machine info DB %1").arg(pathToGameInfoDB));
                        break;
                    }
                }
            }
            gameInfoDB.close();
        } else {
            switch ( emulatorMode ) {
            case QMC2_ARCADE_EMUMODE_MAME:
            case QMC2_ARCADE_EMUMODE_UME:
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Can't open game info DB %1").arg(pathToGameInfoDB));
                break;
            case QMC2_ARCADE_EMUMODE_MESS:
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Can't open machine info DB %1").arg(pathToGameInfoDB));
                break;
            }
        }
    }

    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
    case QMC2_ARCADE_EMUMODE_UME:
        QMC2_ARCADE_LOG_STR(QObject::tr("Done (Loading game info DB)"));
        QMC2_ARCADE_LOG_STR(QObject::tr("%n game info record(s) loaded", "", qmc2GameInfoDB.count()));
        if ( qmc2InfoStopParser ) {
            QMC2_ARCADE_LOG_STR(QObject::tr("Invalidating game info DB"));
            clearGameInfoDB();
        }
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        QMC2_ARCADE_LOG_STR(QObject::tr("Done (Loading machine info DB)"));
        QMC2_ARCADE_LOG_STR(QObject::tr("%n machine info record(s) loaded", "", qmc2GameInfoDB.count()));
        if ( qmc2InfoStopParser ) {
            QMC2_ARCADE_LOG_STR(QObject::tr("Invalidating machine info DB"));
            clearGameInfoDB();
        }
        break;
    }
}

void InfoProvider::loadEmuInfoDB()
{
    QMC2_ARCADE_LOG_STR(QObject::tr("Loading emulator info DB"));

    qmc2InfoStopParser = false;

    clearEmuInfoDB();

    QStringList emuInfoPathList;

    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
        emuInfoPathList << globalConfig->mameInfoDat();
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        emuInfoPathList << globalConfig->messInfoDat();
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        emuInfoPathList << globalConfig->mameInfoDat() << globalConfig->messInfoDat();
        break;
    default:
        return;
    }

    for (int index = 0; index < emuInfoPathList.count(); index++) {
        if ( index % QMC2_ARCADE_LOAD_RESPONSE == 0 )
            qApp->processEvents();
        QString pathToEmuInfoDB = emuInfoPathList[index];
        QFile emuInfoDB(pathToEmuInfoDB);
        emuInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);
        if ( emuInfoDB.isOpen() ) {
            QTextStream ts(&emuInfoDB);
            ts.setCodec(QTextCodec::codecForName("UTF-8"));
            while ( !ts.atEnd() && !qmc2InfoStopParser ) {
                QString singleLine = ts.readLine();
                while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() )
                    singleLine = ts.readLine();
                if ( singleLine.simplified().startsWith("$info=") ) {
                    QStringList gameWords = singleLine.simplified().mid(6).split(",");
                    while ( !singleLine.simplified().startsWith("$mame") && !ts.atEnd() )
                        singleLine = ts.readLine();
                    if ( singleLine.simplified().startsWith("$mame") ) {
                        QString emuInfoString;
                        while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
                            singleLine = ts.readLine();
                            if ( !singleLine.simplified().startsWith("$end") )
                                emuInfoString.append(singleLine + "<br>");
                        }
                        if ( singleLine.simplified().startsWith("$end") ) {
                            // reduce the number of line breaks
                            emuInfoString.replace(QRegExp("(<br>){2,}"), "<p>");
                            if ( emuInfoString.startsWith("<br>") )
                                emuInfoString.remove(0, 4);
                            if ( emuInfoString.endsWith("<p>") )
                                emuInfoString.remove(emuInfoString.length() - 3, emuInfoString.length() - 1);
                            QByteArray *emuInfo;
#if QT_VERSION >= 0x050000
                            emuInfo = new QByteArray(QTextCodec::codecForLocale()->fromUnicode(emuInfoString));
#else
                            emuInfo = new QByteArray(QTextCodec::codecForCStrings()->fromUnicode(emuInfoString));
#endif
                            for (int i = 0; i < gameWords.count(); i++)
                                if ( !gameWords[i].isEmpty() )
                                    qmc2EmuInfoDB[gameWords[i]] = emuInfo;
                        } else
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$end' in emulator info DB %1").arg(pathToEmuInfoDB));
                    } else if ( !ts.atEnd() ) {
                        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$mame' in emulator info DB %1").arg(pathToEmuInfoDB));
                    }
                } else if ( !ts.atEnd() ) {
                    QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$info' in emulator info DB %1").arg(pathToEmuInfoDB));
                }
            }
            emuInfoDB.close();
        } else
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Can't open emulator info DB %1").arg(pathToEmuInfoDB));
    }

    QMC2_ARCADE_LOG_STR(QObject::tr("Done (Loading emulator info DB)"));
    QMC2_ARCADE_LOG_STR(QObject::tr("%n emulator info record(s) loaded", "", qmc2EmuInfoDB.count()));
    if ( qmc2InfoStopParser ) {
        QMC2_ARCADE_LOG_STR(QObject::tr("Invalidating emulator info DB"));
        clearEmuInfoDB();
    }
}

QString InfoProvider::requestInfo(const QString &id, InfoClass infoClass)
{
    QString infoText;

    switch ( infoClass ) {
    case InfoProvider::InfoClassGame:
        if ( qmc2GameInfoDB.contains(id) ) {
            QByteArray *newGameInfo = qmc2GameInfoDB[id];
            if ( newGameInfo ) {
                switch ( emulatorMode ) {
                case QMC2_ARCADE_EMUMODE_MAME:
                    infoText = QString(*newGameInfo).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
                    break;
                case QMC2_ARCADE_EMUMODE_UME:
                    infoText = QString(*newGameInfo);
                    if ( isMessGameInfo(id) )
                        infoText = messWikiToHtml(infoText);
                    else
                        infoText.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
                    break;
                case QMC2_ARCADE_EMUMODE_MESS:
                    infoText = QString(*newGameInfo);
                    infoText = messWikiToHtml(infoText);
                    break;
                }
            } else
                infoText = "<p>" + QObject::tr("no info available") + "</p>";
        } else
            infoText = "<p>" + QObject::tr("no info available") + "</p>";
        break;
    case InfoProvider::InfoClassEmu:
        if ( qmc2EmuInfoDB.contains(id) ) {
            QByteArray *newEmuInfo = qmc2EmuInfoDB[id];
            if ( newEmuInfo )
                infoText = QString(*newEmuInfo).replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
            else
                infoText = "<p>" + QObject::tr("no info available") + "</p>";
        } else
            infoText = "<p>" + QObject::tr("no info available") + "</p>";
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
