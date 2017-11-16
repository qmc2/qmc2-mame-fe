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

#include <QtGui>
#include <QtWebKit>
#include <QHBoxLayout>
#include <QTest>
#include <QTabBar>
#include <QMap>
#include <QHash>
#include <QList>
#include <QPixmap>
#include <QIcon>
#include <QTreeWidgetItem>
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QCache>
#include <QHash>
#include <QRegExp>
#include <QDesktopServices>
#if QT_VERSION >= 0x050000
#include <QDesktopWidget>
#include <QToolButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QToolTip>
#include <QtWebKitWidgets/QWebFrame>
#endif
#include <QListWidgetItem>

#include <algorithm> // std::sort()

#include "macros.h"
#include "miniwebbrowser.h"
#include "htmleditor.h"
#include "highlighter.h"
#include "imagewidget.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "softwarelist.h"
#include "softwaresnapshot.h"
#include "machinelist.h"
#include "romalyzer.h"
#include "qmc2main.h"
#include "imagewidget.h"
#include "softwareimagewidget.h"
#include "customartwork.h"
#include "customsoftwareartwork.h"
#include "itemselect.h"
#include "ui_htmleditor.h"
#include "ui_inserthtmldialog.h"
#include "ui_tablepropertydialog.h"

#define userDataDb qmc2MachineList->userDataDb()
#define FOLLOW_ENABLE(a1, a2) a1->setEnabled(ui->webView->pageAction(a2)->isEnabled())
#define FOLLOW_CHECK(a1, a2) a1->setChecked(ui->webView->pageAction(a2)->isChecked())
#define FORWARD_ACTION(action1, action2) \
    connect(action1, SIGNAL(triggered()), \
            ui->webView->pageAction(action2), SLOT(trigger())); \
    connect(ui->webView->pageAction(action2), \
            SIGNAL(changed()), SLOT(adjustActions()));

// external global variables
extern Settings *qmc2Config;
extern bool qmc2CleaningUp;
extern QTreeWidgetItem *qmc2CurrentItem;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern SoftwareSnapshot *qmc2SoftwareSnapshot;
extern SoftwareList *qmc2SoftwareList;
extern bool qmc2UseSoftwareSnapFile;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern MachineList *qmc2MachineList;
extern MainWindow *qmc2MainWindow;
extern bool qmc2ParentImageFallback;
extern QHash<QString, QString> qmc2ParentHash;
extern QHash<QString, QString> softwareParentHash;

