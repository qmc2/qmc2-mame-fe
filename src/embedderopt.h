#ifndef EMBEDDEROPT_H
#define EMBEDDEROPT_H

#include <qglobal.h>
#include <QMap>
#include <QtGui>
#include "macros.h"

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)

#include "ui_embedderopt.h"

#define QMC2_EMBEDDER_SNAP_IMGTYPE_PREVIEW	0
#define QMC2_EMBEDDER_SNAP_IMGTYPE_TITLE	1
#define QMC2_EMBEDDER_SNAP_IMGTYPE_SWS		2
#define QMC2_EMBEDDER_SNAP_IMGTYPE_FLYER	3
#define QMC2_EMBEDDER_SNAP_IMGTYPE_CABINET	4
#define QMC2_EMBEDDER_SNAP_IMGTYPE_CONTROLLER	5
#define QMC2_EMBEDDER_SNAP_IMGTYPE_MARQUEE	6
#define QMC2_EMBEDDER_SNAP_IMGTYPE_PCB		7

#define QMC2_EMBEDDER_SNAP_SCALE_NONE		0
#define QMC2_EMBEDDER_SNAP_SCALE_NO_FILTER	1
#define QMC2_EMBEDDER_SNAP_SCALE_FILTERED	2

class SnapshotViewer : public QWidget
{
	Q_OBJECT

	public:
		QListWidgetItem *myItem;
		QMenu *contextMenu;
		QMenu *useAsMenu;
		QMenu *swsMenu;
		QString fileName;
		QStringList imageTypeNames;
		QStringList cachePrefixes;
		QStringList imageTypeIcons;
		QMap<QString, QAction *> useAsActions;
		int zoom;

		SnapshotViewer(QListWidgetItem *item, QWidget *parent = 0);

	public slots:
		void useAsImage();
		void copyToClipboard();
		void saveAs();
		void zoomIn();
		void zoomOut();
		void resetZoom();
		void updateUseAsMenu() { contextMenuEvent(0); };

	protected:
		void leaveEvent(QEvent *);
		void mousePressEvent(QMouseEvent *);
		void contextMenuEvent(QContextMenuEvent *);
		void keyPressEvent(QKeyEvent *);
		void paintEvent(QPaintEvent *);

	signals:
		void scaleRequested(QListWidgetItem *);
};

class EmbedderOptions : public QWidget, public Ui::EmbedderOptions
{
	Q_OBJECT

	public:
		QMap<QListWidgetItem *, QPixmap> snapshotMap;
		SnapshotViewer *snapshotViewer;
		bool showSnapshotViewer;

		EmbedderOptions(QWidget *parent = 0);
		~EmbedderOptions();

	public slots:
		void on_toolButtonTakeSnapshot_clicked();
		void on_toolButtonClearSnapshots_clicked();
		void on_toolButtonSaveAs_clicked();
		void on_listWidgetSnapshots_itemPressed(QListWidgetItem *);
		void on_listWidgetSnapshots_itemSelectionChanged();
		void on_comboBoxScaleMode_currentIndexChanged(int);
		void on_spinBoxZoom_valueChanged(int);
		void adjustIconSizes();
};
#endif

#endif
