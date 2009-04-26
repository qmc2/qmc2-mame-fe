#ifndef _ROMSTATUSEXPORT_H_
#define _ROMSTATUSEXPORT_H_

#include <QTextStream>
#include "ui_romstatusexport.h"

#define QMC2_ROMSTATUSEXPORT_FORMAT_ASCII_INDEX		0
#define QMC2_ROMSTATUSEXPORT_FORMAT_CSV_INDEX		1
#define QMC2_ROMSTATUSEXPORT_FORMAT_HTML_INDEX		2

class ROMStatusExporter : public QDialog, public Ui::ROMStatusExporter
{
  Q_OBJECT

  public:
    ROMStatusExporter(QWidget *parent = 0);
    ~ROMStatusExporter();

    void exportToASCII();
    void exportToCSV();
    void exportToHTML();

  public slots:
    void on_toolButtonBrowseASCIIFile_clicked();
    void on_toolButtonBrowseCSVFile_clicked();
    void on_toolButtonBrowseHTMLFile_clicked();
    void on_pushButtonExport_clicked();
    void on_comboBoxOutputFormat_currentIndexChanged(int);

  protected:
    void closeEvent(QCloseEvent *);
};

#endif
