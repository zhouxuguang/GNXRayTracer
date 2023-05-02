#include "InteractionDockWidget.h"

InteractionDockWidget::InteractionDockWidget(QWidget * parent)
{
	setWindowTitle("Interaction");
	
	setFeatures(QDockWidget::DockWidgetMovable);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    setFeatures(QDockWidget::NoDockWidgetFeatures);
#else
    setFeatures(QDockWidget::AllDockWidgetFeatures);
#endif
	
	dockCentralWidget = new QFrame;

	setWidget(dockCentralWidget);
	centerLayout = new QVBoxLayout;
	dockCentralWidget->setWindowFlags(Qt::FramelessWindowHint);
	dockCentralWidget->setLayout(centerLayout);
	

	setupDock();

	setMinimumWidth(200);
}

InteractionDockWidget::~InteractionDockWidget()
{

}

void InteractionDockWidget::closeEvent(QCloseEvent * event)
{

}

void InteractionDockWidget::setupDock()
{

	m_DataTreeWidget = new DataTreeWidget;
	centerLayout->addWidget(m_DataTreeWidget);

	renderButton = new QPushButton;
	renderButton->setText("Start Rendering");

	centerLayout->addWidget(renderButton);
}










