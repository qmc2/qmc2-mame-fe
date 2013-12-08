#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QIcon>
#include <QTreeWidgetItem>
#endif
#include "macros.h"

class Gamelist : public QObject
{
  Q_OBJECT

  public:
    QIcon qmc2UnknownImageIcon;
    QIcon qmc2UnknownBIOSImageIcon;
    QIcon qmc2UnknownDeviceImageIcon;
    QIcon qmc2CorrectImageIcon;
    QIcon qmc2CorrectBIOSImageIcon;
    QIcon qmc2CorrectDeviceImageIcon;
    QIcon qmc2MostlyCorrectImageIcon;
    QIcon qmc2MostlyCorrectBIOSImageIcon;
    QIcon qmc2MostlyCorrectDeviceImageIcon;
    QIcon qmc2IncorrectImageIcon;
    QIcon qmc2IncorrectBIOSImageIcon;
    QIcon qmc2IncorrectDeviceImageIcon;
    QIcon qmc2NotFoundImageIcon;
    QIcon qmc2NotFoundBIOSImageIcon;
    QIcon qmc2NotFoundDeviceImageIcon;
    QProcess *loadProc;
    QProcess *verifyProc;
    QString gamelistBuffer;
    QStringList xmlLines;
    QStringList emulatorIdentifiers;
    static QStringList romTypeNames;
    static QStringList phraseTranslatorList;
    static QMap<QString, QString> reverseTranslation;
    QTime loadTimer;
    QTime verifyTimer;
    QTime parseTimer;
    QTime miscTimer;
    QFile romCache;
    QFile gamelistCache;
    QFile listXMLCache;
    QTextStream tsRomCache;
    QTextStream tsGamelistCache;
    QTextStream tsListXMLCache;
    int numTotalGames;
    int numGames;
    int numCorrectGames;
    int numMostlyCorrectGames;
    int numIncorrectGames;
    int numNotFoundGames;
    int numUnknownGames;
    int numSearchGames;
    int numDevices;
    int cachedGamesCounter;
    int numTaggedSets;
    QString emulatorType;
    QString emulatorVersion;
    bool verifyCurrentOnly;
    QTreeWidgetItem *checkedItem;
    bool autoRomCheck;
    QMap<QString, QString> driverNameMap;
    QMap<QString, char> gameStatusMap;
    QMap<QString, int> xmlGamePositionMap;
    bool mergeCategories;
    QChar oldRomState;
    QMap<QString, QString *> categoryNames;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    QMap<QString, QString *> versionNames;
#endif

    QString lookupDriverName(QString);
    QString romStatus(QString, bool translated = false);
    void clearCategoryNames();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    void clearVersionNames();
#endif

    Gamelist(QObject *parent = 0);
    ~Gamelist();

  public slots:
    void load();
    void verify(bool currentOnly = false);
    void save();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    void loadCatverIni();
    void createVersionView();
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
    void loadCategoryIni();
#endif
    void createCategoryView();
    void loadFavorites();
    void saveFavorites();
    void loadPlayHistory();
    void savePlayHistory();
    QString status();

    // process management
    void loadStarted();
    void loadFinished(int, QProcess::ExitStatus);
    void loadReadyReadStandardOutput();
    void loadError(QProcess::ProcessError);
    void loadStateChanged(QProcess::ProcessState);
    void verifyStarted();
    void verifyFinished(int, QProcess::ExitStatus);
    void verifyReadyReadStandardOutput();
    void verifyError(QProcess::ProcessError);
    void verifyStateChanged(QProcess::ProcessState);

    // internal methods
    QString value(QString, QString, bool translate = false);
    void parse();
    void parseGameDetail(QTreeWidgetItem *);
    void insertAttributeItems(QTreeWidgetItem *, QString, QStringList, QStringList, bool translate = false);
    void enableWidgets(bool enable = true);
    void filter(bool initial = false);
    bool loadIcon(QString, QTreeWidgetItem *, bool checkOnly = false, QString *fileName = NULL);
};

class GamelistItem : public QTreeWidgetItem
{
  public:
    GamelistItem(QTreeWidget *parentTreeWidget = 0) : QTreeWidgetItem(parentTreeWidget, QTreeWidgetItem::UserType) {;}
    GamelistItem(QTreeWidgetItem *parentItem) : QTreeWidgetItem(parentItem, QTreeWidgetItem::UserType) {;}

    virtual bool operator<(const QTreeWidgetItem &) const;
};

#endif