HtmlEditor::HtmlEditor(QString editorName, bool embedded, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui_HTMLEditorMainWindow),
	htmlDirty(false),
	wysiwygDirty(false),
	highlighter(0),
	ui_dialog(0),
	insertHtmlDialog(0),
	ui_tablePropertyDialog(0),
	tablePropertyDialog(0)
{
	ui->setupUi(this);

	// replace the standard QWebView with the MiniWebBrowser's tweaked one
	ui->verticalLayoutWYSIWYG->removeWidget(ui->webView);
	delete ui->webView;
	ui->webView = new BrowserWidget(ui->tabWYSIWYG, 0);
	ui->verticalLayoutWYSIWYG->addWidget(ui->webView);

	myEditorName = editorName;
	isEmbeddedEditor = embedded;

	// this 'trick' allows a nested QMainWindow :)
	setWindowFlags(Qt::Widget);

#if defined(QMC2_OS_MAC)
	ui->menubar->setNativeMenuBar(false);
#endif

	loadActive = false;
	loadSuccess = true;
	xmlQueryBuffer = 0;
	xmlDocument = 0;

	// hide new-from-template and file-revert actions initially
	ui->actionFileNewFromTemplate->setVisible(false);
	ui->actionFileRevert->setVisible(false);

	// enable menu tear-off
	foreach (QMenu *menu, ui->menubar->findChildren<QMenu *>())
		menu->setTearOffEnabled(true);

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/WidgetState").arg(myEditorName)) )
		restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/WidgetState").arg(myEditorName), QByteArray()).toByteArray());

	ui->tabWidget->setTabText(0, tr("WYSIWYG"));
	ui->tabWidget->setTabText(1, tr("HTML"));

	if ( isEmbeddedEditor ) {
		ui->actionFileOpen->setText(tr("&Load..."));
		ui->actionFileOpen->setIconText(tr("Load notes from file"));
		ui->actionFileOpen->setToolTip(tr("Load notes from an HTML file"));
		ui->actionFileOpen->setStatusTip(tr("Load notes from an HTML file"));
		ui->actionFileSave->setIconText(tr("Save current notes"));
		ui->actionFileSave->setToolTip(tr("Save current notes"));
		ui->actionFileSave->setStatusTip(tr("Save current notes"));
	}

	// the 'corner-widget'
	frameCornerWidget = new QFrame(ui->tabWidget);
	frameCornerWidget->setFrameShape(QFrame::NoFrame);
	toolButtonSettings = new QToolButton(frameCornerWidget);
	toolButtonSettings->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
	toolButtonSettings->setPopupMode(QToolButton::MenuButtonPopup);
	toolButtonSettings->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolButtonSettings->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	menuSettings = new QMenu(toolButtonSettings);
	actionHideMenu = menuSettings->addAction(tr("Hide menu"));
	actionHideMenu->setCheckable(true);
	connect(actionHideMenu, SIGNAL(toggled(bool)), ui->menubar, SLOT(setHidden(bool)));
	actionHideMenu->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/MenuHidden").arg(myEditorName), false).toBool());
	actionReadOnly = menuSettings->addAction(tr("Read only"));
	actionReadOnly->setCheckable(true);
	connect(actionReadOnly, SIGNAL(toggled(bool)), this, SLOT(setContentEditable(bool)));
	actionReadOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/ReadOnly").arg(myEditorName), false).toBool());
	actionShowHTML = menuSettings->addAction(tr("Show HTML"));
	actionShowHTML->setCheckable(true);
	connect(actionShowHTML, SIGNAL(toggled(bool)), this, SLOT(showHtmlTab(bool)));
	actionShowHTML->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/ShowHtml").arg(myEditorName), true).toBool());
	toolButtonSettings->setMenu(menuSettings);
	connect(toolButtonSettings, SIGNAL(clicked()), toolButtonSettings, SLOT(showMenu()));
	loadProgress = new QProgressBar(frameCornerWidget);
	loadProgress->setRange(0, 100);
	loadProgress->setValue(0);
	loadProgress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	loadProgress->setFormat("");
	loadProgress->setToolTip(tr("Page load progress"));
	loadProgress->setStatusTip(tr("Page load progress"));
	QHBoxLayout *layout = new QHBoxLayout(frameCornerWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(loadProgress, 1, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(toolButtonSettings, 0, Qt::AlignRight | Qt::AlignVCenter);
	loadProgress->setVisible(false);
	frameCornerWidget->setLayout(layout);
	frameCornerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->tabWidget->setCornerWidget(frameCornerWidget, Qt::BottomRightCorner);

	connect(ui->tabWidget, SIGNAL(currentChanged(int)), SLOT(changeTab(int)));

	highlighter = new Highlighter(ui->plainTextEdit->document());

	QWidget *spacer = new QWidget(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	ui->standardToolBar->insertWidget(ui->actionZoomOut, spacer);

	zoomLabel = new QLabel(this);
	ui->standardToolBar->insertWidget(ui->actionZoomOut, zoomLabel);

	zoomSlider = new QSlider(this);
	zoomSlider->setOrientation(Qt::Horizontal);
	zoomSlider->setMaximumWidth(150);
	zoomSlider->setRange(25, 400);
	zoomSlider->setSingleStep(25);
	zoomSlider->setPageStep(100);
	connect(zoomSlider, SIGNAL(valueChanged(int)), SLOT(changeZoom(int)));
	ui->standardToolBar->insertWidget(ui->actionZoomIn, zoomSlider);

#if defined(QMC2_BROWSER_PLUGINS_ENABLED)
	ui->webView->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
#else
	ui->webView->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, false);
#endif
	ui->webView->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, false);
	ui->webView->page()->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);

	connect(ui->webView, SIGNAL(loadStarted()), this, SLOT(setLoadActive()));
	connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(setLoadInactive()));
	connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(setLoadSuccess(bool)));
	connect(ui->webView, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
	connect(ui->webView, SIGNAL(loadProgress(int)), this, SLOT(loadProgressed(int)));
	connect(ui->webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));

	// popup the 'insert image' menu when the tool-bar button is pressed
	QToolButton *tb = (QToolButton *)ui->formatToolBar->widgetForAction(ui->menuInsertImage->menuAction());
	if ( tb )
		tb->setPopupMode(QToolButton::InstantPopup);

	// menu actions
	connect(ui->actionFileNew, SIGNAL(triggered()), SLOT(fileNew()));
	connect(ui->actionFileNewFromTemplate, SIGNAL(triggered()), SLOT(fileNewFromTemplate()));
	connect(ui->actionFileRevert, SIGNAL(triggered()), SLOT(fileRevert()));
	connect(ui->actionFileOpen, SIGNAL(triggered()), SLOT(fileOpen()));
	connect(ui->actionFileSave, SIGNAL(triggered()), SLOT(fileSave()));
	connect(ui->actionFileSaveAs, SIGNAL(triggered()), SLOT(fileSaveAs()));
	connect(ui->actionFileOpenInBrowser, SIGNAL(triggered()), SLOT(fileOpenInBrowser()));
	connect(ui->actionInsertImageFromFile, SIGNAL(triggered()), SLOT(insertImageFromFile()));
	connect(ui->actionInsertImageFromUrl, SIGNAL(triggered()), SLOT(insertImageFromUrl()));
	connect(ui->actionCreateLink, SIGNAL(triggered()), SLOT(createLink()));
	connect(ui->actionInsertHtml, SIGNAL(triggered()), SLOT(insertHtml()));
	connect(ui->actionInsertTable, SIGNAL(triggered()), SLOT(insertTable()));
	connect(ui->actionZoomOut, SIGNAL(triggered()), SLOT(zoomOut()));
	connect(ui->actionZoomIn, SIGNAL(triggered()), SLOT(zoomIn()));

	// these are forwarded to the internal QWebView
	FORWARD_ACTION(ui->actionEditUndo, QWebPage::Undo);
	FORWARD_ACTION(ui->actionEditRedo, QWebPage::Redo);
	FORWARD_ACTION(ui->actionEditCut, QWebPage::Cut);
	FORWARD_ACTION(ui->actionEditCopy, QWebPage::Copy);
	FORWARD_ACTION(ui->actionEditPaste, QWebPage::Paste);
	FORWARD_ACTION(ui->actionFormatBold, QWebPage::ToggleBold);
	FORWARD_ACTION(ui->actionFormatItalic, QWebPage::ToggleItalic);
	FORWARD_ACTION(ui->actionFormatUnderline, QWebPage::ToggleUnderline);

	// Qt 4.5.0 has a bug: always returns 0 for QWebPage::SelectAll
	connect(ui->actionEditSelectAll, SIGNAL(triggered()), SLOT(editSelectAll()));
	// FIXME: still required?

	connect(ui->actionStyleParagraph, SIGNAL(triggered()), SLOT(styleParagraph()));
	connect(ui->actionStyleHeading1, SIGNAL(triggered()), SLOT(styleHeading1()));
	connect(ui->actionStyleHeading2, SIGNAL(triggered()), SLOT(styleHeading2()));
	connect(ui->actionStyleHeading3, SIGNAL(triggered()), SLOT(styleHeading3()));
	connect(ui->actionStyleHeading4, SIGNAL(triggered()), SLOT(styleHeading4()));
	connect(ui->actionStyleHeading5, SIGNAL(triggered()), SLOT(styleHeading5()));
	connect(ui->actionStyleHeading6, SIGNAL(triggered()), SLOT(styleHeading6()));
	connect(ui->actionStylePreformatted, SIGNAL(triggered()), SLOT(stylePreformatted()));
	connect(ui->actionStyleAddress, SIGNAL(triggered()), SLOT(styleAddress()));
	connect(ui->actionFormatFontName, SIGNAL(triggered()), SLOT(formatFontName()));
	connect(ui->actionFormatFontSize, SIGNAL(triggered()), SLOT(formatFontSize()));
	connect(ui->actionFormatTextColor, SIGNAL(triggered()), SLOT(formatTextColor()));
	connect(ui->actionFormatBackgroundColor, SIGNAL(triggered()), SLOT(formatBackgroundColor()));

	// no page actions exist for these yet, so use execCommand trick
	connect(ui->actionFormatStrikethrough, SIGNAL(triggered()), SLOT(formatStrikeThrough()));
	connect(ui->actionFormatAlignLeft, SIGNAL(triggered()), SLOT(formatAlignLeft()));
	connect(ui->actionFormatAlignCenter, SIGNAL(triggered()), SLOT(formatAlignCenter()));
	connect(ui->actionFormatAlignRight, SIGNAL(triggered()), SLOT(formatAlignRight()));
	connect(ui->actionFormatAlignJustify, SIGNAL(triggered()), SLOT(formatAlignJustify()));
	connect(ui->actionFormatDecreaseIndent, SIGNAL(triggered()), SLOT(formatDecreaseIndent()));
	connect(ui->actionFormatIncreaseIndent, SIGNAL(triggered()), SLOT(formatIncreaseIndent()));
	connect(ui->actionFormatNumberedList, SIGNAL(triggered()), SLOT(formatNumberedList()));
	connect(ui->actionFormatBulletedList, SIGNAL(triggered()), SLOT(formatBulletedList()));

	// it's necessary to sync our actions
	connect(ui->webView->page(), SIGNAL(selectionChanged()), SLOT(adjustActions()));
	connect(ui->webView->page(), SIGNAL(contentsChanged()), SLOT(adjustHTML()));
	connect(ui->webView->page(), SIGNAL(contentsChanged()), SLOT(checkRevertStatus()));
	connect(ui->plainTextEdit, SIGNAL(textChanged()), SLOT(adjustWYSIWYG()));
	connect(ui->plainTextEdit, SIGNAL(textChanged()), SLOT(checkRevertStatus()));

	// web-page connections
	connect(ui->webView->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)), SLOT(linkHovered(const QString &, const QString &, const QString &)));

	// this effectively *disables* internal link-following
	ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
	connect(ui->webView, SIGNAL(linkClicked(QUrl)), SLOT(openLink(QUrl)));
	ui->webView->pageAction(QWebPage::OpenImageInNewWindow)->setVisible(false);
	ui->webView->pageAction(QWebPage::DownloadImageToDisk)->setVisible(false);
	ui->webView->pageAction(QWebPage::OpenFrameInNewWindow)->setVisible(false);
	ui->webView->pageAction(QWebPage::OpenLinkInNewWindow)->setVisible(false);
	ui->webView->pageAction(QWebPage::OpenLink)->setVisible(false);
	ui->webView->pageAction(QWebPage::DownloadLinkToDisk)->setVisible(false);
	ui->webView->pageAction(QWebPage::Back)->setVisible(false);
	ui->webView->pageAction(QWebPage::Forward)->setVisible(false);
	ui->webView->pageAction(QWebPage::Stop)->setVisible(false);
	ui->webView->pageAction(QWebPage::Reload)->setVisible(false);

	ui->webView->setFocus();
	ui->webView->page()->setContentEditable(!actionReadOnly->isChecked());
	ui->plainTextEdit->setReadOnly(actionReadOnly->isChecked());

	if ( !actionShowHTML->isChecked() )
		showHtmlTab(false);

	connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));

	changeZoom(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/Zoom").arg(myEditorName), 100).toInt());

	adjustIconSizes();
	adjustActions();
	adjustHTML();

	imageTypes << "prv" << "fly" << "cab" << "ctl" << "mrq" << "ttl" << "pcb" << "sws";

	localModified = false;
}

