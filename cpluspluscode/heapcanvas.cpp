#include "heapcanvas.h"

#include <QPixmap>

HeapCanvas::HeapCanvas(QWidget *parent, HeapVisualizer *heapVisualizer):QGraphicsView(parent){
  //  this->scene = new QGraphicsScene(this);
   // this->setScene(this->scene);
   // this->setSceneRect(0,0,100,100);
    //QPixmap* pMap = new QPixmap();
    //this->addItem(pMap);
}
/*
void HeapCanvas::paintEvent(QPaintEvent* event){
//    this->scene = new QGraphicsScene(this);
    qDebug()<<"I'm Painting!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
/*
    QPainter painter(this);
        QPen pen(Qt::red);
        pen.setWidth(5);
        painter.setPen(pen);
        painter.drawLine(10,10,100,100);
      //  this->window->repaint();*/
/*}
*/

void HeapCanvas::paintEvent(QPaintEvent* event){
  //  QPainter painter(this);
  //  QPen pen(Qt::red);
  //  pen.setWidth(5);
  //  painter.setPen(pen);
  //  painter.drawLine(10,10,100,100);
}
