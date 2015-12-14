#include <QTableView>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QNetworkAccessManager>
#include <QSortFilterProxyModel>

#include "settings.h"
#include "qmc2main.h"
#include "macros.h"
#include "cookiemanager.h"
#include "cookiejar.h"
#include "networkaccessmanager.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern NetworkAccessManager *qmc2NetworkAccessManager;

CookieManager::CookieManager(QWidget *parent) : QDialog(parent)
{
	setupUi(this);

	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CookieManager/Geometry", QByteArray()).toByteArray());
	tableViewCookies->horizontalHeader()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CookieManager/CookieTableHeaderState", QByteArray()).toByteArray());
#if QT_VERSION < 0x050000
	tableViewCookies->horizontalHeader()->setMovable(true);
#else
	tableViewCookies->horizontalHeader()->setSectionsMovable(true);
#endif

	CookieJar *cj = (CookieJar *)qmc2NetworkAccessManager->cookieJar();
	if ( cj->db.isOpen() ) {
		sqlTableModel = new QSqlQueryModel(this);
		sqlTableModel->setQuery("SELECT name, domain, path, value, expiry, secure, http_only FROM qmc2_cookies", cj->db);
		sqlTableModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
		sqlTableModel->setHeaderData(1, Qt::Horizontal, tr("Domain"));
		sqlTableModel->setHeaderData(2, Qt::Horizontal, tr("Path"));
		sqlTableModel->setHeaderData(3, Qt::Horizontal, tr("Value"));
		sqlTableModel->setHeaderData(4, Qt::Horizontal, tr("Expiry date"));
		sqlTableModel->setHeaderData(5, Qt::Horizontal, tr("Secure?"));
		sqlTableModel->setHeaderData(6, Qt::Horizontal, tr("HTTP only?"));
		QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
		proxyModel->setSourceModel(sqlTableModel);
		tableViewCookies->setModel(proxyModel);
	}
	connect(tableViewCookies->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(tableViewCookies_selectionChanged(const QItemSelection &, const QItemSelection &)));
	tableViewCookies->sortByColumn(tableViewCookies->horizontalHeader()->sortIndicatorSection(), tableViewCookies->horizontalHeader()->sortIndicatorOrder());
}

CookieManager::~CookieManager()
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CookieManager/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CookieManager/CookieTableHeaderState", tableViewCookies->horizontalHeader()->saveState());
}

void CookieManager::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	pushButtonRemove->setIconSize(iconSize);
}

void CookieManager::tableViewCookies_selectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
	pushButtonRemove->setEnabled(tableViewCookies->selectionModel()->selectedRows().count() > 0);
}

void CookieManager::on_pushButtonRemove_clicked()
{
	CookieJar *cj = (CookieJar *)qmc2NetworkAccessManager->cookieJar();
	if ( cj->db.isOpen() ) {
		QSqlQuery query(cj->db);
		cj->db.driver()->beginTransaction();
		foreach (QModelIndex index, tableViewCookies->selectionModel()->selectedRows()) {
			QByteArray name = index.data().toByteArray();
			QString domain = index.sibling(index.row(), 1).data().toString();
			QString path = index.sibling(index.row(), 2).data().toString();
			query.prepare("DELETE FROM qmc2_cookies WHERE domain=:domain AND path=:path AND name=:name");
			query.bindValue(":domain", domain);
			query.bindValue(":path", path);
			query.bindValue(":name", name);
			if ( !query.exec() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to remove cookie from database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(query.lastError().text()));
			query.finish();
		}
		cj->db.driver()->commitTransaction();
	}
	sqlTableModel->setQuery("SELECT name, domain, path, value, expiry, secure, http_only FROM qmc2_cookies", cj->db);
}