HtmlEditor::~HtmlEditor()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/WidgetState").arg(myEditorName), saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/MenuHidden").arg(myEditorName), actionHideMenu->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/ReadOnly").arg(myEditorName), actionReadOnly->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/ShowHtml").arg(myEditorName), actionShowHTML->isChecked());
	closeXmlBuffer();
	if ( highlighter )
		delete highlighter;
	delete ui;
	delete ui_dialog;
}

void HtmlEditor::clearContent()
{
	htmlDirty = wysiwygDirty = false;
	templateMap.clear();
	loadedContent.clear();
	fileName.clear();
	templateName.clear();
	ui->webView->setHtml("");
}

void HtmlEditor::showHtmlTab(bool enable)
{
	if ( enable )
		ui->tabWidget->insertTab(1, ui->tabHTML, tr("HTML"));
	else
		ui->tabWidget->removeTab(1);
}

void HtmlEditor::loadStarted()
{
	ui->tabWidget->cornerWidget()->setUpdatesEnabled(false);
	loadProgress->setVisible(true);
	loadProgress->setValue(0);
	ui->tabWidget->cornerWidget()->setUpdatesEnabled(true);
}

void HtmlEditor::loadProgressed(int value)
{
	ui->tabWidget->cornerWidget()->setUpdatesEnabled(false);
	loadProgress->setVisible(true);
	loadProgress->setValue(value);
	ui->tabWidget->cornerWidget()->setUpdatesEnabled(true);
}

void HtmlEditor::loadFinished(bool)
{
	ui->tabWidget->cornerWidget()->setUpdatesEnabled(false);
	loadProgress->setVisible(false);
	ui->tabWidget->cornerWidget()->setUpdatesEnabled(true);
}

void HtmlEditor::setContentEditable(bool readonly)
{
	ui->webView->page()->setContentEditable(!readonly);
	ui->plainTextEdit->setReadOnly(readonly);
}

void HtmlEditor::checkRevertStatus()
{
	if ( !fileName.isEmpty() ) {
		bool wasModified;
		if ( loadedContent.isEmpty() )
			wasModified = false;
		else if ( ui->tabWidget->currentIndex() == 0 )
			wasModified = loadedContent != ui->webView->page()->mainFrame()->toHtml();
		else
			wasModified = loadedContent != ui->plainTextEdit->toPlainText();
		
		if ( ui->webView->page()->mainFrame()->toHtml() != emptyContent && wasModified ) {
			QFile f(fileName);
			ui->actionFileRevert->setVisible(f.exists());
		} else
			ui->actionFileRevert->setVisible(false);
	} else
		ui->actionFileRevert->setVisible(false);
}

void HtmlEditor::hideTearOffMenus()
{
	foreach (QMenu *menu, ui->menubar->findChildren<QMenu *>())
		if ( menu->isTearOffMenuVisible() )
			menu->hideTearOffMenu();
}

QString &HtmlEditor::noScript(QString &data)
{
	while ( data.contains("<script>") ) {
		int startIndex = data.indexOf("<script>");
		int endIndex = data.indexOf("</script>", startIndex);
		if ( endIndex > startIndex ) {
			endIndex += 9;
			data.remove(startIndex, endIndex - startIndex);
		} else 
			data.remove(startIndex, 8);
	}
	return data;
}

