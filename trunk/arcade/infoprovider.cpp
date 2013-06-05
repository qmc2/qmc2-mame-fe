
#include <QTextStream>
#include <QTextCodec>

#include "infoprovider.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

InfoProvider::InfoProvider()
{
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
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
#ifdef QMC2_DEBUG
  QMC2_ARCADE_LOG_STR(QString("DEBUG: InfoProvider::loadGameInfoDB()"));
#endif
  qmc2InfoStopParser = false;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  QMC2_ARCADE_LOG_STR(QObject::tr("loading game info DB"));
#elif defined(QMC2_EMUTYPE_MESS)
  QMC2_ARCADE_LOG_STR(QObject::tr("loading machine info DB"));
#endif

  // clear game/machine info DB
  clearGameInfoDB();

  bool compressData = globalConfig->compressGameInfoDB();
  QString pathToGameInfoDB = globalConfig->gameInfoDB();

  QFile gameInfoDB(pathToGameInfoDB);
  gameInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);

  if ( gameInfoDB.isOpen() ) {
    QTextStream ts(&gameInfoDB);
    ts.setCodec(QTextCodec::codecForName("UTF-8"));
    while ( !ts.atEnd() && !qmc2InfoStopParser ) {
      QString singleLine = ts.readLine();
      while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() ) {
        singleLine = ts.readLine();
      }
      if ( singleLine.simplified().startsWith("$info=") ) {
        QStringList gameWords = singleLine.simplified().mid(6).split(",");
        while ( !singleLine.simplified().startsWith("$bio") && !ts.atEnd() ) {
          singleLine = ts.readLine();
        }
        if ( singleLine.simplified().startsWith("$bio") ) {
          QString gameInfoString;
          bool firstLine = true;
          while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
            singleLine = ts.readLine();
            if ( !singleLine.simplified().startsWith("$end") ) {
              if ( !firstLine ) {
                  gameInfoString.append(singleLine.trimmed() + "<br>");
              } else if ( !singleLine.isEmpty() ) {
                gameInfoString.append("<b>" + singleLine.trimmed() + "</b><br>");
                firstLine = false;
              }
            }
          }
          if ( singleLine.simplified().startsWith("$end") ) {
            // reduce the number of line breaks
            gameInfoString.replace("<br><br><br><br>", "<p>");
            gameInfoString.replace("<br><br><br>", "<p>");
            gameInfoString.replace("<br><br>", "<p>");
            if ( gameInfoString.endsWith("<p>") )
              gameInfoString.remove(gameInfoString.length() - 3, gameInfoString.length() - 1);
            QByteArray *gameInfo;
#if QT_VERSION >= 0x050000
            if ( compressData )
              gameInfo = new QByteArray(QMC2_ARCADE_COMPRESS(QTextCodec::codecForLocale()->fromUnicode(gameInfoString))); 
            else
              gameInfo = new QByteArray(QTextCodec::codecForLocale()->fromUnicode(gameInfoString));
#else
            if ( compressData )
              gameInfo = new QByteArray(QMC2_ARCADE_COMPRESS(QTextCodec::codecForCStrings()->fromUnicode(gameInfoString))); 
            else
              gameInfo = new QByteArray(QTextCodec::codecForCStrings()->fromUnicode(gameInfoString));
#endif
            int i;
            for (i = 0; i < gameWords.count(); i++) {
              if ( !gameWords[i].isEmpty() )
                qmc2GameInfoDB[gameWords[i]] = gameInfo;
            }
          } else {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$end' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$end' in machine info DB %1").arg(pathToGameInfoDB));
#endif
          }
        } else {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
          QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$bio' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
          QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$bio' in machine info DB %1").arg(pathToGameInfoDB));
#endif
        }
      } else if ( !ts.atEnd() ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$info' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$info' in machine info DB %1").arg(pathToGameInfoDB));
#endif
      }
    }
    gameInfoDB.close();
  } else {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: can't open game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
    QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: can't open machine info DB %1").arg(pathToGameInfoDB));
#endif
  }
  
#if defined(QMC2_EMUTYPE_MESS)
  QMC2_ARCADE_LOG_STR(QObject::tr("done (loading machine info DB"));
  QMC2_ARCADE_LOG_STR(QObject::tr("%n machine info record(s) loaded", "", qmc2GameInfoDB.count()));
  if ( qmc2InfoStopParser ) {
    QMC2_ARCADE_LOG_STR(QObject::tr("invalidating machine info DB"));
#else
  QMC2_ARCADE_LOG_STR(QObject::tr("done (loading game info DB)"));
  QMC2_ARCADE_LOG_STR(QObject::tr("%n game info record(s) loaded", "", qmc2GameInfoDB.count()));
  if ( qmc2InfoStopParser ) {
    QMC2_ARCADE_LOG_STR(QObject::tr("invalidating game info DB"));
#endif
    clearGameInfoDB();
  }
}

