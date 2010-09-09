#include <QtGui>

#include "template_tool.h"
#include "macros.h"

TemplateTool *mainWindow;

int main(int argc, char *argv[])
{
	DEBUG_CODE(qDebug("DEBUG: main(int argc = %d, char *argv[] = ...)", argc);)

	QApplication templateToolApp(argc, argv);
	mainWindow = new TemplateTool(0);
	mainWindow->show();
	return templateToolApp.exec();
}
