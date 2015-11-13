#ifndef _COMPONENTSETUP_H_
#define _COMPONENTSETUP_H_

#include <QWidget>
#include <QTabWidget>
#include <QHash>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QSplitter>

#include "ui_componentsetup.h"

class ComponentInfo
{
	public:
		ComponentInfo() { ; }

		void setShortTitle(int index, QString title) { m_shortTitleHash[index] = title; }
		QString shortTitle(int index) { return m_shortTitleHash[index]; }
		QHash<int, QString> &shortTitleHash() { return m_shortTitleHash; }

		void setLongTitle(int index, QString title) { m_longTitleHash[index] = title; }
		QString longTitle(int index) { return m_longTitleHash[index]; }
		QHash<int, QString> &longTitleHash() { return m_longTitleHash; }

		void setIcon(int index, QIcon icon) { m_iconHash[index] = icon; }
		QIcon icon(int index) { return m_iconHash[index]; }
		QHash<int, QIcon> &iconHash() { return m_iconHash; }

		void setWidget(int index, QWidget *widget) { m_widgetHash[index] = widget; }
		QWidget *widget(int index) { return m_widgetHash[index]; }
		QHash<int, QWidget *> &widgetHash() { return m_widgetHash; }

		void setRemovable(int index, bool removable) { m_removableHash[index] = removable; }
		bool removable(int index) { if ( m_removableHash.contains(index) ) return m_removableHash[index]; else return true; }

		QList<int> &availableFeatureList() { return m_availableFeatureList; }
		QList<int> &configurableFeatureList() { return m_configurableFeatureList; }
		QList<int> &activeFeatureList() { return m_activeFeatureList; }
		QList<int> &appliedFeatureList() { return m_appliedFeatureList; }

	private:
		QHash<int, QString> m_shortTitleHash;
		QHash<int, QString> m_longTitleHash;
		QHash<int, QIcon> m_iconHash;
		QHash<int, QWidget *> m_widgetHash;
		QHash<int, bool> m_removableHash;
		QList<int> m_availableFeatureList;
		QList<int> m_configurableFeatureList;
		QList<int> m_activeFeatureList;
		QList<int> m_appliedFeatureList;
};

class ComponentSetup : public QDialog, public Ui::ComponentSetup
{
	Q_OBJECT

	public:
		ComponentSetup(QWidget *parent = 0);
		~ComponentSetup();

		QHash<QString, ComponentInfo *> &componentInfoHash() { return m_componentInfoHash; }
		QStringList &components() { return m_components; }

	public slots:
		void loadComponent(QString name = QString(), bool fromSettings = true);
		void saveComponent(QString name = QString());
		void loadArrangement();
		void saveArrangement();
		void setArrangement(int index);
		void adjustIconSizes();

		// automatically connected slots
		void on_listWidgetAvailableFeatures_itemSelectionChanged(); 
		void on_listWidgetActiveFeatures_itemSelectionChanged(); 
		void on_pushButtonActivateFeatures_clicked();
		void on_pushButtonConfigureFeature_clicked();
		void on_pushButtonDeactivateFeatures_clicked();
		void on_pushButtonFeatureUp_clicked();
		void on_pushButtonFeatureDown_clicked();
		void on_pushButtonOk_clicked() { saveArrangement(); saveComponent(); }
		void on_pushButtonApply_clicked() { saveArrangement(); saveComponent(); }
		void on_pushButtonCancel_clicked() { loadArrangement(); loadComponent(); }
		void on_comboBoxComponents_currentIndexChanged(int index);
		void on_comboBoxArrangements_currentIndexChanged(int index) { setArrangement(index); }

	protected:
		void showEvent(QShowEvent *) { adjustIconSizes(); loadArrangement(); loadComponent(); }

	private:
		QHash<QString, ComponentInfo *> m_componentInfoHash;
		QHash<QString, QTabWidget *> m_componentToWidgetHash;
		QHash<QString, QSplitter *> m_componentToSplitterHash;
		QHash<QString, int> m_componentToSplitterIndexHash;
		QStringList m_components;
		QSplitter *splitter0, *splitter1;
		QWidget *widget00, *widget01, *widget10, *widget11;
		int m_artworkIndex;

		ComponentInfo *initComponent1();
		ComponentInfo *initComponent2();
		ComponentInfo *initComponent3();
		ComponentInfo *initComponent4();
};

#endif
