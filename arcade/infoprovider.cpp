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
            gameInfoString.replace(QRegExp("(<br>){2,}"), "<p>");
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
  qmc2InfoStopParser = false;
  QMC2_ARCADE_LOG_STR(QObject::tr("Loading emulator info DB"));

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
            // reduce the number of line breaks
            emuInfoString.replace(QRegExp("(<br>){2,}"), "<p>");
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
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: Missing '$end' in emulator info DB %1").arg(pathToEmuInfoDB));
          }
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

  QMC2_ARCADE_LOG_STR(QObject::tr("Done (Loading emulator info DB)"));
  QMC2_ARCADE_LOG_STR(QObject::tr("%n emulator info record(s) loaded", "", qmc2EmuInfoDB.count()));
  if ( qmc2InfoStopParser ) {
    QMC2_ARCADE_LOG_STR(QObject::tr("Invalidating emulator info DB"));
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
		switch ( emulatorMode ) {
			case QMC2_ARCADE_EMUMODE_MAME:
			case QMC2_ARCADE_EMUMODE_UME:
				if ( globalConfig->compressGameInfoDB() )
					infoText = QString(QMC2_ARCADE_UNCOMPRESS(*newGameInfo)).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
				else
					infoText = QString(*newGameInfo).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
				break;
			case QMC2_ARCADE_EMUMODE_MESS:
				if ( globalConfig->compressGameInfoDB() )
					infoText = QString(QMC2_ARCADE_UNCOMPRESS(*newGameInfo));
				else
					infoText = QString(*newGameInfo);
				break;
		}
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