void InfoProvider::loadEmuInfoDB()
{
#ifdef QMC2_DEBUG
  QMC2_ARCADE_LOG_STR(QString("DEBUG: InfoProvider::loadEmuInfoDB()"));
#endif
  qmc2InfoStopParser = false;
  QMC2_ARCADE_LOG_STR(QObject::tr("loading emulator info DB"));

  // clear emulator info DB
  clearEmuInfoDB();

  bool compressData = globalConfig->compressEmuInfoDB();
  QString pathToEmuInfoDB = globalConfig->emuInfoDB();

  QFile emuInfoDB(pathToEmuInfoDB);
  emuInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);

  if ( emuInfoDB.isOpen() ) {
    QTextStream ts(&emuInfoDB);
    ts.setCodec(QTextCodec::codecForName("UTF-8"));
    while ( !ts.atEnd() && !qmc2InfoStopParser ) {
      QString singleLine = ts.readLine();
      while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() ) {
        singleLine = ts.readLine();
      }
      if ( singleLine.simplified().startsWith("$info=") ) {
        QStringList gameWords = singleLine.simplified().mid(6).split(",");
        while ( !singleLine.simplified().startsWith("$mame") && !ts.atEnd() ) {
          singleLine = ts.readLine();
        }
        if ( singleLine.simplified().startsWith("$mame") ) {
          QString emuInfoString;
          while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
            singleLine = ts.readLine();
            if ( !singleLine.simplified().startsWith("$end") )
              emuInfoString.append(singleLine + "<br>");
          }
          if ( singleLine.simplified().startsWith("$end") ) {
            // convert "two (or more) empty lines" to a paragraph delimiter
            emuInfoString = emuInfoString.replace("<br><br><br>", "<p>").replace("<br><br>", "<p>");
            if ( emuInfoString.startsWith("<br>") )
              emuInfoString.remove(0, 4);
            if ( emuInfoString.endsWith("<p>") )
              emuInfoString.remove(emuInfoString.length() - 3, emuInfoString.length() - 1);
            QByteArray *emuInfo;
#if QT_VERSION >= 0x050000
            if ( compressData )
              emuInfo = new QByteArray(QMC2_COMPRESS(QTextCodec::codecForLocale()->fromUnicode(emuInfoString))); 
            else
              emuInfo = new QByteArray(QTextCodec::codecForLocale()->fromUnicode(emuInfoString));
#else
            if ( compressData )
              emuInfo = new QByteArray(QMC2_ARCADE_COMPRESS(QTextCodec::codecForCStrings()->fromUnicode(emuInfoString))); 
            else
              emuInfo = new QByteArray(QTextCodec::codecForCStrings()->fromUnicode(emuInfoString));
#endif
            int i;
            for (i = 0; i < gameWords.count(); i++) {
              if ( !gameWords[i].isEmpty() )
                qmc2EmuInfoDB[gameWords[i]] = emuInfo;
            }
          } else {
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$end' in emulator info DB %1").arg(pathToEmuInfoDB));
          }
        } else if ( !ts.atEnd() ) {
          QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$mame' in emulator info DB %1").arg(pathToEmuInfoDB));
        }
      } else if ( !ts.atEnd() ) {
        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: missing '$info' in emulator info DB %1").arg(pathToEmuInfoDB));
      }
    }
    emuInfoDB.close();
  } else
    QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: can't open emulator info DB %1").arg(pathToEmuInfoDB));

  QMC2_ARCADE_LOG_STR(QObject::tr("done (loading emulator info DB)"));
  QMC2_ARCADE_LOG_STR(QObject::tr("%n emulator info record(s) loaded", "", qmc2EmuInfoDB.count()));
  if ( qmc2InfoStopParser ) {
    QMC2_ARCADE_LOG_STR(QObject::tr("invalidating emulator info DB"));
    clearEmuInfoDB();
  }
}

QString InfoProvider::requestInfo(const QString &id, InfoClass infoClass ) {

  QString infoText("");
  // URL replacement regexp
  QString urlChar = QLatin1String("\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*");
  QString urlSectionRegExp = QString("[%1]+").arg(urlChar);
  switch(infoClass) {

    case InfoProvider::InfoClassGame :
      if ( qmc2GameInfoDB.contains(id) ) {
        QByteArray *newGameInfo = qmc2GameInfoDB[id];
        if ( newGameInfo ) {
#if defined(QMC2_EMUTYPE_MESS)
          if ( globalConfig->compressGameInfoDB() )
            infoText = QString(QMC2_ARCADE_UNCOMPRESS(*newGameInfo));
          else
            infoText = QString(*newGameInfo);
#else
          if ( globalConfig->compressGameInfoDB() )
            infoText = QString(QMC2_ARCADE_UNCOMPRESS(*newGameInfo)).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
          else
            infoText = QString(*newGameInfo).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
#endif
        } else
          infoText = "<p>" + QObject::tr("no info available") + "</p>";
      } else
        infoText = "<p>" + QObject::tr("no info available") + "</p>";
      break;

    case InfoProvider::InfoClassEmu :
      if ( qmc2EmuInfoDB.contains(id) ) {
        QByteArray *newEmuInfo = qmc2EmuInfoDB[id];
        if ( newEmuInfo ) {
          if ( globalConfig->compressEmuInfoDB() )
            infoText = QString(QMC2_ARCADE_UNCOMPRESS(*newEmuInfo)).replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
          else
            infoText = QString(*newEmuInfo).replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
        } else
          infoText = "<p>" + QObject::tr("no info available") + "</p>";        
      } else
        infoText = "<p>" + QObject::tr("no info available") + "</p>";
      break;
  }
  return infoText;
} 
