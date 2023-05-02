#ifndef RENDERSTATUS_JKDGJFGJ_INCLUDE_H
#define RENDERSTATUS_JKDGJFGJ_INCLUDE_H

#include <QObject>

class renderStatus : public QObject
{
	Q_OBJECT

public:
	renderStatus(QObject * parent = Q_NULLPTR) { }
	~renderStatus() {};


	//Çå³ýÈ«²¿
	void clearAllStatus() {
		
	}

private:

public:


signals:
	void setDataChanged(const QString& Group, const QString& Name, const QString& Value, const QString& Unit = "", const QString& Icon = "");

private slots:


};


extern renderStatus g_RenderStatus;


#endif











