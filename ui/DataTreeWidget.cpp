#include "DataTreeWidget.h"
#include <qtreeview.h>
#include <QHeaderView>
#include "RenderStatus.h"

DataTreeWidget::DataTreeWidget(QWidget* pParent) :
	QTreeWidget(pParent) {
	// Set the size policy, making sure the widget fits nicely in the layout
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	// Status and tooltip
	setToolTip("Data");
	//setStatusTip("Statistics");

	// Configure tree
	setColumnCount(3);

	QStringList ColumnNames;

	ColumnNames << "Property" << "Value" << "Unit";
	setHeaderLabels(ColumnNames);
	// Configure headers
	header()->resizeSection(0, 150);
	header()->resizeSection(1, 150);
	header()->resizeSection(2, 100);
	
    connect(&g_RenderStatus, SIGNAL(setDataChanged(const QString&, const QString&, const QString&, const QString&, const QString&)), 
		this, SLOT(dataChanged(const QString&, const QString&, const QString&, const QString&, const QString&)));

	generateTree();
}

void DataTreeWidget::generateTree() {
	addTreeItem(NULL, "Performance", "", "", "DataTree-application");
}

QTreeWidgetItem* DataTreeWidget::addTreeItem(QTreeWidgetItem* pParent, const QString& Property, const QString& Value, const QString& Unit, const QString& Icon)
{
	// Create new item
	QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent);

	// Set item properties
	pItem->setText(0, Property);
	pItem->setText(1, Value);
	pItem->setText(2, Unit);
	QIcon ic = QIcon("Icons/" + Icon + ".png");
	pItem->setIcon(0, ic);
	if (!pParent)
		addTopLevelItem(pItem);
	return pItem;
}

void DataTreeWidget::UpdateData(const QString& Group, const QString& Name, const QString& Value, const QString& Unit, const QString& Icon)
{
	QTreeWidgetItem* pGroup = FindItem(Group);

	if (!pGroup) {
		pGroup = addTreeItem(NULL, Group);

		addTreeItem(pGroup, Name, Value, Unit, Icon);
	}
	else {
		bool Found = false;

		for (int i = 0; i < pGroup->childCount(); i++)
		{
			if (pGroup->child(i)->text(0) == Name)
			{
				pGroup->child(i)->setText(1, Value);
				pGroup->child(i)->setText(2, Unit);

				Found = true;
			}
		}

		if (!Found)
			addTreeItem(pGroup, Name, Value, Unit, Icon);
	}
}

QTreeWidgetItem* DataTreeWidget::FindItem(const QString& Name)
{
	QList<QTreeWidgetItem*> Items = findItems(Name, Qt::MatchRecursive, 0);
	if (Items.size() <= 0)
		return NULL;
	else
		return Items[0];
}

void DataTreeWidget::dataChanged(const QString& Group, const QString& Name, const QString& Value, const QString& Unit, const QString& Icon)
{
	UpdateData(Group, Name, Value, Unit, Icon);
}
