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
    // print a message to the script-log
    void log(QString message);

    // progress-bar
    void progressSetRange(int min, int max);
    void progressSetValue(int value);

    // project creation / destruction / status
    void projectCreate(QString id, QString type);
    void projectCreateFromFile(QString id, QString fileName);
    void projectCreateFromString(QString id, QString buffer);
    void projectDestroy(QString id);
    QString projectStatus(QString id);

    // properties for project-type 'Info'
    void projectSetInfoInputFile(QString id, QString file);
    void projectSetInfoVerbose(QString id, bool verbose);

    // properties for project-type 'Verify'
    void projectSetVerifyInputFile(QString id, QString file);
    void projectSetVerifyParentInputFile(QString id, QString file);

    // properties for project-type 'Copy'
    void projectSetCopyInputFile(QString id, QString file);
    void projectSetCopyParentInputFile(QString id, QString file);
    void projectSetCopyOutputFile(QString id, QString file);
    void projectSetCopyParentOutputFile(QString id, QString file);
    void projectSetCopyForce(QString id, bool force);
    void projectSetCopyInputStartByte(QString id, int byte);
    void projectSetCopyInputStartHunk(QString id, int hunk);
    void projectSetCopyInputBytes(QString id, int bytes);
    void projectSetCopyInputHunks(QString id, int hunks);
    void projectSetCopyCompressors(QString id, QString compressors);

    // properties for project-type 'CreateRaw'

    // properties for project-type 'CreateHD'

    // properties for project-type 'CreateCD'

    // properties for project-type 'CreateLD'

    // properties for project-type 'ExtractRaw'

    // properties for project-type 'ExtractHD'

    // properties for project-type 'ExtractCD'

    // properties for project-type 'ExtractLD'

    // properties for project-type 'DumpMeta'

    // properties for project-type 'AddMeta'

    // properties for project-type 'DelMeta'

    // run / stop / synchronize / destroy projects
    void runProjects(QString idList);
    void stopProjects(QString idList);
    void syncProjects(QString idList);
    void destroyProjects(QString idList);

private:
    QScriptEngine *mEngine;
    QScriptEngineDebugger *mEngineDebugger;
    QMap<QString, ProjectWidget *> mProjectMap;
    ScriptWidget *mScriptWidget;
    QStringList mErrorStates;

    void cleanUpProjects();
};

#endif // SCRIPTENGINE_H
