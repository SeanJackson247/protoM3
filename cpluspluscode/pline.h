#ifndef PLINE_H
#define PLINE_H

#include "pwidget.h"

class PDrawable;

class PLine: public PDrawable{
public: PLine(int x, int y, int x2, int y2, QString colour,int width);
public: void drawOn(PWidget* pWidget);
public: int x;
public: int y;
public: int x2;
public: int y2;
public: QString colour;
public: int width;
};

#endif // PLINE_H
