/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Graphics Dojo project on Qt Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 or 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#ifndef HTML_EDITOR_H
#define HTML_EDITOR_H

#include <QMainWindow>
#include <QFrame>
#include <QProgressBar>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QBuffer>
#include <QByteArray>
#include <QXmlQuery>
#include <QStringList>
#include "highlighter.h"

class Ui_HTMLEditorMainWindow;
class Ui_InsertHtmlDialog;
class Ui_TablePropertyDialog;

class QLabel;
class QSlider;
class QUrl;

class HtmlEditor : public QMainWindow
{
	Q_OBJECT

	public:
		QMap<QString, QString> templateMap;
		QString myEditorName;
		bool isEmbeddedEditor;
		QString loadedContent;
		QFrame *frameCornerWidget;
		QMenu *menuSettings;
		QToolButton *toolButtonSettings;
		QAction *actionHideMenu;
		QAction *actionReadOnly;
		QAction *actionShowHTML;
		QProgressBar *loadProgress;
		bool loadActive;
		bool stopLoading;
		bool loadSuccess;
		QStringList imageTypes;
		QBuffer *xmlQueryBuffer;
		QByteArray *xmlDocument;
		QXmlQuery xmlQuery;
		QStringList xmlResult;

		HtmlEditor(QString, bool embedded = false, QWidget *parent = 0);
		~HtmlEditor();

		static QUrl guessUrlFromString(const QString &string);
		QString &noScript(QString &);

	protected:
		virtual void closeEvent(QCloseEvent *e);

	private:
		void execCommand(const QString&);
		void execCommand(const QString &cmd, const QString &arg);
		bool queryCommandState(const QString&);

	public slots:
		void loadStarted();
		void loadProgressed(int);
		void loadFinished(bool);
		void fileNew();
		void fileNewFromTemplate();
		void fileRevert();
		void fileOpen();
		bool fileSave();
		bool fileSaveAs();
		void fileOpenInBrowser();
		void editSelectAll();
		void styleParagraph();
		void styleHeading1();
		void styleHeading2();
		void styleHeading3();
		void styleHeading4();
		void styleHeading5();
		void styleHeading6();
		void stylePreformatted();
		void styleAddress();
		void formatStrikeThrough();
		void formatAlignLeft();
		void formatAlignCenter();
		void formatAlignRight();
		void formatAlignJustify();
		void formatIncreaseIndent();
		void formatDecreaseIndent();
		void formatNumberedList();
		void formatBulletedList();
		void formatFontName();
		void formatFontSize();
		void formatTextColor();
		void formatBackgroundColor();
		void insertImageFromFile();
		void insertImageFromUrl();
		void createLink();
		void insertHtml();
		void insertTable();
		void zoomOut();
		void zoomIn();
		void adjustActions();
		void adjustWYSIWYG();
		void adjustHTML();
		void changeTab(int);
		void openLink(const QUrl&);
		void changeZoom(int);
		void linkHovered(const QString &, const QString &, const QString &);
		void adjustIconSizes();
		bool load(const QString &f);
		bool loadCurrent();
		bool loadTemplate(const QString &f);
		bool save();
		void setCurrentFileName(const QString &fileName);
		void setCurrentTemplateName(const QString &templateName);
		void hideTearOffMenus();
		void enableFileNewFromTemplateAction(bool enable = true);
		void checkRevertStatus();
		void setContentEditable(bool);
		void setLoadActive() { loadActive = true; }
		void setLoadInactive() { loadActive = false; }
		void setLoadSuccess(bool success) { loadSuccess = success; }
		void javaScriptWindowObjectCleared();
		void showHtmlTab(bool enable = true);
		void closeXmlBuffer();
		void clearContent();

		// helper functions (not only) for template use
		bool loadCurrentTemplate();
		bool reloadTemplate() { return loadCurrentTemplate(); }
		QString getIconData();
		QString getColor(QString currentColor = QString());
		QString getImage(QString currentImage = QString());
		bool isZippedImage(QString imageType);
		QString getImageData(QString imageType);
		bool queryXml(QString id, QString queryString, bool sort = true, QString systemEntityName = QString()) { return queryLocalXml(id, queryString, sort, systemEntityName); }
		bool queryLocalXml(QString id, QString queryString, bool sort = true, QString systemEntityName = QString());
		QStringList getXmlResult() { return xmlResult; }
		bool isBios(QString id);
		bool isDevice(QString id);
		QString romStatus(QString id, bool translated = false);
		int rank(QString id);
		QString comment(QString id);
		QString systemInfo(QString id);
		QString emuInfo(QString id);
		QStringList videoSnapUrls(QString id);
		QString softwareInfo(QString list, QString id);
		void openLinkInDefaultBrowser(QString linkUrl);
		QStringList customSystemArtwork();
		QStringList customSoftwareArtwork();
		bool customArtworkZipped(QString artworkName);
		QString customSystemArtworkUrl(QString id, QString artworkName);
		QString customSystemArtworkData(QString id, QString artworkName);
		QString customSoftwareArtworkUrl(QString listName, QString softwareName, QString artworkName);
		QString customSoftwareArtworkData(QString listName, QString softwareName, QString artworkName);
		bool systemManualExists(const QString &id);
		void openSystemManual(const QString &id);
		bool softwareManualExists(const QString &list, const QString &id);
		void openSoftwareManual(const QString &list, const QString &id);

	private:
		Ui_HTMLEditorMainWindow *ui;
		QString fileName;
		QString templateName;
		bool htmlDirty;
		bool wysiwygDirty;
		bool generateEmptyContent;
		bool localModified;
		QString emptyContent;
		QLabel *zoomLabel;
		QSlider *zoomSlider;
		Highlighter *highlighter;
		Ui_InsertHtmlDialog *ui_dialog;
		QDialog *insertHtmlDialog;
		Ui_TablePropertyDialog *ui_tablePropertyDialog;
		QDialog *tablePropertyDialog;
};

#endif // HTML_EDITOR_H
