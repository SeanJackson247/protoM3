#include "pwidget.h"

#include <QPainter>
#include <QPen>
#include <QGraphicsView>
#include <QDebug>
#include <QMouseEvent>


PWidget::PWidget(int w,int h){
//    this->qwidget = new QWidget();
    this->scene = new QGraphicsScene(this);
    //this->view = new QGraphicsView(this->scene,this);
    this->setScene(this->scene);
    this->canvas = QPixmap(w,h);
    this->canvasItem = new QGraphicsPixmapItem(this->scene->addPixmap(this->canvas));
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->onresizeevent=false;
    this->drawables={};
    this->relativeWidth=100;
    this->relativeHeight=100;
    this->setWidth(w);
    this->setHeight(h);
    this->resize(w,h);
    this->update();
    this->repaint();
    this->update();
}
void PWidget::show(){
    QGraphicsView::show();
    //this->view->show();
}

void PWidget::addItem(QGraphicsItem* item){
    this->scene->addItem(item);
}

void PWidget::update(){
    QGraphicsView::update();
}
/*
void PWidget::setMousePressCallback(void (*callback)(PWidget* context,QMouseEvent *)){
    this->onclick=callback;
}*/

void PWidget::setTitle(QString title){
    this->setWindowTitle(title);
}
void PWidget::resizeEvent(QResizeEvent* event){
    QGraphicsView::resizeEvent(event);
    int i=0;
    while(i<this->drawables.size()){
       // this->drawables[i]->resize();
        i++;
    }
    qDebug()<<"Resize event!";
    PResizeEvent* e = new PResizeEvent(this,event);
    if(this->onresizeevent==true){
        this->onresize(this,e);
        if(e->posted==true){
      //      QWidget::resizeEvent(event);
        }
        else{
            qDebug()<<"Unhandled event!";
        //    QWidget::resizeEvent(event);
        }
    }
    else{
//        QWidget::resizeEvent(event);
    }

    QGraphicsView::fitInView(this->scene->sceneRect(), Qt::KeepAspectRatio);

    QGraphicsView::repaint();
 //   QGraphicsView::resizeEvent(event);*/
    //this->setSceneRect(0,0,this->frameSize().width(),this->frameSize().height());
}
/*
void PWidget::paintEvent(QPaintEvent* e){
    int i=0;

    //QPainter *paint = new QPainter(&this->canvas);
  //  paint->setPen(*(new QColor(colour)));
    this->canvas.fill(QColor("#000000"));
   // paint->end();
   // delete paint;
    this->canvasItem->setPixmap(this->canvas);

    while(i<this->drawables.size()){
        qDebug()<<"Drawing!";
        this->drawables[i]->drawOn(this);
        i++;
    }
    QGraphicsView::paintEvent(e);
}*/

void PWidget::repaint(){
    QGraphicsView::repaint();
}

void PWidget::addDrawable(PDrawable* drawable){
    this->drawables.append(drawable);
    //drawable->drawOn(this);
//    this->addItem(drawable);
}

int PWidget::innerHeight(){
    return this->canvas.height();
}
int PWidget::innerWidth(){
    return this->canvas.width();
}
int PWidget::width(){
    return QGraphicsView::width();
}
int PWidget::height(){
    return QGraphicsView::height();
}
/*void PWidget::drawLine(int x1,int y1,int x2,int y2,QString colour){
//    this->qwidget
 //   this->canvas->drawLine();
    QPainter *paint = new QPainter(&this->canvas);
    paint->setPen(*(new QColor(colour)));
    paint->drawLine(x1,y1,x2,y2);
    this->canvasItem->setPixmap(this->canvas); // Added this line
}
void PWidget::drawRect(int x, int y, int w, int h,QString colour){
}
void PWidget::fillRect(int x, int y, int w, int h,QString colour){
 /*   QPainter *paint = new QPainter(&this->canvas);
  //  paint->setPen(*(new QColor(colour)));
    paint->fillRect(QRect(x,y,w,h),QBrush(QColor(colour)));
    paint->end();
    delete paint;
    this->canvasItem->setPixmap(this->canvas); // Added this line*/
 /*   while(w>0 || h>0){
        this->drawRect(x,y,w,h,colour);
        w--;
        h--;
        if(w<0){
            w=0;
        }
        if(h<0){
            h=0;
        }
    }
}
void PWidget::drawText(QString text,int x,int y,int fs,QString colour){
    QPainter *paint = new QPainter(&this->canvas);
    paint->setPen(*(new QColor(colour)));
    paint->setFont( QFont("Arial") );
    paint->drawText( QPoint(x, y), text );
    paint->end();
    delete paint;
    this->canvasItem->setPixmap(this->canvas); // Added this line

}
*/
void PWidget::setWidth(int w){
    this->resize(w,this->height());
}

void PWidget::setHeight(int h){
    this->resize(this->width(),h);
}

void PWidget::setRelativeWidth(int w){
    this->relativeWidth=w;
}

void PWidget::setRelativeHeight(int h){
    this->relativeHeight=h;
}

int PWidget::getRelativeWidth(){
    return this->relativeWidth;
}

int PWidget::getRelativeHeight(){
    return this->relativeHeight;
}

void PWidget::mousePressEvent(QMouseEvent* qMouseEvent){
    qDebug()<<"Mouse Event!";
    QGraphicsView::mousePressEvent(qMouseEvent);
/*   // QWidget::mouseEvent(qMouseEvent);
    this->mousePressCallback(this,qMouseEvent);
    qDebug()<<qMouseEvent->x()<<qMouseEvent->y();*/
}
