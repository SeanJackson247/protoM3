#include "ptext.h"

PText::PText(PWidget *parent, QString text, int x, int y, int fs, QString colour, int textWidth){
    this->parent=parent;
    this->x=x;
    this->y=y;
    this->fs=fs;
    this->colour=colour;
    this->text=text;
    this->inner = new QGraphicsTextItem(text);
    QFont qFont;
    qFont.setPixelSize(fs);
    this->inner->setFont(qFont);
    this->inner->setX(
                (((double)((double)x/(double)this->parent->relativeWidth))*this->parent->width())
                );
    this->inner->setY(
                (((double)((double)y/(double)this->parent->relativeHeight))*this->parent->height())
                );
    this->inner->setTextWidth(textWidth);
    this->parent->addItem(this->inner);
}

void PText::setValues(QString text, int x, int y, int fs, QString colour,int textWidth){
    this->inner->setTextWidth(textWidth);
    this->x=x;
    this->y=y;
    this->fs=fs;
    this->colour=colour;
    //this->inner->setBrush(QColor(colour));
//    this->inner->update(x,y,fs);
    /*    this->inner->setX(x);
    this->inner->setY(y);
    this->inner->setWidth(w);
    this->inner->setHeight(h);*/
    this->resize();
}
