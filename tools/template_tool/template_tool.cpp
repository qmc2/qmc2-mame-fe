#include <QtGui>

#include "template_tool.h"
#include "macros.h"

TemplateTool::TemplateTool(QWidget *parent) : QMainWindow(parent)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::TemplateTool(QWidget *parent = %p)", parent);)

	screenGeometry = qApp->desktop()->screenGeometry(parent);
	setupUi(this);
	setUpdatesEnabled(false);
	show();
	move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 2);
	hide();
	QList<int> splitterSizes;
	splitterSizes << width()/2 << width()/2;
	splitter->setSizes(splitterSizes);
	menuRecentTemplates = new QMenu(this);
	QAction *action = new QAction(tr("No templates"), this);
	action->setEnabled(false);
	menuRecentTemplates->addAction(action);
	actionRecent->setMenu(menuRecentTemplates);
	on_comboBoxElementType_currentIndexChanged(0);
	templateFileName = tr("untitled.xml");
	templateChanged = false;
	setupTitle();
	QTimer::singleShot(0, this, SLOT(init()));
}

TemplateTool::~TemplateTool()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::~TemplateTool()");)

}

void TemplateTool::init()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::init()");)

	QStringList args = qApp->arguments();

	DEBUG_CODE(int i = 0; foreach (QString arg, args) qDebug("DEBUG: TemplateTool::init(): args[%d] = %s", i++, (const char *)arg.toAscii());)

	// '-t' or '-template'
	int templateArgumentIndex = args.indexOf("-template");
	if ( templateArgumentIndex < 0 )
		templateArgumentIndex = args.indexOf("-t");
	if ( templateArgumentIndex > 0 && args.count() > templateArgumentIndex + 1 )
		loadTemplate(args[templateArgumentIndex + 1]);
	else if ( templateArgumentIndex > 0 )
		qDebug("WARNING: option %s requires an argument (template file) -- ignored", (const char *)args[templateArgumentIndex].toAscii());

	show();
	setUpdatesEnabled(true);
}

void TemplateTool::setupTitle()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::setupTitle()");)

	setWindowTitle(tr("QMC2 Template Tool v%1 -- %2%3").arg(VERSION).arg(templateFileName).arg(templateChanged ? tr(" (changed)") : ""));
}

void TemplateTool::loadTemplate(QString fileName)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::loadTemplate(QString fileName = %s)", (const char *)fileName.toAscii());)

	templateFileName = fileName;
	setupTitle();

	QFile templateFile(templateFileName);
	if ( templateFile.open(QFile::ReadOnly) ) {
		QXmlStreamReader xmlReader(&templateFile);
		QString templateEmulator, templateVersion, templateFormat;
		while ( !xmlReader.atEnd() ) {
			xmlReader.readNext();
			if ( xmlReader.hasError() ) {
				qDebug("FATAL: XML error reading template: '%s' in file '%s' at line %d, column %d",
					(const char *)xmlReader.errorString().toAscii(),
					(const char *)templateFileName.toAscii(),
					(int)xmlReader.lineNumber(),
					(int)xmlReader.columnNumber());
			} else {
				if ( xmlReader.isStartElement() ) {
					QString eT = xmlReader.name().toString();
					QXmlStreamAttributes attributes = xmlReader.attributes();
					QString name = attributes.value("name").toString();
					if ( eT == "section" ) {
					} else if ( eT == "option" ) {
						QString oT = attributes.value("type").toString();
						QString dV = attributes.value("default").toString();
					} else if ( eT == "template" ) {
						templateEmulator = attributes.value("emulator").toString();
						templateVersion = attributes.value("version").toString();
						templateFormat = attributes.value("format").toString();
					}
				}
			}
		}
		templateFile.close();
	} else
		qDebug("FATAL: can't open '%s' for reading", (const char *)templateFileName.toAscii());
}

void TemplateTool::saveTemplate(QString fileName)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::saveTemplate(QString fileName = %s)", (const char *)fileName.toAscii());)

	templateFileName = fileName;
	templateChanged = false;
	setupTitle();
}

void TemplateTool::on_actionNew_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionNew_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionOpen_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionOpen_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionRecent_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionRecent_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionRecent_hovered()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionRecent_hovered()");)

}

