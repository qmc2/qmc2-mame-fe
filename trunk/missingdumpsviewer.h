#ifndef _MISSINGDUMPSVIEWER_H_
#define _MISSINGDUMPSVIEWER_H_

#include <QTreeWidget>
#include <QString>
#include "ui_missingdumpsviewer.h"

#define QMC2_MDV_COLUMN_ID		0
#define QMC2_MDV_COLUMN_TYPE		1
#define QMC2_MDV_COLUMN_NAME		2
#define QMC2_MDV_COLUMN_SIZE		3
#define QMC2_MDV_COLUMN_CRC		4
#define QMC2_MDV_COLUMN_SHA1		5
#define QMC2_MDV_COLUMN_REASON		6

#define QMC2_MDV_EXPORT_RESPONSE	10

class DumpRecord : public QObject
{
	Q_OBJECT

	public:
		DumpRecord(QString name, QString type, QString size, QString crc, QString sha1) { setName(name); setType(type); setSize(size); setCrc(crc); setSha1(sha1); }

		void setName(QString name) { m_name = name; }
		QString name() { return m_name; }
		void setType(QString type) { m_type = type; }
		QString type() { return m_type; }
		void setSize(QString size) { m_size = size; }
		QString size() { return m_size; }
		void setCrc(QString crc) { m_crc = crc; }
		QString crc() { return m_crc; }
		void setSha1(QString sha1) { m_sha1 = sha1; }
		QString sha1() { return m_sha1; }

		static bool lessThan(const QObject *d1, const QObject *d2) { return ((DumpRecord *)d1)->name() < ((DumpRecord *)d2)->name(); }

	private:
		QString m_name, m_type, m_size, m_crc, m_sha1;
};

class MissingDumpsViewer : public QDialog, public Ui::MissingDumpsViewer
{
	Q_OBJECT

       	public:
		MissingDumpsViewer(QString settingsKey, QWidget *parent = 0);

		bool defaultEmulator() { return m_defaultEmulator; }
		void setDefaultEmulator(bool enable) { m_defaultEmulator = enable; }

	public slots:
		void on_toolButtonExportToDataFile_clicked();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);

	private:
		QString m_settingsKey;
		bool m_defaultEmulator;
};

#endif
