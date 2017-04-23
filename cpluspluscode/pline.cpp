#include "pline.h"

#include<QPainter>
#include<QPen>

PLine::PLine(int x, int y, int x2, int y2, QString colour, int width){
    this->x=x;
    this->y=y;
    this->x2=x2;
    this->y2=y2;
    this->colour=colour;
    this->width=width;
}

void PLine::drawOn(PWidget* pWidget){
    int x=0;
    if(this->x!=0){
        x=((int)((double)this->x*(double)pWidget->innerWidth()))/pWidget->relativeWidth;
    }
    int y=0;
    if(this->y!=0){
        y=((int)((double)this->y*(double)pWidget->innerHeight()))/pWidget->relativeHeight;
    }
    int x2=0;
    if(this->x2!=0){
        x2=(int)(((double)((double)this->x2*(double)pWidget->innerWidth()))/pWidget->relativeWidth);
    }
    int y2=0;
    if(this->y2!=0){
        y2=(int)(((double)((double)this->y2*(double)pWidget->innerHeight()))/pWidget->relativeHeight);
    }

    QPainter *paint = new QPainter(&pWidget->canvas);
    QPen* pen = new QPen(QColor(colour));
    pen->setWidth(this->width);
    paint->setPen(*(pen));
    paint->drawLine(x,y,x2,y2);
    pWidget->canvasItem->setPixmap(pWidget->canvas); // Added this line
    paint->end();
    delete paint;
}