void TemplateTool::on_actionSave_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionSave_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionSaveAs_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionSaveAs_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionCheckAgainstEmulator_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionCheckAgainstEmulator_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionCreateFromEmulator_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionCreateFromEmulator_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_actionCompareWithTemplate_triggered(bool checked)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_actionCompareWithTemplate_triggered(bool checked = %d)", checked));

}

void TemplateTool::on_pushButtonAdd_clicked()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_pushButtonAdd_clicked()");)

}

void TemplateTool::on_pushButtonRemove_clicked()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_pushButtonRemove_clicked()");)

}

void TemplateTool::on_pushButtonCopy_clicked()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_pushButtonCopy_clicked()");)

}

void TemplateTool::on_pushButtonPaste_clicked()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_pushButtonPaste_clicked()");)

}

void TemplateTool::on_pushButtonSave_clicked()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_pushButtonSave_clicked()");)

}

void TemplateTool::on_lineEditEmulatorName_textChanged(const QString &text)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_lineEditEmulatorName_textChanged(const QString &text = %s)", (const char *)text.toAscii());)

}

void TemplateTool::on_lineEditEmulatorVersion_textChanged(const QString &text)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_lineEditEmulatorVersion_textChanged(const QString &text = %s)", (const char *)text.toAscii());)

}

void TemplateTool::on_lineEditTemplateVersion_textChanged(const QString &text)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_lineEditTemplateVersion_textChanged(const QString &text = %s)", (const char *)text.toAscii());)

}


void TemplateTool::on_lineEditName_textChanged(const QString &text)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_lineEditName_textChanged(const QString &text = %s)", (const char *)text.toAscii());)

}

void TemplateTool::on_lineEditDefaultValue_textChanged(const QString &text)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_lineEditDefaultValue_textChanged(const QString &text = %s)", (const char *)text.toAscii());)

}

void TemplateTool::on_plainTextEditNativeDescription_textChanged()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_plainTextEditNativeDescription_textChanged()");)

}

void TemplateTool::on_comboBoxLanguage_currentIndexChanged(int index)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_comboBoxLanguage_currentIndexChanged(int index = %d)", index);)

}

void TemplateTool::on_plainTextEditTranslatedDescription_textChanged()
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_plainTextEditTranslatedDescription_textChanged()");)

}

void TemplateTool::on_comboBoxElementType_currentIndexChanged(int index)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_comboBoxElementType_currentIndexChanged(int index = %d)", index);)

	switch ( index ) {
		case 0:
			labelOptionType->hide();
			comboBoxOptionType->hide();
      			labelDefaultValue->hide();
      			lineEditDefaultValue->hide();
			labelParentSection->hide();
			comboBoxParentSection->hide();
      			break;

      		case 1:
      		default:
      			labelOptionType->show();
      			comboBoxOptionType->show();
      			labelDefaultValue->show();
      			lineEditDefaultValue->show();
			labelParentSection->show();
			comboBoxParentSection->show();
      			break;
      	}
}

void TemplateTool::on_comboBoxOptionType_currentIndexChanged(int index)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_comboBoxOptionType_currentIndexChanged(int index = %d)", index);)

}

void TemplateTool::on_comboBoxParentSection_currentIndexChanged(int index)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::on_comboBoxParentSection_currentIndexChanged(int index = %d)", index);)

}

void TemplateTool::closeEvent(QCloseEvent *e)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::closeEvent(QCloseEvent *e = %p)", e);)

	QMainWindow::closeEvent(e);
}

void TemplateTool::resizeEvent(QResizeEvent *e)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::resizeEvent(QResizeEvent *e = %p): e->size() = %dx%d", e, e->size().width(), e->size().height());)

	QMainWindow::resizeEvent(e);
}

void TemplateTool::moveEvent(QMoveEvent *e)
{
	DEBUG_CODE(qDebug("DEBUG: TemplateTool::moveEvent(QMoveEvent *e = %p): e->pos() = %d,%d", e, e->pos().x(), e->pos().y());)

	QMainWindow::moveEvent(e);
}

