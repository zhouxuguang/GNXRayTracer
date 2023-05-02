#ifndef _DEBUGSTEXT_H__
#define _DEBUGSTEXT_H__

#include <QWidget>
#include <qtextedit.h>
#include <qlayout.h>
#include <qmutex.h>
#include <QString>

class DebugText : public QWidget
{
	Q_OBJECT
public:
	~DebugText();
	void addContents(const QString& s1);
	static DebugText* getDebugText();

private:
	QTextEdit *ShowDebugArea;
	QHBoxLayout *qlayout;

	DebugText(QWidget * parent = Q_NULLPTR);
};

#define  TextDinodonS(text)  DebugText::getDebugText()->addContents(text)

#define  TextDinodonN(text, num)  DebugText::getDebugText()->addContents(QString(text) + " " + QString::number(num))
#define  TextDinodonN2(text, num1, num2)  DebugText::getDebugText()->addContents(QString(text) + " " + QString::number(num1) + " " + QString::number(num2))
#define  TextDinodonN3(text, num1, num2, num3)  DebugText::getDebugText()->addContents(QString(text) + " " + QString::number(num1) + " " + QString::number(num2) + " " + QString::number(num3))
#define  TextDinodonN4(text, num1, num2, num3, num4)  DebugText::getDebugText()->addContents(QString(text) + " " + QString::number(num1) + " " + QString::number(num2) + " " + QString::number(num3) + " " + QString::number(num4))


#endif