void HtmlEditor::fileNew()
{
	ui->webView->page()->setContentEditable(!actionReadOnly->isChecked());
	ui->plainTextEdit->setReadOnly(actionReadOnly->isChecked());

	if ( !isEmbeddedEditor )
		setCurrentFileName(QString());

	// quirk in QWebView: need an initial mouse click to show the cursor
	int mx = ui->webView->width() / 2;
	int my = ui->webView->height() / 2;
	QPoint center = QPoint(mx, my);
	ui->webView->setFocus();
	QMouseEvent *e1 = new QMouseEvent(QEvent::MouseButtonPress, center, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	QMouseEvent *e2 = new QMouseEvent(QEvent::MouseButtonRelease, center, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
	QApplication::postEvent(ui->webView, e1);
	QApplication::postEvent(ui->webView, e2);

	ui->webView->setHtml("");
	generateEmptyContent = true;
	localModified = false;

	QTimer::singleShot(0, this, SLOT(styleParagraph()));
	QTimer::singleShot(25, this, SLOT(adjustHTML()));
}

void HtmlEditor::fileNewFromTemplate()
{
	if ( loadCurrentTemplate() ) {
		localModified = true;
		if ( !fileName.isEmpty() ) {
			QFile f(fileName);
			ui->actionFileRevert->setVisible(f.exists());
		} else
			ui->actionFileRevert->setVisible(false);
	}
}

void HtmlEditor::fileRevert()
{
	loadCurrent();
	checkRevertStatus();
}

void HtmlEditor::fileOpen()
{
	QString fn(QFileDialog::getOpenFileName(this, isEmbeddedEditor ? tr("Select file to load") : tr("Select file to open"), QString(), tr("HTML files (*.html *.htm)") + ";;" + tr("All files (*)")));
	if ( !fn.isEmpty() ) {
		load(fn);
		adjustHTML();
	}
	localModified = true;
}

void HtmlEditor::fileOpenInBrowser()
{
	MiniWebBrowser *webBrowser = new MiniWebBrowser(0);
	webBrowser->setAttribute(Qt::WA_DeleteOnClose);
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry") )
		webBrowser->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry").toByteArray());
	else {
		webBrowser->adjustSize();
		webBrowser->move(QApplication::desktop()->screen()->rect().center() - webBrowser->rect().center());
	}
	connect(webBrowser->webViewBrowser->page(), SIGNAL(windowCloseRequested()), webBrowser, SLOT(close()));
	if ( ui->tabWidget->currentIndex() == 1 ) {
		ui->webView->page()->mainFrame()->setHtml(ui->plainTextEdit->toPlainText());
		wysiwygDirty = false;
	}
	QString data(ui->webView->page()->mainFrame()->toHtml());
	webBrowser->webViewBrowser->setHtml(noScript(data));
	if ( !fileName.isEmpty() && QFile(fileName).exists() ) {
		webBrowser->homeUrl = QUrl::fromUserInput(fileName);
		webBrowser->comboBoxURL->lineEdit()->setText(webBrowser->homeUrl.toString());
	}
	webBrowser->show();
}

bool HtmlEditor::fileSave()
{
	if ( isEmbeddedEditor && !fileName.isEmpty() && !fileName.startsWith(QLatin1String(":/")) ) {
		bool rc = save();
		checkRevertStatus();
		return rc;
	}

	if ( fileName.isEmpty() || fileName.startsWith(QLatin1String(":/")) ) {
		bool rc = fileSaveAs();
		checkRevertStatus();
		return rc;
	}

	QFile file(fileName);
	bool success = file.open(QIODevice::WriteOnly);
	if ( success ) {
		if ( ui->tabWidget->currentIndex() == 1 ) {
			ui->webView->page()->mainFrame()->setHtml(ui->plainTextEdit->toPlainText());
			wysiwygDirty = false;
		}
		QString content(ui->webView->page()->mainFrame()->toHtml());
		QTextStream ts(&file);
		ts << noScript(content);
		ts.flush();
		file.close();
		success = true;
	}
	localModified = !success;
	checkRevertStatus();
	return success;
}

bool HtmlEditor::fileSaveAs()
{
	QString fn(QFileDialog::getSaveFileName(this, tr("Save a copy"), QString(), tr("HTML files (*.html *.htm)") + ";;" + tr("All files (*)")));

	if ( fn.isEmpty() )
		return false;

	if ( !(fn.endsWith(".htm", Qt::CaseInsensitive) || fn.endsWith(".html", Qt::CaseInsensitive)) )
		fn += ".html"; // default

	if ( fileName.isEmpty() )
		setCurrentFileName(fn);

	QFile file(fn);
	bool success = file.open(QIODevice::WriteOnly);
	if ( success ) {
		if ( ui->tabWidget->currentIndex() == 1 ) {
			ui->webView->page()->mainFrame()->setHtml(ui->plainTextEdit->toPlainText());
			wysiwygDirty = false;
		}
		QString content(ui->webView->page()->mainFrame()->toHtml());
		QTextStream ts(&file);
		ts << noScript(content);
		ts.flush();
		file.close();
		success = true;
	}
	return success;
}

void HtmlEditor::insertImageFromFile()
{
	QString filters;
	filters += tr("Common graphics formats (*.png *.jpg *.jpeg *.gif);;");
	filters += tr("Portable Network Graphics (PNG) (*.png);;");
	filters += tr("Joint Photographic Experts Group (JPEG) (*.jpg *.jpeg);;");
	filters += tr("Graphics Interchange Format (GIF) (*.gif);;");
	filters += tr("All files (*)");

	QString fn(QFileDialog::getOpenFileName(this, tr("Open image..."), QString(), filters));

	if ( fn.isEmpty() )
		return;

	if ( !QFile::exists(fn) )
		return;

	QUrl url = QUrl::fromLocalFile(fn);
	execCommand("insertImage", url.toString());
}

void HtmlEditor::insertImageFromUrl()
{
	QString link(QInputDialog::getText(this, tr("Insert image from URL"), tr("Enter URL:")));
	if ( !link.isEmpty() ) {
		QUrl url = guessUrlFromString(link);
		if ( url.isValid() )
			execCommand("insertImage", url.toString());
	}
}

// shamelessly copied from Qt Demo Browser
QUrl HtmlEditor::guessUrlFromString(const QString &string)
{
	QString urlStr(string.trimmed());
	QRegExp test(QLatin1String("^[a-zA-Z]+\\:.*"));

	// check if it looks like a qualified URL. Try parsing it and see
	bool hasSchema = test.exactMatch(urlStr);
	if ( hasSchema ) {
		QUrl url(urlStr, QUrl::TolerantMode);
		if ( url.isValid() )
			return url;
	}

	// might be a file
	if ( QFile::exists(urlStr) )
		return QUrl::fromLocalFile(urlStr);

	// might be a short-url - try to detect the schema
	if ( !hasSchema ) {
		int dotIndex = urlStr.indexOf(QLatin1Char('.'));
		if ( dotIndex != -1 ) {
			QString prefix(urlStr.left(dotIndex).toLower());
			QString schema(prefix == QLatin1String("ftp") ? prefix : QLatin1String("http"));
			QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
			if ( url.isValid() )
				return url;
		}
	}

	// fall back to QUrl's own tolerant parser
	return QUrl(string, QUrl::TolerantMode);
}

void HtmlEditor::createLink()
{
	QString link(QInputDialog::getText(this, tr("Create link"), tr("Enter URL:")));

	if ( !link.isEmpty() ) {
		QUrl url = guessUrlFromString(link);
		if ( url.isValid() )
			execCommand("createLink", url.toString());
	}
}

void HtmlEditor::insertHtml()
{
	if ( !insertHtmlDialog ) {
		insertHtmlDialog = new QDialog(this);
		if ( !ui_dialog )
			ui_dialog = new Ui_InsertHtmlDialog;
		ui_dialog->setupUi(insertHtmlDialog);
		connect(ui_dialog->buttonBox, SIGNAL(accepted()), insertHtmlDialog, SLOT(accept()));
		connect(ui_dialog->buttonBox, SIGNAL(rejected()), insertHtmlDialog, SLOT(reject()));
	}

	ui_dialog->plainTextEdit->clear();
	ui_dialog->plainTextEdit->setFocus();
	Highlighter *hilite = new Highlighter(ui_dialog->plainTextEdit->document());

	if ( insertHtmlDialog->exec() == QDialog::Accepted )
		execCommand("insertHTML", ui_dialog->plainTextEdit->toPlainText());

	delete hilite;
}

void HtmlEditor::insertTable()
{
	if ( !tablePropertyDialog ) {
		tablePropertyDialog = new QDialog(this);
		if ( !ui_tablePropertyDialog )
			ui_tablePropertyDialog = new Ui_TablePropertyDialog;
		ui_tablePropertyDialog->setupUi(tablePropertyDialog);
	}
    
	ui_tablePropertyDialog->spinBoxRows->setValue(1);
	ui_tablePropertyDialog->spinBoxColumns->setValue(1);
	ui_tablePropertyDialog->lineEditProperties->setText("border=1 width=100%");

	tablePropertyDialog->adjustSize();

	if ( tablePropertyDialog->exec() == QDialog::Accepted ) {
		QString properties(ui_tablePropertyDialog->lineEditProperties->text().simplified());
		int rows = ui_tablePropertyDialog->spinBoxRows->value();
		int cols = ui_tablePropertyDialog->spinBoxColumns->value();
		QString tableTag;
		if ( properties.isEmpty() )
			tableTag = QString("<table>");
		else
			tableTag = QString("<table %1>").arg(properties);
		for (int i = 0; i < rows; i++) {
			tableTag += "<tr>";
			for (int j = 0; j < cols; j++)
				tableTag += "<td><br></td>";
			tableTag += "</tr>";
		}
		tableTag += "</table>";
		execCommand("insertHTML", tableTag);
	}
}

void HtmlEditor::zoomOut()
{
	int percent = static_cast<int>(ui->webView->zoomFactor() * 100);

	if ( percent > 25 ) {
		percent -= 25;
		percent = 25 * (int((percent + 25 - 1) / 25));
		qreal factor = static_cast<qreal>(percent) / 100;
		ui->webView->setZoomFactor(factor);
		ui->actionZoomOut->setEnabled(percent > 25);
		ui->actionZoomIn->setEnabled(true);
		zoomSlider->setValue(percent);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/Zoom").arg(myEditorName), percent);
	}
}

void HtmlEditor::zoomIn()
{
	int percent = static_cast<int>(ui->webView->zoomFactor() * 100);

	if ( percent < 400 ) {
		percent += 25;
		percent = 25 * (int(percent / 25));
		qreal factor = static_cast<qreal>(percent) / 100;
		ui->webView->setZoomFactor(factor);
		ui->actionZoomIn->setEnabled(percent < 400);
		ui->actionZoomOut->setEnabled(true);
		zoomSlider->setValue(percent);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/Zoom").arg(myEditorName), percent);
	}
}

void HtmlEditor::editSelectAll()
{
	ui->webView->triggerPageAction(QWebPage::SelectAll);
}

void HtmlEditor::execCommand(const QString &cmd)
{
	QWebFrame *frame = ui->webView->page()->mainFrame();
	QString js(QString("document.execCommand(\"%1\", false, null)").arg(cmd));
	frame->evaluateJavaScript(js);
	localModified = true;
}

void HtmlEditor::execCommand(const QString &cmd, const QString &arg)
{
	QWebFrame *frame = ui->webView->page()->mainFrame();
	QString js(QString("document.execCommand(\"%1\", false, \"%2\")").arg(cmd).arg(arg));
	frame->evaluateJavaScript(js);
	localModified = true;
}

bool HtmlEditor::queryCommandState(const QString &cmd)
{
	QWebFrame *frame = ui->webView->page()->mainFrame();
	QString js(QString("document.queryCommandState(\"%1\", false, null)").arg(cmd));
	QVariant result = frame->evaluateJavaScript(js);
	return result.toString().simplified().toLower() == "true";
}

void HtmlEditor::styleParagraph()
{
	execCommand("formatBlock", "p");
	if ( generateEmptyContent ) {
		emptyContent = ui->webView->page()->mainFrame()->toHtml();
		generateEmptyContent = false;
	}
}

void HtmlEditor::styleHeading1()
{
	execCommand("formatBlock", "h1");
}

void HtmlEditor::styleHeading2()
{
	execCommand("formatBlock", "h2");
}

void HtmlEditor::styleHeading3()
{
	execCommand("formatBlock", "h3");
}

void HtmlEditor::styleHeading4()
{
	execCommand("formatBlock", "h4");
}

void HtmlEditor::styleHeading5()
{
	execCommand("formatBlock", "h5");
}

void HtmlEditor::styleHeading6()
{
	execCommand("formatBlock", "h6");
}

void HtmlEditor::stylePreformatted()
{
	execCommand("formatBlock", "pre");
}

void HtmlEditor::styleAddress()
{
	execCommand("formatBlock", "address");
}

void HtmlEditor::formatStrikeThrough()
{
	execCommand("strikeThrough");
}

void HtmlEditor::formatAlignLeft()
{
	execCommand("justifyLeft");
}

void HtmlEditor::formatAlignCenter()
{
	execCommand("justifyCenter");
}

void HtmlEditor::formatAlignRight()
{
	execCommand("justifyRight");
}

void HtmlEditor::formatAlignJustify()
{
	execCommand("justifyFull");
}

void HtmlEditor::formatIncreaseIndent()
{
	execCommand("indent");
}

void HtmlEditor::formatDecreaseIndent()
{
	execCommand("outdent");
}

void HtmlEditor::formatNumberedList()
{
	execCommand("insertOrderedList");
}

void HtmlEditor::formatBulletedList()
{
	execCommand("insertUnorderedList");
}

void HtmlEditor::formatFontName()
{
	QStringList families(QFontDatabase().families());
	bool ok = false;
	QString family(QInputDialog::getItem(this, tr("Font"), tr("Select font:"), families, 0, false, &ok));
	if ( ok )
		execCommand("fontName", family);
}

void HtmlEditor::formatFontSize()
{
	QStringList sizes;
	sizes << tr("XS") << tr("S") << tr("M") << tr("L") << tr("XL") << tr("XXL");
	bool ok = false;
	QString size(QInputDialog::getItem(this, tr("Font size"), tr("Font size:"), sizes, sizes.indexOf(tr("M")), false, &ok));
	if ( ok )
		execCommand("fontSize", QString::number(sizes.indexOf(size) + 1));
}

void HtmlEditor::formatTextColor()
{
	QColor color = QColorDialog::getColor(Qt::black, this);
	if ( color.isValid() )
		execCommand("foreColor", color.name());
}

void HtmlEditor::formatBackgroundColor()
{
	QColor color = QColorDialog::getColor(Qt::white, this);
	if ( color.isValid() )
		execCommand("hiliteColor", color.name());
}

void HtmlEditor::adjustActions()
{
	FOLLOW_ENABLE(ui->actionEditUndo, QWebPage::Undo);
	FOLLOW_ENABLE(ui->actionEditRedo, QWebPage::Redo);
	FOLLOW_ENABLE(ui->actionEditCut, QWebPage::Cut);
	FOLLOW_ENABLE(ui->actionEditCopy, QWebPage::Copy);
	FOLLOW_ENABLE(ui->actionEditPaste, QWebPage::Paste);
	FOLLOW_CHECK(ui->actionFormatBold, QWebPage::ToggleBold);
	FOLLOW_CHECK(ui->actionFormatItalic, QWebPage::ToggleItalic);
	FOLLOW_CHECK(ui->actionFormatUnderline, QWebPage::ToggleUnderline);

	ui->actionFormatStrikethrough->setChecked(queryCommandState("strikeThrough"));
	ui->actionFormatNumberedList->setChecked(queryCommandState("insertOrderedList"));
	ui->actionFormatBulletedList->setChecked(queryCommandState("insertUnorderedList"));
}

void HtmlEditor::adjustWYSIWYG()
{
	wysiwygDirty = true;
	if ( ui->tabWidget->currentIndex() == 0 )
		changeTab(0);
}

void HtmlEditor::adjustHTML()
{
	htmlDirty = true;
	if ( ui->tabWidget->currentIndex() == 1 )
		changeTab(1);
}

void HtmlEditor::changeTab(int index)
{
	switch ( index ) {
		case 0:
			if ( wysiwygDirty ) {
				ui->webView->page()->mainFrame()->setHtml(ui->plainTextEdit->toPlainText());
				wysiwygDirty = false;
			}
			break;

		case 1:
			if ( htmlDirty ) {
				ui->plainTextEdit->blockSignals(true);
				ui->plainTextEdit->setPlainText(ui->webView->page()->mainFrame()->toHtml());
				ui->plainTextEdit->blockSignals(false);
				htmlDirty = false;
			}
			break;

		default:
			break;
	}
}

void HtmlEditor::openLink(const QUrl &url)
{
	MiniWebBrowser *webBrowser = new MiniWebBrowser(0);
	webBrowser->setAttribute(Qt::WA_DeleteOnClose);
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry") )
		webBrowser->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry").toByteArray());
	else {
		webBrowser->adjustSize();
		webBrowser->move(QApplication::desktop()->screen()->rect().center() - webBrowser->rect().center());
	}
	connect(webBrowser->webViewBrowser->page(), SIGNAL(windowCloseRequested()), webBrowser, SLOT(close()));
	webBrowser->webViewBrowser->load(url);
	webBrowser->homeUrl = url;
	webBrowser->show();
}

void HtmlEditor::changeZoom(int percent)
{
	ui->actionZoomOut->setEnabled(percent > 25);
	ui->actionZoomIn->setEnabled(percent < 400);
	double factor = (double)percent / 100.0;
	ui->webView->setZoomFactor(factor);
	zoomLabel->setText(tr("Zoom: %1%").arg(percent));
	zoomSlider->setValue(percent);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("HtmlEditor/%1/Zoom").arg(myEditorName), percent);
}

void HtmlEditor::closeEvent(QCloseEvent *e)
{
	e->accept();
}

bool HtmlEditor::loadCurrent()
{
	return load(fileName);
}

bool HtmlEditor::load(const QString &f)
{
	if ( f.isEmpty() )
		return false;

	emptyContent.clear();

	if ( !QFile::exists(f) ) {
		fileNew();
		return false;
	}

	QFile file(f);
	if ( !file.open(QFile::ReadOnly) )
		return false;

	QString data(file.readAll());
	file.close();

	noScript(data);

	if ( f == fileName )
		loadedContent = data;

	ui->webView->setHtml(data);
	ui->webView->page()->setContentEditable(!actionReadOnly->isChecked());
	ui->plainTextEdit->setReadOnly(actionReadOnly->isChecked());

	if ( fileName.isEmpty() )
		setCurrentFileName(f);

	adjustHTML();

	if ( f != fileName )
		checkRevertStatus();

	return true;
}

bool HtmlEditor::loadCurrentTemplate()
{
	return loadTemplate(templateName);
}

bool HtmlEditor::loadTemplate(const QString &f)
{
	if ( f.isEmpty() )
		return false;

	if ( !QFile::exists(f) ) {
		fileNew();
		return false;
	}

	QFile file(f);
	if ( !file.open(QFile::ReadOnly) )
		return false;

	ui->webView->stop();
	loadActive = false;
	emptyContent.clear();
	loadedContent.clear();

	ui->actionFileNewFromTemplate->setVisible(true);

	QString data(file.readAll().simplified());
	file.close();

	while ( data.contains("<!--") ) {
		int startIndex = data.indexOf("<!--");
		int endIndex = data.indexOf("-->", startIndex);
		if ( endIndex > startIndex ) {
			endIndex += 3;
			data.remove(startIndex, endIndex - startIndex);
		} else
			data.remove(startIndex, 4);
	}

	QMapIterator<QString, QString> it(templateMap);
	while ( it.hasNext() ) {
		it.next();
		QString replacementString(it.value());
		data.replace(it.key(), replacementString);
	}

	// pre-execute JavaScript (if any) and wait for asynchronous loads to finish...
	loadActive = true;
	stopLoading = false;
	emptyContent = data;
	ui->webView->setHtml(data, QUrl::fromLocalFile(f));
	while ( loadActive && !qmc2CleaningUp && !stopLoading ) {
		QTest::qWait(1);
		if ( !qmc2CleaningUp && !stopLoading )
			qApp->processEvents(QEventLoop::AllEvents, 1);
	}
	if ( !qmc2CleaningUp && !stopLoading ) {
		ui->webView->setHtml(data, QUrl::fromLocalFile(f));
		ui->webView->page()->setContentEditable(!actionReadOnly->isChecked());
		ui->plainTextEdit->setReadOnly(actionReadOnly->isChecked());
		if ( fileName.isEmpty() )
			setCurrentFileName(f);
		emptyContent = ui->webView->page()->mainFrame()->toHtml();
		adjustHTML();
	} else
		emptyContent = "QMC2_INVALID";

	return true;
}

void HtmlEditor::javaScriptWindowObjectCleared()
{
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("qmc2NotesEditorObject", this);
	ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("qmc2NEO", this);
}

QString HtmlEditor::getIconData()
{
	QByteArray iconData;
	if ( qmc2CurrentItem ) {
		QPixmap pm(qmc2CurrentItem->icon(QMC2_MACHINELIST_COLUMN_ICON).pixmap(64, 64));
		QBuffer buffer(&iconData);
		pm.save(&buffer, "PNG");
	}
	return QString(iconData.toBase64());
}

bool HtmlEditor::isZippedImage(QString imageType)
{
	if ( !qmc2CurrentItem )
		return false;

	ImageWidget *imageWidget = 0;
	switch ( imageTypes.indexOf(imageType) ) {
		case QMC2_IMGTYPE_PREVIEW: imageWidget = qmc2Preview; break;
		case QMC2_IMGTYPE_FLYER: imageWidget = qmc2Flyer; break;
		case QMC2_IMGTYPE_CABINET: imageWidget = qmc2Cabinet; break;
		case QMC2_IMGTYPE_CONTROLLER: imageWidget = qmc2Controller; break;
		case QMC2_IMGTYPE_MARQUEE: imageWidget = qmc2Marquee; break;
		case QMC2_IMGTYPE_TITLE: imageWidget = qmc2Title; break;
		case QMC2_IMGTYPE_PCB: imageWidget = qmc2PCB; break;
		case QMC2_IMGTYPE_SWSNAP: return qmc2UseSoftwareSnapFile;
		default: break;
	}

	if ( imageWidget )
		return imageWidget->useZip() || imageWidget->useSevenZip();
	else
		return false;
}

QString HtmlEditor::getImageData(QString imageType)
{
	if ( !qmc2CurrentItem )
		return QString();

	ImageWidget *imageWidget = 0;
	switch ( imageTypes.indexOf(imageType) ) {
		case QMC2_IMGTYPE_PREVIEW: imageWidget = qmc2Preview; break;
		case QMC2_IMGTYPE_FLYER: imageWidget = qmc2Flyer; break;
		case QMC2_IMGTYPE_CABINET: imageWidget = qmc2Cabinet; break;
		case QMC2_IMGTYPE_CONTROLLER: imageWidget = qmc2Controller; break;
		case QMC2_IMGTYPE_MARQUEE: imageWidget = qmc2Marquee; break;
		case QMC2_IMGTYPE_TITLE: imageWidget = qmc2Title; break;
		case QMC2_IMGTYPE_PCB: imageWidget = qmc2PCB; break;
		case QMC2_IMGTYPE_SWSNAP:
			if ( qmc2SoftwareSnapshot ) {
				QString listName(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST));
				QString entryName(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME));
				QString cacheKey("sws_" + listName + '_' + entryName);
				ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
				if ( !cpm )
					qmc2SoftwareSnapshot->loadImage(listName, entryName);
				else
					qmc2SoftwareSnapshot->currentPixmap = *cpm;
				return qmc2SoftwareSnapshot->toBase64();
			}
			break;
		default: break;
	}

	if ( imageWidget ) {
		QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		QString cacheKey(imageType + '_' + machineName);
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
		if ( !cpm )
			imageWidget->loadImage(machineName, machineName, false, 0, false);
		else
			imageWidget->currentPixmap = *cpm;
		return imageWidget->toBase64();
	} else
		return QString();
}

