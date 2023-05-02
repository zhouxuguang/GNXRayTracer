#pragma once
#ifndef __IMAGraphicsView_h__
#define __IMAGraphicsView_h__

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QPixmap>
#include <QMouseEvent>


class IMAGraphicsView : public QGraphicsView {
	Q_OBJECT

public:
	IMAGraphicsView(QGraphicsView * parent = Q_NULLPTR);
	~IMAGraphicsView();
public:
	void getMap(QString mapname);

private:
	int bottom, left, top, right, width, height;
	float _scale = 1.0f;
private:
	QGraphicsScene scene;
	QPixmap map;

signals:
	

private slots:
	void PaintBuffer(unsigned char* buffer, int width, int height, int channals);

protected:
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void drawBackground(QPainter *painter, const QRectF &rect);
	void mouseReleaseEvent(QMouseEvent *event);
};





#endif






