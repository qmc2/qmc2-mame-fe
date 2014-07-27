#ifndef _ROMSTATUSEXPORT_H_
#define _ROMSTATUSEXPORT_H_

#include <QTextStream>
#include "ui_romstatusexport.h"

#define QMC2_ROMSTATUSEXPORT_FORMAT_ASCII_INDEX		0
#define QMC2_ROMSTATUSEXPORT_FORMAT_CSV_INDEX		1
#define QMC2_ROMSTATUSEXPORT_FORMAT_HTML_INDEX		2
#define QMC2_ROMSTATUSEXPORT_FORMAT_SEP_INDEX		3
#define QMC2_ROMSTATUSEXPORT_FORMAT_ALL_INDEX		4

class ROMStatusExporter : public QDialog, public Ui::ROMStatusExporter
{
	Q_OBJECT

	public:
		bool exportListAutoCorrected;

		ROMStatusExporter(QWidget *parent = 0);
		~ROMStatusExporter();

		void exportToASCII();
		void exportToCSV();
		void exportToHTML();

	public slots:
		void adjustIconSizes();

		// automatically connected slots
		void on_toolButtonBrowseASCIIFile_clicked();
		void on_toolButtonBrowseCSVFile_clicked();
		void on_toolButtonBrowseHTMLFile_clicked();
		void on_pushButtonExport_clicked();
		void on_comboBoxOutputFormat_currentIndexChanged(int);
		void on_checkBoxExportToClipboard_toggled(bool);

	protected:
		void closeEvent(QCloseEvent *);
};

#endif
