#ifndef INTERACTIONDOCKWIDGET_FGRGGR_INCLUDE_H
#define INTERACTIONDOCKWIDGET_FGRGGR_INCLUDE_H

#include <QDockWidget>
#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>

#include "DataTreeWidget.h"

class InteractionDockWidget : public QDockWidget
{
	Q_OBJECT

public:
	InteractionDockWidget(QWidget * parent = Q_NULLPTR);
	~InteractionDockWidget();


private:
	void setupDock();

private:
	QVBoxLayout *centerLayout;
	QFrame *dockCentralWidget;

public:
	QPushButton *renderButton;
	DataTreeWidget * m_DataTreeWidget;

protected:
	void closeEvent(QCloseEvent *event);
	
private slots :
	
	
};

#endif
