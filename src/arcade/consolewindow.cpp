#include "consolewindow.h"
#include "arcadesettings.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;

ConsoleWindow::ConsoleWindow(QWidget *parent) :
	QPlainTextEdit(parent)
{
	setLineWrapMode(QPlainTextEdit::NoWrap);
	setReadOnly(true);
	setWindowTitle(QMC2_ARCADE_APP_TITLE + " " + tr("Console"));
	setFont(QFont("Courier", 10));
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::loadSettings()
{
	restoreGeometry(globalConfig->consoleGeometry());
}

void ConsoleWindow::saveSettings()
{
	globalConfig->setConsoleGeometry(saveGeometry());
}
