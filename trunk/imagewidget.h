#ifndef _IMAGEWIDGET_H_
#define _IMAGEWIDGET_H_

#include <QMap>
#include <QMenu>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QAction>
#include <QStringList>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif
#include "unzip.h"
#include "macros.h"

#define QMC2_IMAGE_FORMAT_INDEX_PNG	0
#define QMC2_IMAGE_FORMAT_INDEX_BMP	1
#define QMC2_IMAGE_FORMAT_INDEX_GIF	2
#define QMC2_IMAGE_FORMAT_INDEX_JPG	3
#define QMC2_IMAGE_FORMAT_INDEX_PBM	4
#define QMC2_IMAGE_FORMAT_INDEX_PGM	5
#define QMC2_IMAGE_FORMAT_INDEX_PPM	6
#define QMC2_IMAGE_FORMAT_INDEX_TIFF	7
#define QMC2_IMAGE_FORMAT_INDEX_XBM	8
#define QMC2_IMAGE_FORMAT_INDEX_XPM	9
#define QMC2_IMAGE_FORMAT_INDEX_SVG	10
#define QMC2_IMAGE_FORMAT_INDEX_TGA	11

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

#if QMC2_OPENGL == 1
class ImageWidget : public QGLWidget
#else
class ImageWidget : public QWidget
#endif
{
	Q_OBJECT 

	public:
		QMap<QString, unzFile> imageFileMap;
		ImagePixmap currentPixmap;
		QMenu *contextMenu;
		QString cacheKey;
		QAction *actionCopyPathToClipboard;
		QList<int> activeFormats;

		static QStringList formatNames;
		static QStringList formatExtensions;

		ImageWidget(QWidget *parent);
		~ImageWidget();

		QString cleanDir(QString);
		QString absoluteImagePath() { return currentPixmap.imagePath; }
		QString toBase64();
		QString primaryPathFor(QString);

		// these virtual functions MUST be reimplemented in the concrete image classes
		virtual QString cachePrefix() { return QString(); }
		virtual QString imageZip() { return QString(); }
		virtual QString imageDir() { return QString(); }
		virtual QString imageType() { return QString(); }
		virtual int imageTypeNumeric() { return 0; }
		virtual bool useZip() { return false; }
		virtual bool scaledImage() { return false; }

	protected:
		// events CAN be reimplemented in the concrete image classes
		virtual void paintEvent(QPaintEvent *);
		virtual void contextMenuEvent(QContextMenuEvent *);

	public slots:
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadImage(QString, QString, bool checkOnly = false, QString *fileName = NULL, bool loadImages = true);
		bool replaceImage(QString, QPixmap &);
		bool checkImage(QString, unzFile zip = NULL, QSize *sizeReturn = NULL, int *bytesUsed = NULL, QString *fileName = NULL, QString *readerError = NULL);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
};

#endif