QString HtmlEditor::getColor(QString currentColor)
{
	QColor color(QColorDialog::getColor(QColor(currentColor), this, tr("Select color"), QColorDialog::DontUseNativeDialog));
	if ( color.isValid() )
		return color.name();
	else
		return currentColor;
}

QString HtmlEditor::getImage(QString currentImage)
{
	QString imageFile(QFileDialog::getOpenFileName(this, tr("Choose image file"), currentImage, tr("All files (*)")));
	if ( !imageFile.isEmpty() )
		return imageFile;
	else
		return currentImage;
}

bool HtmlEditor::queryLocalXml(QString id, QString queryString, bool sort, QString systemEntityName)
{
	QByteArray localXmlDocument(ROMAlyzer::getXmlData(id, true).toLocal8Bit());
	if ( !systemEntityName.isEmpty() )
		localXmlDocument.replace("machine", systemEntityName.toLocal8Bit());
	QBuffer localXmlQueryBuffer(&localXmlDocument);
	localXmlQueryBuffer.open(QIODevice::ReadOnly);
	xmlQuery.bindVariable("xmlDocument", &localXmlQueryBuffer);
	xmlResult.clear();
	if ( !queryString.contains("doc($xmlDocument)") )
		queryString.prepend("doc($xmlDocument)");
	xmlQuery.setQuery(queryString);
	if ( xmlQuery.evaluateTo(&xmlResult) ) {
		if ( sort )
			std::sort(xmlResult.begin(), xmlResult.end(), MainWindow::qStringListLessThan);
		return true;
	} else
		return false;
}

