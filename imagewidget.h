#ifndef _IMAGEWIDGET_H_
#define _IMAGEWIDGET_H_

#include <QMap>
#include <QHash>
#include <QMenu>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QAction>
#include <QStringList>

#include "unzip.h"
#include "sevenzipfile.h"
#include "macros.h"

class ImagePixmap : public QPixmap
{
	public:
		QString imagePath;
		bool isGhost;

		ImagePixmap(bool ghost = false) : QPixmap()
		{
			imagePath.clear();
			isGhost = ghost;
		}

		ImagePixmap(const ImagePixmap &other) : QPixmap((QPixmap &)other)
		{
			imagePath = other.imagePath;
			isGhost = other.isGhost;
		}

		ImagePixmap(const QPixmap &other) : QPixmap(other)
		{
			const ImagePixmap *otherImagePixmap = dynamic_cast<const ImagePixmap *>(&other);
			if ( otherImagePixmap ) {
				imagePath = otherImagePixmap->imagePath;
				isGhost = otherImagePixmap->isGhost;
			} else {
				imagePath.clear();
				isGhost = false;
			}
		}

		ImagePixmap &operator=(const ImagePixmap &other)
		{
			QPixmap::operator=((QPixmap &)other);
			imagePath = other.imagePath;
			isGhost = other.isGhost;
			return *this;
		}

		ImagePixmap &operator=(const QPixmap &other)
		{
			QPixmap::operator=(other);
			const ImagePixmap *otherImagePixmap = dynamic_cast<const ImagePixmap *>(&other);
			if ( otherImagePixmap ) {
				imagePath = otherImagePixmap->imagePath;
				isGhost = otherImagePixmap->isGhost;
			} else {
				imagePath.clear();
				isGhost = false;
			}
			return *this;
		}
};

class ImageWidget : public QWidget
{
	Q_OBJECT 

	public:
		QMap<QString, unzFile> imageFileMap;
		QMap<QString, SevenZipFile*> imageFileMap7z;
		ImagePixmap currentPixmap;
		QMenu *contextMenu;
		QString cacheKey;
		QAction *actionCopyPathToClipboard;
		QList<int> activeFormats;

		static QStringList formatNames;
		static QStringList formatExtensions;
		static QStringList formatDescriptions;
		static QHash<int, ImageWidget *> artworkHash;

		ImageWidget(QWidget *parent);
		~ImageWidget();

		QString cleanDir(QString);
		QString &absoluteImagePath() { return currentPixmap.imagePath; }
		QString toBase64();
		QString primaryPathFor(QString);
		void reloadActiveFormats();
		void enableWidgets(bool enable = true);
		void openSource();
		void closeSource();
		void reopenSource() { closeSource(); openSource(); }
		bool parentFallback();
		static void updateArtwork();
		static void reloadArtworkFormats();
		static ImageWidget *customArtworkWidget(QString);

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

		// these virtual functions CAN be reimplemented in the concrete image classes
		virtual bool customArtwork() { return false; }

	protected:
		// events CAN be reimplemented in the concrete image classes
		virtual void paintEvent(QPaintEvent *);
		virtual void contextMenuEvent(QContextMenuEvent *);

	public slots:
		void init();
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadImage(QString, QString, bool checkOnly = false, QString *fileName = NULL, bool loadImages = true);
		bool replaceImage(QString, QPixmap &);
		bool checkImage(QString, unzFile zip = NULL, SevenZipFile *sevenZip = NULL, QSize *sizeReturn = NULL, int *bytesUsed = NULL, QString *fileName = NULL, QString *readerError = NULL, bool *async = NULL, bool *isFillingDict = NULL);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
		void sevenZipDataReady();

	private:
		bool m_async;
};

#endif
