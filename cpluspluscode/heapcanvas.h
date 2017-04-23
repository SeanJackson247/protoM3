#ifndef HEAPCANVAS_H
#define HEAPCANVAS_H

#include "heapvisualizer.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsScene>

//#include <QQuickPaintedItem>
class HeapVisualizer;

class HeapCanvas : public QGraphicsView{
//class HeapCanvas : public QQuickPaintedItem{
    Q_OBJECT
protected: void paintEvent(QPaintEvent* event);
public: HeapCanvas(QWidget *parent,HeapVisualizer* heapVisualizer);
public:QGraphicsScene* scene;
};
#endif // HEAPCANVAS_H
