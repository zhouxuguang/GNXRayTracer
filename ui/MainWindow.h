#ifndef MAIN_WINDOW_HJHDHFH__INCLUDE_H
#define MAIN_WINDOW_HJHDHFH__INCLUDE_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QCloseEvent>

#include "DisplayWidget.h"
#include "InteractionDockWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = Q_NULLPTR);

public:


private:
	QWidget centralWidget;
	QVBoxLayout MainWindowLayout;

	DisplayWidget m_DisplayWidget;

	InteractionDockWidget m_InteractionDockWidget;

private:
	void setMenu(void);
	void setWidget(void);
	void setDock(void);
	void closeEvent(QCloseEvent *event);

private slots:
	void setRendering();

};


#endif


