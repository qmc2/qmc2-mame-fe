#ifndef _SOFTWARESNAPSHOT_H_
#define _SOFTWARESNAPSHOT_H_

#include <QAction>
#include <QMenu>
#include <QString>
#include <QList>

#include "imagewidget.h"

class SoftwareImageWidget : public QWidget
{
	Q_OBJECT

	public:
		ImagePixmap currentSnapshotPixmap;
		QMenu *contextMenu;
		QString myCacheKey;
		QAction *actionCopyPathToClipboard;
		QList<int> activeFormats;

		SoftwareImageWidget(QWidget *parent = 0);
		~SoftwareImageWidget();

		QString cleanDir(QString);
		QString absoluteImagePath() { return currentSnapshotPixmap.imagePath; }
		QString toBase64();
		void reloadActiveFormats();
		void enableWidgets(bool enable = true);

		// these pure virtual functions MUST be reimplemented in the concrete image classes
		virtual QString cachePrefix() = 0;
		virtual QString imageZip() = 0;
		virtual QString imageDir() = 0;
		virtual QString imageType() = 0;
		virtual int imageTypeNumeric() = 0;
		virtual bool useZip() = 0;
		virtual bool useSevenZip() = 0;
		virtual bool scaledImage() = 0;

	public slots:
		void init();
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadSnapshot(QString, QString, bool fromParent = false);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
		void sevenZipDataReady();

	protected:
		// events CAN be reimplemented in the concrete image classes
		virtual void paintEvent(QPaintEvent *);
		virtual void contextMenuEvent(QContextMenuEvent *);
		virtual bool customArtwork() { return false; }

	private:
		bool m_async;
};

class SoftwareSnapshot : public SoftwareImageWidget
{
	Q_OBJECT

	public:
		SoftwareSnapshot(QWidget *parent = 0);

		virtual QString cachePrefix() { return "sws"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("Snapshot"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_SWSNAP; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();
};

#endif
