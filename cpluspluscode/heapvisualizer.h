#ifndef HEAPVISUALIZER_H
#define HEAPVISUALIZER_H

#include <QDebug>
#include <QtGui>
#include <QWindow>
#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTextBrowser>
#include <QDialog>

#include <interpreter.h>
#include <heapcanvas.h>
#include "pwidget.h"
#include "prect.h"
#include "ptext.h"

//class HeapCanvas;

class HeapVisualizer:public PWidget{
    /*
    public: HeapVisualizer();
    private: PWidget* pwidget;
    public: QString title;
    public: QWidget* window;
    public: QIcon* icon;
    public: QVBoxLayout* layout;
    public: QTextEdit* label;
    public: QTextBrowser* browser;
    public: QVector<QString> entries;
    public: HeapCanvas* heapCanvas;
    public: setHeapSize(QString size);
    private: int size;
    public: void draw(int location, QString prettyByte);
    public: void redraw();
    public: int currentHighlighted;
    public: void setInitThreshold(int index);
    private: bool initThresholdSet;
    private: int initThreshold;*/
public: HeapVisualizer();
public: void setInitThreshold(int index);
public: void draw(int location, QString prettyByte);
public: setHeapSize(QString size);
public: void redraw();
private: int size;
private: bool initThresholdSet;
private: int initThreshold;
public: int currentHighlighted;
public: QVector<QString> entries;
public: QVector<PText*> texts;
public: QVector<PRect*> squares;
};

#endif // HEAPVISUALIZER_H
