#include "prect.h"

#include <QDebug>
#include <QPainter>
#include <QPen>

PRect::PRect(PWidget* parent, int x, int y, int w, int h, QString colour, int penSize){
    this->parent=parent;
    this->x=x;
    this->y=y;
    this->w=w;
    this->h=h;
    this->colour=colour;
    this->fill=penSize;
    this->inner = new QGraphicsRectItem(
        (((double)((double)x/(double)this->parent->relativeWidth))*this->parent->width()),
        (((double)((double)y/(double)this->parent->relativeHeight))*this->parent->height()),
        (((double)((double)w/(double)this->parent->relativeWidth))*this->parent->width()),
        (((double)((double)h/(double)this->parent->relativeHeight))*this->parent->height())
    );
    QPen pen = QPen(penSize);
    if(penSize==0){
        this->inner->setBrush(QColor(colour));
        pen.setStyle(Qt::NoPen);
    }
    else{
        pen.setColor(QColor(colour));
    }
    this->inner->setPen(pen);
    this->parent->addItem(this->inner);

}

void PRect::setValues(int x, int y, int w, int h, QString colour, int penSize){
    this->x=x;
    this->y=y;
    this->w=w;
    this->h=h;
    this->colour=colour;
    this->fill=penSize;
    this->inner->setBrush(QColor(colour));
    if(penSize==0){
        this->inner->pen().setStyle(Qt::NoPen);
    }
    this->inner->update(x,y,w,h);
/*    this->inner->setX(x);
    this->inner->setY(y);
    this->inner->setWidth(w);
    this->inner->setHeight(h);*/
    this->resize();
}

void PRect::resize(){
    qDebug()<<"Resizing rectangle!";
    int x=0;
    if(x!=this->x){
        x=(int)(((double)((double)this->x/(double)this->parent->relativeWidth))*this->parent->width());
    }
    int y=0;
    if(x!=this->x){
        y=(int)(((double)((double)this->y/(double)this->parent->relativeHeight))*this->parent->height());
    }
    int w=0;
    if(w!=this->w){
        w=(int)(((double)((double)this->w/(double)this->parent->relativeWidth))*this->parent->width());
    }
    int h=0;
    if(h!=this->h){
        h=(int)(((double)((double)this->h/(double)this->parent->relativeHeight))*this->parent->height());
    }
    qDebug()<<x<<","<<y<<","<<w<<","<<h;
    this->inner->update(x,y,w,h);
}

/*void PRect::drawOn(PWidget* pWidget){
    pWidget->addItem(this->inner);
    /*
    qDebug()<<"Old X"<<this->x<<" inner width: "<<pWidget->innerWidth()<<" relative width: "<<pWidget->relativeWidth;
    int x=0;
    if(this->x!=0){
        x=((int)((double)this->x*(double)pWidget->innerWidth()))/pWidget->relativeWidth;
    }
    qDebug()<<"New X"<<x;
    qDebug()<<"Old Y"<<this->y;
    int y=0;
    if(this->y!=0){
        y=((int)((double)this->y*(double)pWidget->innerHeight()))/pWidget->relativeHeight;
    }
    qDebug()<<"New Y"<<y;
    qDebug()<<"w:"<<this->w;
    int w=0;
    if(this->w!=0){
        w=(int)(((double)((double)this->w*(double)pWidget->innerWidth()))/pWidget->relativeWidth);
    }
    qDebug()<<"new w:"<<w;
    int h=0;
    if(this->h!=0){
        h=(int)(((double)((double)this->h*(double)pWidget->innerHeight()))/pWidget->relativeHeight);
    }
    if(fill==false){
        qDebug()<<"Drawing rectangle!";
        QPainter *paint = new QPainter(&pWidget->canvas);
        paint->setPen(*(new QColor(colour)));
        paint->drawRect(x,y,w,h);
        paint->end();
        delete paint;
        pWidget->canvasItem->setPixmap(pWidget->canvas); // Added this line
    }
    else{
        while(w>0 || h>0){
         //   qDebug()<<"Drawing rectangle!";
            QPainter *paint = new QPainter(&pWidget->canvas);
            paint->setPen(*(new QColor(colour)));
            paint->drawRect(x,y,w,h);
            paint->end();
            delete paint;
            pWidget->canvasItem->setPixmap(pWidget->canvas); // Added this line
            w--;
            h--;
            if(w<0){
                w=0;
            }
            if(h<0){
                h=0;
            }
        }
    }*/
/*}
*/
