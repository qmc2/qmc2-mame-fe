#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>

#include "macros.h"
#include "qmc2main.h"
#include "settings.h"
#include "xmldbmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

XmlDatabaseManager::XmlDatabaseManager(QObject *parent)
	: QObject(parent)
{
}

XmlDatabaseManager::~XmlDatabaseManager()
{
}
