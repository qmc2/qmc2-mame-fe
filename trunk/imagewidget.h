#ifndef _IMAGEWIDGET_H_
#define _IMAGEWIDGET_H_

#include <QMap>
#include <QMenu>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif

#include "unzip.h"

#if QMC2_OPENGL == 1
class ImageWidget : public QGLWidget
#else
class ImageWidget : public QWidget
#endif
{
	Q_OBJECT 

	public:
		unzFile imageFile;
		QPixmap currentPixmap;
		QMenu *contextMenu;
		QString cacheKey;

		ImageWidget(QWidget *parent);
		~ImageWidget();

		QString cleanDir(QString);

	public slots:
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadImage(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
		void copyToClipboard();
		void refresh();

	protected:
		// these virtual functions MUST be reimplemented in the concrete image classes
		virtual QString cachePrefix() { return QString(); }
		virtual QString imageZip() { return QString(); }
		virtual QString imageDir() { return QString(); }
		virtual QString imageType() { return QString(); }
		virtual bool useZip() { return false; }
		virtual bool scaledImage() { return false; }

		// events CAN be reimplemented in the concrete image classes
		virtual void paintEvent(QPaintEvent *);
		virtual void contextMenuEvent(QContextMenuEvent *);
};

#endif
