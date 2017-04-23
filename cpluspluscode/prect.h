#ifndef PRECT_H
#define PRECT_H

#include <pdrawable.h>

#include <QString>

#include <QGraphicsRectItem>

class PDrawable;

class PRect: public PDrawable{
public:PRect(PWidget* parent,int x,int y,int w,int h,QString colour,int penSize);
public: PWidget* parent;
public: int x;
public: int y;
public: int w;
public: int h;
public: QGraphicsRectItem* inner;
public: QString colour;
public: bool fill;
//public: void drawOn(PWidget* pWidget);
public: void resize();
public: void setValues(int x, int y, int w, int h, QString colour, int penSize);
};

#endif // PRECT_H
