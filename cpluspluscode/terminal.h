#ifndef TERMINAL_H
#define TERMINAL_H

#include<QString>

#include <QDebug>
#include <QtGui>
#include <QWindow>
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTextBrowser>

#include <interpreter.h>

//class Interpreter;

class Terminal{
public:Terminal(QString data);
public: QString title;
public: void out(QString str);
public: QWidget* window;
public: QIcon* icon;
public: QVBoxLayout* layout;
public: QTextEdit* label;
public: QTextBrowser* browser;
//public: Interpreter* interpreter;
public: void clear();
};

#endif // TERMINAL_H
