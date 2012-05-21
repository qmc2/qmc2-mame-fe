#include <QStringList>
#include <QString>
#include <QMap>
#include <QTimer>

class Syncer : public QObject
{
    Q_OBJECT

public:
    QMap<QString, QString> sectionDescriptions, targetSectionDescriptions;
    QMap<QString, QString> optionDescriptions, targetOptionDescriptions;
    QString sourceTemplate, targetTemplate, language;

    Syncer(QString source, QString target, QString lang)
    {
        sourceTemplate = source;
        targetTemplate = target;
        language = lang;
    }

    ~Syncer() { ; }

public slots:
    void syncTemplates();
};
