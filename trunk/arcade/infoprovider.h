#ifndef INFOPROVIDER_H
#define INFOPROVIDER_H

#include <QMap>
#include <QString>

class InfoProvider
{
public:
    explicit InfoProvider();
    virtual ~InfoProvider();

    enum InfoClass { InfoClassGame, InfoClassEmu };
    QString requestInfo(const QString &, InfoClass);

private:
    void clearGameInfoDB();
    void clearEmuInfoDB();
    void loadGameInfoDB();
    void loadEmuInfoDB();
    bool qmc2InfoStopParser;
    QMap<QString, QByteArray *> qmc2EmuInfoDB;
    QMap<QString, QByteArray *> qmc2GameInfoDB;
    QString urlSectionRegExp;
};

#endif
