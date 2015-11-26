#ifndef _SOFTWAREIMAGEWIDGET_H_
#define _SOFTWAREIMAGEWIDGET_H_

#include <QAction>
#include <QMenu>
#include <QHash>
#include <QMap>
#include <QString>
#include <QList>

#include "imagewidget.h"

class SoftwareImageWidget : public QWidget
{
	Q_OBJECT

	public:
		QMap<QString, unzFile> imageFileMap;
		QMap<QString, SevenZipFile*> imageFileMap7z;
		ImagePixmap currentPixmap;
		QMenu *contextMenu;
		QString myCacheKey;
		QAction *actionCopyPathToClipboard;
		QList<int> activeFormats;

		static QHash<int, SoftwareImageWidget *> artworkHash;

		SoftwareImageWidget(QWidget *parent = 0);
		~SoftwareImageWidget();

		QString cleanDir(QString);
		QString &absoluteImagePath() { return currentPixmap.imagePath; }
		QString toBase64();
		void reloadActiveFormats();
		void enableWidgets(bool enable = true);
		void openSource();
		void closeSource();
		void reopenSource() { closeSource(); openSource(); }
		bool parentFallback();
		static void updateArtwork();
		static void reloadArtworkFormats();
		static SoftwareImageWidget *customArtworkWidget(QString);

		// these pure virtual functions MUST be reimplemented in the concrete image classes
		virtual QString cachePrefix() = 0;
		virtual QString imageZip() = 0;
		virtual QString imageDir() = 0;
		virtual QString imageType() = 0;
		virtual int imageTypeNumeric() = 0;
		virtual bool useZip() = 0;
		virtual bool useSevenZip() = 0;
		virtual bool scaledImage() = 0;
		virtual QString fallbackSettingsKey() = 0;

		// these virtual functions CANbe reimplemented in the concrete image classes
		virtual bool customArtwork() { return false; }

	public slots:
		void init();
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadImage(QString, QString, bool fromParent = false);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
		void sevenZipDataReady();

	protected:
		// events CAN be reimplemented in the concrete image classes
		virtual void paintEvent(QPaintEvent *);
		virtual void contextMenuEvent(QContextMenuEvent *);

	private:
		bool m_async;
};

#endif
