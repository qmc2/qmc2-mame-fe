#ifndef CATVERINIOPTIMIZER_H
#define CATVERINIOPTIMIZER_H

#include <QMap>
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
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);

	private:
		void clearCategoryNames();
		void clearVersionNames();
		bool loadCatverIni();
		void optimize();

		QString m_fileName;
		QString m_categoryStr;
		QString m_verAddedStr;
		QHash<QString, QString *> m_categoryNames;
		QMap<QString, QString *> m_categoryMap;
		QHash<QString, QString *> m_versionNames;
		QMap<QString, QString *> m_versionMap;
};

#endif
