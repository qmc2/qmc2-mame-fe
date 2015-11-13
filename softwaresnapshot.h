#ifndef _SOFTWARESNAPSHOT_H_
#define _SOFTWARESNAPSHOT_H_

#include <QAction>
#include <QMenu>
#include <QString>
#include <QList>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif

#include "imagewidget.h"

#if QMC2_OPENGL == 1
class SoftwareSnapshot : public QGLWidget
#else
class SoftwareSnapshot : public QWidget
#endif
{
	Q_OBJECT

	public:
		ImagePixmap currentSnapshotPixmap;
		QMenu *contextMenu;
		QString myCacheKey;
		QAction *actionCopyPathToClipboard;
		QList<int> activeFormats;

		SoftwareSnapshot(QWidget *parent = 0);
		~SoftwareSnapshot();

		QString toBase64();
		void reloadActiveFormats();
		void enableWidgets(bool enable = true);

	public slots:
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadSnapshot(QString, QString, bool fromParent = false);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
		void sevenZipDataReady();

	protected:
		void paintEvent(QPaintEvent *);
		void contextMenuEvent(QContextMenuEvent *);

	private:
		bool m_async;
};

#endif
