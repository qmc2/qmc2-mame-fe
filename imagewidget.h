#ifndef _IMAGEWIDGET_H_
#define _IMAGEWIDGET_H_

#include <QMap>
#include <QMenu>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QAction>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif
#include "unzip.h"

class ImagePixmap : public QPixmap
{
	public:
		QString imagePath;
		ImagePixmap() : QPixmap()
		{
			imagePath.clear();
		}

		ImagePixmap(const ImagePixmap &other)
		{
			QPixmap::operator=((QPixmap &)other);
			imagePath = other.imagePath;
		}

		ImagePixmap(const QPixmap &other)
		{
			QPixmap::operator=(other);
			const ImagePixmap *otherImagePixmap = dynamic_cast<const ImagePixmap *>(&other);
			if ( otherImagePixmap )
				imagePath = otherImagePixmap->imagePath;
			else
				imagePath.clear();
		}

		ImagePixmap &operator=(const ImagePixmap &other)
		{
			QPixmap::operator=((QPixmap &)other);
			imagePath = other.imagePath;
			return *this;
		}

		ImagePixmap &operator=(const QPixmap &other)
		{
			QPixmap::operator=(other);
			const ImagePixmap *otherImagePixmap = dynamic_cast<const ImagePixmap *>(&other);
			if ( otherImagePixmap )
				imagePath = otherImagePixmap->imagePath;
			else
				imagePath.clear();
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
		unzFile imageFile;
		ImagePixmap currentPixmap;
		QMenu *contextMenu;
		QString cacheKey;
		QAction *actionCopyPathToClipboard;

		ImageWidget(QWidget *parent);
		~ImageWidget();

		QString cleanDir(QString);
		QString absoluteImagePath() { return currentPixmap.imagePath; }

		// these virtual functions MUST be reimplemented in the concrete image classes
		virtual QString cachePrefix() { return QString(); }
		virtual QString imageZip() { return QString(); }
		virtual QString imageDir() { return QString(); }
		virtual QString imageType() { return QString(); }
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
		bool checkImage(QString, unzFile zip = NULL, QSize *sizeReturn = NULL, int *bytesUsed = NULL, QString *fileName = NULL, QString *readerError = NULL);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
};

#endif
