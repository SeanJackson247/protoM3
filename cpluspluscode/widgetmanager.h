#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H

#include <QObject>
#include "guirequest.h"

#include "terminal.h"
#include "heapvisualizer.h"
#include "pwidget.h"

class Terminal;

class WidgetManager:public QObject{
    Q_OBJECT
public:
    WidgetManager();
public:QVector<Terminal*> Terminals;
public: QVector<PWidget*> pwidgets;
public: static int pwidgetIds;
//public:QVector<HeapVisualizer*> heapVisualizers;
//public: static QString CurrentRequestAction;
//public: static QString CurrentRequestObject;
public: static QVector<QVector<QString>> CurrentRequestValues;
public: static bool flagged;
public slots:
    void recieveGUIRequest();
};

#endif // WIDGETMANAGER_H