bool HtmlEditor::isBios(QString id)
{
	return qmc2MachineList->isBios(id);
}

bool HtmlEditor::isDevice(QString id)
{
	return qmc2MachineList->isDevice(id);
}

QString HtmlEditor::romStatus(QString id, bool translated)
{
	return qmc2MachineList->romStatus(id, translated);
}

int HtmlEditor::rank(QString id)
{
	return qmc2MachineList->rank(id);
}

QString HtmlEditor::comment(QString id)
{
	return qmc2MachineList->comment(id);
}

QString HtmlEditor::systemInfo(QString id)
{
	QString sysInfo(qmc2MachineList->datInfoDb()->machineInfo(id));
	if ( sysInfo.isEmpty() )
		sysInfo = tr("No data available");
	else
		sysInfo.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(qmc2MainWindow->urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
	return sysInfo;
}

QString HtmlEditor::emuInfo(QString id)
{
	QString emulatorInfo(qmc2MachineList->datInfoDb()->emuInfo(id));
	if ( emulatorInfo.isEmpty() )
		emulatorInfo = tr("No data available");
	else
		emulatorInfo.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(qmc2MainWindow->urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
	return emulatorInfo;
}

QStringList HtmlEditor::videoSnapUrls(QString id)
{
	QStringList vsUrls;
	foreach (QString videoSnapFolder, qmc2Config->value("MAME/FilesAndDirectories/VideoSnapFolder", QMC2_DEFAULT_DATA_PATH + "/vdo/").toString().split(";", QString::SkipEmptyParts)) {
		foreach (QString formatExtension, qmc2MainWindow->videoSnapAllowedFormatExtensions) {
			QFileInfo fi(QDir::cleanPath(videoSnapFolder + "/" + id + formatExtension));
			if ( fi.exists() && fi.isReadable() ) {
				QString videoSnapUrl(fi.absoluteFilePath());
#if defined(QMC2_OS_WIN)
				videoSnapUrl.prepend("file:///");
#else
				videoSnapUrl.prepend("file://");
#endif
				vsUrls << videoSnapUrl;
			}
		}
		if ( vsUrls.isEmpty() ) { // parent fallback
			if ( qmc2ParentImageFallback ) {
				QString parentId(qmc2ParentHash.value(id));
				if ( !parentId.isEmpty() ) {
					foreach (QString formatExtension, qmc2MainWindow->videoSnapAllowedFormatExtensions) {
						QFileInfo fi(QDir::cleanPath(videoSnapFolder + "/" + parentId + formatExtension));
						if ( fi.exists() && fi.isReadable() ) {
							QString videoSnapUrl(fi.absoluteFilePath());
#if defined(QMC2_OS_WIN)
							videoSnapUrl.prepend("file:///");
#else
							videoSnapUrl.prepend("file://");
#endif
							vsUrls << videoSnapUrl;
						}
					}
				}
			}
		}
	}
	return vsUrls;
}

QString HtmlEditor::softwareInfo(QString list, QString id)
{
	QString softInfo(qmc2MachineList->datInfoDb()->softwareInfo(list, id));
	if ( softInfo.isEmpty() )
		softInfo = tr("No data available");
	else
		softInfo.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(qmc2MainWindow->urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
	return softInfo;
}

void HtmlEditor::openLinkInDefaultBrowser(QString linkUrl)
{
	QDesktopServices::openUrl(QUrl::fromUserInput(linkUrl));
}

QStringList HtmlEditor::customSystemArtwork()
{
	QStringList artworkList;
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups())
		if ( qmc2Config->value(QString("%1/Target").arg(name), 0).toInt() == QMC2_AW_INDEX_TARGET_SYSTEM )
			artworkList << name;
	qmc2Config->endGroup();
	return artworkList;
}

QStringList HtmlEditor::customSoftwareArtwork()
{
	QStringList artworkList;
	qmc2Config->beginGroup("Artwork");
	foreach (QString name, qmc2Config->childGroups())
		if ( qmc2Config->value(QString("%1/Target").arg(name), 0).toInt() == QMC2_AW_INDEX_TARGET_SOFTWARE )
			artworkList << name;
	qmc2Config->endGroup();
	return artworkList;
}

bool HtmlEditor::customArtworkZipped(QString artworkName)
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(artworkName), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE;
}

QString HtmlEditor::customSystemArtworkUrl(QString id, QString artworkName)
{
	QHashIterator<int, ImageWidget *> it(ImageWidget::artworkHash);
	ImageWidget *imw = 0;
	while ( it.hasNext() && !imw ) {
		it.next();
		if ( it.value()->customArtwork() ) {
			if ( ((CustomArtwork *)it.value())->name() == artworkName )
				imw = it.value();
		}
	}
	QString filePath;
	QDir dataDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString());
	QString ghostPath(QDir::fromNativeSeparators(dataDir.absolutePath() + "/img/ghost.png"));
	if ( imw ) {
		QString cacheKey(imw->cachePrefix() + '_' + id);
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
		if ( !cpm ) {
			if ( imw->loadImage(id, id, true, &filePath, false) )
				filePath = QDir::fromNativeSeparators(filePath);
			else
				filePath = ghostPath;
		} else
			filePath = cpm->imagePath;
	}
	if ( filePath.isEmpty() )
		filePath = ghostPath;
#if defined(QMC2_OS_WIN)
	return QString("file:///%1").arg(QDir::fromNativeSeparators(filePath));
#else
	return QString("file://%1").arg(QDir::fromNativeSeparators(filePath));
#endif
}

