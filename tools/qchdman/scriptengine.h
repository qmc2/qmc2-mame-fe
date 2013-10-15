#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtScript>
#include <QtScriptTools>
#include <QObject>
#include <QStringList>
#include <QMap>

#include "projectwidget.h"
#include "scriptwidget.h"
#include "macros.h"

class ScriptWidget;
class ProjectWidget;

class ScriptEngine : public QObject
{
    Q_OBJECT

public:
    bool externalStop;

    explicit ScriptEngine(ScriptWidget *parent);
    virtual ~ScriptEngine();

    void runScript(QString script);
    void stopScript();

public slots:
    void log(QString message);

    void projectCreate(QString id, QString type);
    void projectCreateFromFile(QString id, QString fileName);
    void projectCreateFromString(QString id, QString buffer);
    void projectDestroy(QString id);
    QString projectStatus(QString id);

    // Info
    void projectSetInfoInputFile(QString id, QString file);
    void projectSetInfoVerbose(QString id, bool verbose);

    void runProjects(QString idList);
    void stopProjects(QString idList);
    void syncProjects(QString idList);

private:
    QScriptEngine *mEngine;
    QScriptEngineDebugger *mEngineDebugger;
    QMap<QString, ProjectWidget *> mProjectMap;
    ScriptWidget *mScriptWidget;
};

#endif // SCRIPTENGINE_H
