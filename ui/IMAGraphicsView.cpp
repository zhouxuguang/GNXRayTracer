#include "IMAGraphicsView.h"
#include "DebugText.h"


IMAGraphicsView::IMAGraphicsView(QGraphicsView * parent)
{
	setFrameShadow(Sunken);
	setFrameShape(NoFrame);
	//设置View的最小显示区
	setMinimumHeight(400);
	setMinimumWidth(300);
	//设置渲染区域抗锯齿
	setRenderHint(QPainter::Antialiasing);

	getMap("Icons/background.png");
	
	
	setScene(&scene);
	//scene.setBackgroundBrush(QColor(0, 255, 255, 255));

	setCacheMode(CacheBackground);
	scale(_scale, _scale);
}

IMAGraphicsView::~IMAGraphicsView()
{
}


void IMAGraphicsView::mouseMoveEvent(QMouseEvent *event)
{

}

void IMAGraphicsView::mousePressEvent(QMouseEvent *event)
{
	//QGraphicsView 坐标
	QPoint viewPoint = event->pos();
	//QGraphicsScene 坐标
	QPointF scenePoint = mapToScene(viewPoint);
	DebugText::getDebugText()->addContents(QString::number(scenePoint.x()) + " " + QString::number(scenePoint.y()));
}

void IMAGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
	//QGraphicsView 坐标
	QPoint viewPoint = event->pos();
	//QGraphicsScene 坐标
	QPointF scenePoint = mapToScene(viewPoint);
	DebugText::getDebugText()->addContents(QString::number(scenePoint.x()) + " " + QString::number(scenePoint.y()));
}

void IMAGraphicsView::drawBackground(QPainter *painter, const QRectF &rect)
{
	painter->drawPixmap(int(sceneRect().left()), int(sceneRect().top()), map);
	
}

void IMAGraphicsView::getMap(QString mapname)
{
	map.load(mapname);

	width = map.width();
	height = map.height();
	bottom = -0.5 * height;
	left = -0.5 * width;

	scene.setSceneRect(left, bottom, width, height);
}

void IMAGraphicsView::wheelEvent(QWheelEvent *event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	if (event->angleDelta().y() > 0) {
		_scale = 1.1f;
	}
	else {
		_scale = 0.9f; 
	}
#else
    if (event->delta() > 0) {
        _scale = 1.1f;
    }
    else {
        _scale = 0.9f;
    }
#endif
	scale(_scale, _scale); 
}

void IMAGraphicsView::PaintBuffer(unsigned char* buffer, int width, int height, int channals)
{
	QImage::Format format;
	if (channals == 4) format = QImage::Format_ARGB32;
	else if (channals == 3) format = QImage::Format_RGB888;
	QImage image(buffer, width, height, static_cast<int>(width * channals * sizeof(unsigned char)), format);
	image.constBits();
	map = QPixmap::fromImage(image.rgbSwapped());

	width = map.width();
	height = map.height();
	bottom = -0.5 * height;
	left = -0.5 * width;
	scene.setSceneRect(left, bottom, width, height);

	setScene(&scene);
	scale(0.5, 0.5);
	scale(2.0, 2.0);
}

