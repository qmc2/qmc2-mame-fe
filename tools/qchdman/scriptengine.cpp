#include "scriptengine.h"

ScriptEngine::ScriptEngine(ScriptWidget *parent) :
    QObject(parent)
{
    mEngine = new QScriptEngine(this);
    mEngine->globalObject().setProperty("scriptEngine", mEngine->newQObject(this));
    mScriptWidget = parent;
}

ScriptEngine::~ScriptEngine()
{
    delete mEngine;
}

void ScriptEngine::runScript(QString script)
{
    mEngine->evaluate(script);
}

void ScriptEngine::stopScript()
{
    if ( mEngine->isEvaluating() )
        mEngine->abortEvaluation();
}

void ScriptEngine::log(QString message)
{
    mScriptWidget->log(message);
}

void ScriptEngine::projectCreate(QString id, QString type)
{
    // FIXME
}

void ScriptEngine::projectCreateFromFile(QString id, QString fileName)
{
    // FIXME
}

void ScriptEngine::projectCreateFromString(QString id, QString buffer)
{
    // FIXME
}

void ScriptEngine::projectDestroy(QString id)
{
    // FIXME
}

QString ScriptEngine::projectStatus(QString id)
{
    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->status;
    else
        return QCHDMAN_PRJSTAT_UNKNOWN;
}

void ScriptEngine::runProjects(QString idList)
{
    // FIXME
}

void ScriptEngine::stopProjects(QString idList)
{
    // FIXME
}

void ScriptEngine::syncProjects(QString idList)
{
    // FIXME
}