QString HtmlEditor::customSystemArtworkData(QString id, QString artworkName)
{
	QHashIterator<int, ImageWidget *> it(ImageWidget::artworkHash);
	ImageWidget *imw = 0;
	while ( it.hasNext() && !imw ) {
		it.next();
		if ( it.value()->customArtwork() ) {
			if ( ((CustomArtwork *)it.value())->name() == artworkName )
				imw = it.value();
		}
	}
	if ( imw ) {
		QString cacheKey(imw->cachePrefix() + '_' + id);
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
		if ( !cpm ) {
			imw->loadImage(id, id, false, 0, false);
			cpm = qmc2ImagePixmapCache.object(cacheKey);
		}
		if ( cpm ) {
			imw->currentPixmap = *cpm;
			imw->currentPixmap.imagePath = cpm->imagePath;
		}
		return imw->toBase64();
	} else
		return QString();
}

QString HtmlEditor::customSoftwareArtworkUrl(QString listName, QString softwareName, QString artworkName)
{
	QHashIterator<int, SoftwareImageWidget *> it(SoftwareImageWidget::artworkHash);
	SoftwareImageWidget *imw = 0;
	while ( it.hasNext() && !imw ) {
		it.next();
		if ( it.value()->customArtwork() ) {
			if ( ((CustomSoftwareArtwork *)it.value())->name() == artworkName )
				imw = it.value();
		}
	}
	QString filePath;
	QDir dataDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString());
	QString ghostPath(QDir::fromNativeSeparators(dataDir.absolutePath() + "/img/ghost.png"));
	if ( imw ) {
		QString cacheKey(imw->cachePrefix() + '_' + listName + '_' + softwareName);
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
		if ( !cpm ) {
			if ( imw->loadImage(listName, softwareName) )
				filePath = imw->currentPixmap.imagePath;
			else
				filePath = ghostPath;
		} else
			filePath = cpm->imagePath;
	}
	if ( filePath.isEmpty() )
		filePath = ghostPath;
#if defined(QMC2_OS_WIN)
	return QString("file:///%1").arg(QDir::fromNativeSeparators(filePath));
#else
	return QString("file://%1").arg(QDir::fromNativeSeparators(filePath));
#endif
}

QString HtmlEditor::customSoftwareArtworkData(QString listName, QString softwareName, QString artworkName)
{
	QHashIterator<int, SoftwareImageWidget *> it(SoftwareImageWidget::artworkHash);
	SoftwareImageWidget *imw = 0;
	while ( it.hasNext() && !imw ) {
		it.next();
		if ( it.value()->customArtwork() ) {
			if ( ((CustomSoftwareArtwork *)it.value())->name() == artworkName )
				imw = it.value();
		}
	}
	if ( imw ) {
		QString cacheKey(imw->cachePrefix() + '_' + listName + '_' + softwareName);
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
		if ( !cpm ) {
			imw->loadImage(listName, softwareName);
			cpm = qmc2ImagePixmapCache.object(cacheKey);
		}
		if ( cpm ) {
			imw->currentPixmap = *cpm;
			imw->currentPixmap.imagePath = cpm->imagePath;
		}
		return imw->toBase64();
	} else
		return QString();
}

