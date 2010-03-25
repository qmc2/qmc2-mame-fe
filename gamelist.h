#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QString>
#include <QTime>
#include <QProcess>
#include <QIcon>
#include <QFile>
#include <QTreeWidget>
#include <QTextStream>
#include "macros.h"

class Gamelist : public QObject
{
  Q_OBJECT

  public:
    QIcon qmc2SmallGhostImageIcon;
    QIcon qmc2UnknownImageIcon;
    QIcon qmc2UnknownBIOSImageIcon;
    QIcon qmc2CorrectImageIcon;
    QIcon qmc2CorrectBIOSImageIcon;
    QIcon qmc2MostlyCorrectImageIcon;
    QIcon qmc2MostlyCorrectBIOSImageIcon;
    QIcon qmc2IncorrectImageIcon;
    QIcon qmc2IncorrectBIOSImageIcon;
    QIcon qmc2NotFoundImageIcon;
    QIcon qmc2NotFoundBIOSImageIcon;
    QProcess *loadProc;
    QProcess *verifyProc;
    QString gamelistBuffer;
    QStringList xmlLines;
    static QStringList phraseTranslatorList;
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
    QString emulatorType;
    QString emulatorVersion;
    bool verifyCurrentOnly;
    QTreeWidgetItem *checkedItem;
    bool autoROMCheck;

    Gamelist(QObject *parent = 0);
    ~Gamelist();

  public slots:
    void load();
    void verify(bool currentOnly = FALSE);
    void save();
#if defined(QMC2_EMUTYPE_MAME)
    void loadCatverIni();
#endif
    void loadFavorites();
    void saveFavorites();
    void loadPlayHistory();
    void savePlayHistory();
    QString status();

    // process management
    void loadStarted();
    void loadFinished(int, QProcess::ExitStatus);
    void loadReadyReadStandardOutput();
    void loadReadyReadStandardError();
    void loadError(QProcess::ProcessError);
    void loadStateChanged(QProcess::ProcessState);
    void verifyStarted();
    void verifyFinished(int, QProcess::ExitStatus);
    void verifyReadyReadStandardOutput();
    void verifyReadyReadStandardError();
    void verifyError(QProcess::ProcessError);
    void verifyStateChanged(QProcess::ProcessState);

    // internal methods
    QString value(QString, QString, bool translate = FALSE);
    void parse();
    void parseGameDetail(QTreeWidgetItem *);
    void insertAttributeItems(QTreeWidgetItem *, QString, QStringList, QStringList, bool translate = FALSE);
    void enableWidgets(bool enable = TRUE);
    void filter();
    bool loadIcon(QString, QTreeWidgetItem *, bool checkOnly = FALSE, QString *fileName = NULL);
};

class GamelistItem : public QTreeWidgetItem
{
  public:
    GamelistItem(QTreeWidget *parentTreeWidget) : QTreeWidgetItem(parentTreeWidget, QTreeWidgetItem::UserType) {;}
    GamelistItem(QTreeWidgetItem *parentItem) : QTreeWidgetItem(parentItem, QTreeWidgetItem::UserType) {;}

    virtual bool operator<(const QTreeWidgetItem &) const;
};

#endif
