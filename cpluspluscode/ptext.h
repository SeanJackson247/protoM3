#ifndef PTEXT_H
#define PTEXT_H

#include "pwidget.h"

class PText: public PDrawable{
public: PText(PWidget* parent,QString text,int x,int y,int fs,QString colour,int textWidth);
public: PWidget* parent;
public: int x;
public: int y;
public: int fs;
public: QString colour;
public: QString text;
public: QGraphicsTextItem* inner;
public: void setValues(QString text, int x, int y, int fs, QString colour, int textWidth);
};

#endif // PTEXT_H
