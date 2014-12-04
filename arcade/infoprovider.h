#ifndef INFOPROVIDER_H
#define INFOPROVIDER_H

#include <QString>

#include "datinfodbmgr.h"

class InfoProvider
{
public:
    explicit InfoProvider();
    virtual ~InfoProvider();

    enum InfoClass { InfoClassGame, InfoClassEmu, InfoClassSoft };
    QString requestInfo(const QString &, InfoClass);

    bool isMessGameInfo(QString id) { return datInfoDb()->gameInfoEmulator(id) == "MESS"; }
    bool isMameGameInfo(QString id) { return datInfoDb()->gameInfoEmulator(id) == "MAME"; }

    QString &messWikiToHtml(QString &);
    DatInfoDatabaseManager *datInfoDb() { return m_datInfoDb; }

private:
    void loadGameInfo();
    void loadEmuInfo();
    void loadSoftwareInfo();
    QString urlSectionRegExp;
    DatInfoDatabaseManager *m_datInfoDb;
};

#endif
