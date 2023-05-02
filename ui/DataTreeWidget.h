#ifndef __DATATREEWIDGET_H__
#define __DATATREEWIDGET_H__


#include <qtreewidget.h>

class DataTreeWidget : public QTreeWidget
{
	Q_OBJECT
public:
	DataTreeWidget(QWidget* pParent = NULL);
	void generateTree(void);
	QTreeWidgetItem* addTreeItem(QTreeWidgetItem* pParent, const QString& Property, const QString& Value = "", const QString& Unit = "", const QString& Icon = "");
	void UpdateData(const QString& Group, const QString& Name, const QString& Value, const QString& Unit, const QString& Icon);
	QTreeWidgetItem* FindItem(const QString& Name);
	private slots:
	void dataChanged(const QString& Group, const QString& Name, const QString& Value, const QString& Unit, const QString& Icon);

};


#endif
