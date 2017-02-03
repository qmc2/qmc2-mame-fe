#ifndef _DBCOLUMNINFO_H_
#define _DBCOLUMNINFO_H_

#include <QString>

class DbColumnInfo
{
	public:
		explicit DbColumnInfo(quint64 cid, const QString &name, const QString &type, const QString &dflt_value, bool notnull, bool pk)
			: m_cid(cid), m_name(name), m_type(type), m_dflt_value(dflt_value), m_notnull(notnull), m_pk(pk) {}

		void setCid(quint64 cid) { m_cid = cid; }
		quint64 cid() { return m_cid; }
		void setName(const QString &name) { m_name = name; }
		QString &name() { return m_name; }
		void setType(const QString &type) { m_type = type; }
		QString &type() { return m_type; }
		void setDefaultValue(const QString &dflt_value) { m_dflt_value = dflt_value; }
		QString &defaultValue() { return m_dflt_value; }
		void setNotNull(bool notnull) { m_notnull = notnull; }
		bool notNull() { return m_notnull; }
		void setPrimaryKey(bool pk) { m_pk = pk; }
		bool primaryKey() { return m_pk; }

	private:
		quint64 m_cid;
		QString m_name;
		QString m_type;
		QString m_dflt_value;
		bool m_notnull;
		bool m_pk;
};

#endif