bool HtmlEditor::systemManualExists(const QString &id)
{
	bool exists = !userDataDb->systemManualPaths(id).isEmpty();
	if ( !exists ) {
		QString parentName(qmc2ParentHash.value(id));
		if ( !parentName.isEmpty() )
			exists = !userDataDb->systemManualPaths(parentName).isEmpty();
	}
	return exists;
}

QStringList HtmlEditor::systemManualPaths(const QString &id)
{
	QStringList manualPaths(userDataDb->systemManualPaths(id));
	if ( manualPaths.isEmpty() ) {
		QString parentName(qmc2ParentHash.value(id));
		if ( !parentName.isEmpty() )
			manualPaths = userDataDb->systemManualPaths(parentName);
	}
	return manualPaths;
}

QStringList HtmlEditor::systemManualUrls(const QString &id)
{
	QStringList manualPaths(systemManualPaths(id));
	QStringList manualUrls;
	foreach (QString path, manualPaths)
		manualUrls << QUrl::fromUserInput(path).toString();
	return manualUrls;
}

void HtmlEditor::openSystemManual(const QString &id)
{
	QStringList manualPaths(systemManualPaths(id));
	if ( manualPaths.count() > 1 ) {
		ItemSelector itemSelector(this, manualPaths);
		itemSelector.setWindowTitle(tr("Manual selection"));
		itemSelector.labelMessage->setText(tr("Multiple PDF manuals exist. Select the ones you want to open:"));
		itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
		if ( itemSelector.exec() != QDialog::Rejected ) {
			QList<QListWidgetItem *> itemList(itemSelector.listWidgetItems->selectedItems());
			for (int i = 0; i < itemList.count(); i++) {
				QFileInfo fi(itemList.at(i)->text());
				if ( qmc2MainWindow->actionManualInternalViewer->isChecked() )
					qmc2MainWindow->viewPdf(fi.absoluteFilePath());
				else
					QDesktopServices::openUrl(QUrl::fromUserInput(fi.absoluteFilePath()));
			}
		}
	} else if ( manualPaths.count() > 0 ) {
		QFileInfo fi(manualPaths.first());
		if ( qmc2MainWindow->actionManualInternalViewer->isChecked() )
			qmc2MainWindow->viewPdf(fi.absoluteFilePath());
		else
			QDesktopServices::openUrl(QUrl::fromUserInput(fi.absoluteFilePath()));
	}
}

bool HtmlEditor::softwareManualExists(const QString &list, const QString &id)
{
	bool exists = !userDataDb->softwareManualPaths(list, id).isEmpty();
	if ( !exists ) {
		QString parentKey(softwareParentHash.value(list + ':' + id));
		if ( !parentKey.isEmpty() && parentKey != "<np>" ) {
			QStringList parentWords(parentKey.split(':', QString::SkipEmptyParts));
			exists = !userDataDb->softwareManualPaths(parentWords.at(0), parentWords.at(1)).isEmpty();
		}
	}
	return exists;
}

QStringList HtmlEditor::softwareManualPaths(const QString &list, const QString &id)
{
	QStringList manualPaths(userDataDb->softwareManualPaths(list, id));
	if ( manualPaths.isEmpty() ) {
		QString parentKey(softwareParentHash.value(list + ':' + id));
		if ( !parentKey.isEmpty() && parentKey != "<np>" ) {
			QStringList parentWords(parentKey.split(':', QString::SkipEmptyParts));
			manualPaths = userDataDb->softwareManualPaths(parentWords.at(0), parentWords.at(1));
		}
	}
	return manualPaths;
}

QStringList HtmlEditor::softwareManualUrls(const QString &list, const QString &id)
{
	QStringList manualPaths(softwareManualPaths(list, id));
	QStringList manualUrls;
	foreach (QString path, manualPaths)
		manualUrls << QUrl::fromUserInput(path).toString();
	return manualUrls;
}

void HtmlEditor::openSoftwareManual(const QString &list, const QString &id)
{
	QStringList manualPaths(softwareManualPaths(list, id));
	if ( manualPaths.count() > 1 ) {
		ItemSelector itemSelector(this, manualPaths);
		itemSelector.setWindowTitle(tr("Manual selection"));
		itemSelector.labelMessage->setText(tr("Multiple PDF manuals exist. Select the ones you want to open:"));
		itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
		if ( itemSelector.exec() != QDialog::Rejected ) {
			QList<QListWidgetItem *> itemList(itemSelector.listWidgetItems->selectedItems());
			for (int i = 0; i < itemList.count(); i++) {
				if ( qmc2MainWindow->actionManualInternalViewer->isChecked() )
					qmc2MainWindow->viewPdf(itemList.at(i)->text());
				else
					QDesktopServices::openUrl(QUrl::fromUserInput(itemList.at(i)->text()));
			}
		}
	} else if ( manualPaths.count() > 0 ) {
		if ( qmc2MainWindow->actionManualInternalViewer->isChecked() )
			qmc2MainWindow->viewPdf(manualPaths.at(0));
		else
			QDesktopServices::openUrl(QUrl::fromUserInput(manualPaths.at(0)));
	}
}

void HtmlEditor::closeXmlBuffer()
{
	if ( xmlQueryBuffer ) {
		xmlQueryBuffer->close();
		delete xmlQueryBuffer;
		delete xmlDocument;
		xmlQueryBuffer = 0;
		xmlDocument = 0;
	}
}

void HtmlEditor::enableFileNewFromTemplateAction(bool enable)
{
	ui->actionFileNewFromTemplate->setVisible(enable);
}

bool HtmlEditor::save()
{
	if ( emptyContent == "QMC2_INVALID" )
		return true;

	if ( !ui->webView->page()->isModified() && !ui->plainTextEdit->document()->isModified() && !localModified )
		return true;

	if ( fileName.isEmpty() )
		return false;

	if ( ui->tabWidget->currentIndex() == 1 ) {
		ui->webView->page()->mainFrame()->setHtml(ui->plainTextEdit->toPlainText());
		wysiwygDirty = false;
	}

	loadedContent = ui->webView->page()->mainFrame()->toHtml();

	if ( loadedContent == emptyContent ) {
		QFile f(fileName);
		if ( f.exists() )
			f.remove();
		localModified = false;
		return true;
	}

	QFileInfo fi(fileName);
	QString targetPath(QDir::cleanPath(fi.absoluteDir().path()));

	if ( !QFile::exists(targetPath) ) {
		QDir dummyDir;
		if ( !dummyDir.mkpath(targetPath) )
			return false;
	}

	QFile f(fileName);
	if ( !f.open(QIODevice::WriteOnly) )
		return false;

	QTextStream ts(&f);
	ts << noScript(loadedContent);
	ts.flush();
	f.close();
	localModified = false;

	return true;
}

void HtmlEditor::setCurrentFileName(const QString &fileName)
{
	this->fileName = fileName;
}

void HtmlEditor::setCurrentTemplateName(const QString &templateName)
{
	this->templateName = templateName;
}

void HtmlEditor::linkHovered(const QString &link, const QString & /*title*/, const QString & /*textContent*/)
{
	// FIXME: doesn't work as expected (hides too early)
	QToolTip::showText(QCursor::pos(), link);
}

void HtmlEditor::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height(), fm.height());
	ui->formatToolBar->setIconSize(iconSize);
	ui->standardToolBar->setIconSize(iconSize);
	loadProgress->setFixedHeight(fm.height() - 4);
	toolButtonSettings->setIconSize(iconSize);
	QTabBar *tabBar = ui->tabWidget->findChild<QTabBar *>();
	if ( tabBar ) {
		f.setPointSize(f.pointSize() * 0.666666f);
		tabBar->setFont(f);
	}
}
