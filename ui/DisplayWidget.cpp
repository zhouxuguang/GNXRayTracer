#include "DisplayWidget.h"

DisplayWidget::DisplayWidget(QGroupBox * parent)
{
	renderFlag = false;
	rThread = nullptr;

	setLayout(&displayWidgetLayout);
	displayWidgetLayout.addWidget(&m_IMAGraphicsView);

	framebuffer.InitBuffer(800, 600, 4);
}

DisplayWidget::~DisplayWidget()
{
	killRenderThread();
	framebuffer.FreeBuffer();
}

void DisplayWidget::closeEvent(QCloseEvent *event)
{
	killRenderThread();
}

void DisplayWidget::startRenderThread()
{
	if (!rThread)
    {
//		rThread = new RenderThread;
//		renderFlag = true;
//		rThread->renderFlag = true;
//		rThread->p_framebuffer = &framebuffer;
//
//		connect(rThread, SIGNAL(PrintString(char*)), this, SLOT(PrintString(char*)));
//		connect(rThread, SIGNAL(PrintDataD(const char*, const double)), this, SLOT(PrintDataD(const char*, const double)));
//		connect(rThread, SIGNAL(PaintBuffer(unsigned char*, int, int, int)), &m_IMAGraphicsView, SLOT(PaintBuffer(unsigned char*, int, int, int)));
//
//		rThread->start();
	}

	renderFlag = true;
}

void DisplayWidget::killRenderThread()
{
#if 0
	if (!rThread)
		return;
	renderFlag = false;
	rThread->renderFlag = false;
	// Kill the render thread
	rThread->quit();
	// Wait for thread to end
	rThread->wait();
	// Remove the render thread
	delete rThread;

	rThread = nullptr;
#endif
}

void DisplayWidget::PrintString(char* s)
{
	TextDinodonS(s);
}

void DisplayWidget::PrintDataD(const char* s, const double data)
{
	TextDinodonN(s, data);
}










