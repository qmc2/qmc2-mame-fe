#ifndef INFOPROVIDER_H
#define INFOPROVIDER_H

#include <QMap>
#include <QMultiMap>
#include <QString>

class InfoProvider
{
public:
    explicit InfoProvider();
    virtual ~InfoProvider();

    enum InfoClass { InfoClassGame, InfoClassEmu };
    QString requestInfo(const QString &, InfoClass);

    bool isMessGameInfo(QString setName) { return qmc2GameInfoSourceMap.values("MESS").contains(setName); }
    bool isMameGameInfo(QString setName) { return qmc2GameInfoSourceMap.values("MAME").contains(setName); }

    QString &messWikiToHtml(QString &);

private:
    void clearGameInfoDB();
    void clearEmuInfoDB();
    void loadGameInfoDB();
    void loadEmuInfoDB();
    bool qmc2InfoStopParser;
    QMap<QString, QByteArray *> qmc2EmuInfoDB;
    QMap<QString, QByteArray *> qmc2GameInfoDB;
    QMultiMap<QString, QString> qmc2GameInfoSourceMap;
    QString urlSectionRegExp;
};

#endif
