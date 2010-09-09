#ifndef _TEMPLATE_TOOL_H_
#define _TEMPLATE_TOOL_H_

#include "ui_template_tool.h"

class TemplateElement
{
	public:
		int elementType;
		int optionType;
		QString parentSection;
		QString name;
		QString defaultValue;
		QString nativeDescription;
		QMap<QString, QString> translatedDescriptions;
		QTreeWidgetItem *templateItem;
		
		TemplateElement(int eT = -1, int oT = -1, QString pS = QString(), QString n = QString(), QString dV = QString(), QString nD = QString(), QTreeWidgetItem *tI = NULL)
		{
			elementType = eT;
			optionType = oT;
			parentSection = pS;
			name = n;
			defaultValue = dV;
			nativeDescription = nD;
			// translatedDescriptions needs to be set outside the c'tor
			templateItem = tI;
		}
};

class TemplateTool : public QMainWindow, public Ui::TemplateTool
{
	Q_OBJECT

	public:
		TemplateTool(QWidget *parent = 0);
		~TemplateTool();

		QRect screenGeometry;
		QMenu *menuRecentTemplates;
		QString templateFileName;
		bool templateChanged;
		QMap<QString, TemplateElement> sectionMap;
		QMap<QString, QMap<QString, TemplateElement> > optionMap;

	public slots:
		void init();
		void setupTitle();
		void loadTemplate(QString);
		void saveTemplate(QString fileName = QString());

		// auto-connected slots
		void on_actionNew_triggered(bool);
		void on_actionOpen_triggered(bool);
		void on_actionRecent_triggered(bool);
		void on_actionRecent_hovered();
		void on_actionSave_triggered(bool);
		void on_actionSaveAs_triggered(bool);
		void on_actionCheckAgainstEmulator_triggered(bool);
		void on_actionCreateFromEmulator_triggered(bool);
		void on_actionCompareWithTemplate_triggered(bool);
		void on_lineEditEmulatorName_textChanged(const QString &);
		void on_lineEditEmulatorVersion_textChanged(const QString &);
		void on_lineEditTemplateVersion_textChanged(const QString &);
		void on_comboBoxElementType_currentIndexChanged(int);
		void on_comboBoxOptionType_currentIndexChanged(int);
		void on_comboBoxParentSection_currentIndexChanged(int);
		void on_pushButtonAdd_clicked();
		void on_pushButtonRemove_clicked();
		void on_pushButtonCopy_clicked();
		void on_pushButtonPaste_clicked();
		void on_pushButtonSave_clicked();
		void on_lineEditName_textChanged(const QString &);
		void on_lineEditDefaultValue_textChanged(const QString &);
		void on_plainTextEditNativeDescription_textChanged();
		void on_comboBoxLanguage_currentIndexChanged(int);
		void on_plainTextEditTranslatedDescription_textChanged();

	protected:
		void closeEvent(QCloseEvent *);
		void resizeEvent(QResizeEvent *);
		void moveEvent(QMoveEvent *);
};

#endif
