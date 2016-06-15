#ifndef _CATVERINIOPTIMIZER_H_
#define _CATVERINIOPTIMIZER_H_

#include <QHash>
#include <QString>

#include "ui_catverinioptimizer.h"

class CatverIniOptimizer : public QDialog, public Ui::CatverIniOptimizer
{
	Q_OBJECT

       	public:
		explicit CatverIniOptimizer(QString fileName, QWidget *parent = 0);
		~CatverIniOptimizer();

	public slots:
		void on_pushButtonOptimize_clicked();
		void log(const QString &);

	protected:
		void closeEvent(QCloseEvent *);

	private:
		void clearCategoryNames();
		void clearVersionNames();
		bool loadCatverIni();
		void optimize();

		QString m_fileName;
		QHash<QString, QString *> m_categoryNames;
		QHash<QString, QString *> m_categoryHash;
		QHash<QString, QString *> m_versionNames;
		QHash<QString, QString *> m_versionHash;
};

#endif
