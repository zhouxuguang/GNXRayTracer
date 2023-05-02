#include "DebugText.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <ostream>
#include <fstream>

static DebugText *dt = NULL;


DebugText::DebugText(QWidget * parent) : QWidget(parent) {
	setMinimumSize(800,800);
	
	qlayout = new QHBoxLayout(this);
	ShowDebugArea = new QTextEdit(this);
	ShowDebugArea->setFontPointSize(18);
	qlayout->setAlignment(Qt::AlignCenter);
	qlayout->addWidget(ShowDebugArea);
	show();
}

DebugText::~DebugText() {
}

static QMutex mutexInDebugText;
void DebugText::addContents(const QString& s1)
{
	//QMutexLocker locker(&mutexInDebugText);
	mutexInDebugText.lock();
	ShowDebugArea->append(s1);
	show();
	mutexInDebugText.unlock();
}

static QMutex mutexInStaticDebugText;
DebugText* DebugText::getDebugText() {
	//QMutexLocker locker(&mutexInStaticDebugText);
	mutexInStaticDebugText.lock();
	if (dt == NULL)
		dt = new DebugText;
	mutexInStaticDebugText.unlock();
	return dt;
}


