#ifndef _IMGCHECK_H_
#define _IMGCHECK_H_

#include <QDialog>
#include <QTreeWidget>
#include <QStringList>
#include "ui_imgcheck.h"

#include "flyer.h"
#include "preview.h"

class ImageChecker : public QDialog, public Ui::ImageChecker
{
  Q_OBJECT

  public:
    bool ignoreResizeAndMove;

    ImageChecker(QWidget *parent = 0);
    ~ImageChecker();

  public slots:
    // callback handlers
    void on_pushButtonPreviewsCheck_clicked();
    void on_pushButtonPreviewsRemoveObsolete_clicked();
    void on_listWidgetPreviewsFound_itemSelectionChanged();
    void on_listWidgetPreviewsFound_clicked(const QModelIndex &) { on_listWidgetPreviewsFound_itemSelectionChanged(); }
    void on_listWidgetPreviewsMissing_itemSelectionChanged();
    void on_listWidgetPreviewsMissing_clicked(const QModelIndex &) { on_listWidgetPreviewsMissing_itemSelectionChanged(); }
    void on_pushButtonFlyersCheck_clicked();
    void on_pushButtonFlyersRemoveObsolete_clicked();
    void on_listWidgetFlyersFound_itemSelectionChanged();
    void on_listWidgetFlyersFound_clicked(const QModelIndex &) { on_listWidgetFlyersFound_itemSelectionChanged(); }
    void on_listWidgetFlyersMissing_itemSelectionChanged();
    void on_listWidgetFlyersMissing_clicked(const QModelIndex &) { on_listWidgetFlyersMissing_itemSelectionChanged(); }
    void on_pushButtonIconsCheck_clicked();
    void on_pushButtonIconsRemoveObsolete_clicked();
    void on_listWidgetIconsFound_itemSelectionChanged();
    void on_listWidgetIconsFound_clicked(const QModelIndex &) { on_listWidgetIconsFound_itemSelectionChanged(); }
    void on_listWidgetIconsMissing_itemSelectionChanged();
    void on_listWidgetIconsMissing_clicked(const QModelIndex &) { on_listWidgetIconsMissing_itemSelectionChanged(); }

    // other
    bool lockProcessing();
    void selectItem(QString);
    void restoreLayout();
    void recursiveFileList(const QString &, QStringList &);

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
