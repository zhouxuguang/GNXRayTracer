#include "MainWindow.h"
#include "DebugText.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	setMinimumSize(800, 800);

	setCentralWidget(&centralWidget);

	MainWindowLayout.setAlignment(Qt::AlignCenter);

	centralWidget.setLayout(&MainWindowLayout);

	setMenu();

	setWidget();

	setDock();

	connect(m_InteractionDockWidget.renderButton, SIGNAL(clicked()), this, SLOT(setRendering()));

}

void MainWindow::closeEvent(QCloseEvent *event)
{
	// 关闭渲染线程
	m_DisplayWidget.killRenderThread();

}

void MainWindow::setMenu(void)
{


}

void MainWindow::setWidget(void) {
	
	MainWindowLayout.addWidget(&m_DisplayWidget);

}

void MainWindow::setDock(void) {

	addDockWidget(Qt::LeftDockWidgetArea, &m_InteractionDockWidget);

}

void MainWindow::setRendering()
{

	if (!m_DisplayWidget.renderFlag)
    {
		// 启动渲染
		m_InteractionDockWidget.renderButton->setText("Rendering");

		m_DisplayWidget.startRenderThread();
	}

}







