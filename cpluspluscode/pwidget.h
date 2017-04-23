#ifndef PWIDGET_H
#define PWIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>

#include "presizeevent.h"
#include "pclickevent.h"
#include "player.h"

class PResizeEvent;

class PClickEvent;

class PDrawable;

class PLayer;

class PWidget:QGraphicsView{
    Q_OBJECT
public: QPixmap canvas;
public: QGraphicsPixmapItem* canvasItem;
private: QGraphicsScene* scene;
//public: QGraphicsView* view;
public: PWidget(int w, int h);
public: void setTitle(QString title);
public: void show();
//public: void drawLine(int x,int y,int x2,int y2,QString colour);
//public: void drawRect(int x,int y,int w,int h,QString colour);
//public: void fillRect(int x,int y,int w,int h,QString colour);
//public: void drawText(QString text,int x,int y,int fs,QString colour);
public: int innerWidth();
public: int innerHeight();
public: int width();
public: int height();
public: void setWidth(int w);
public: void setHeight(int h);
public: void setRelativeWidth(int w);
public: void setRelativeHeight(int h);
public: int getRelativeWidth();
public: int getRelativeHeight();
public: int relativeWidth;
public: int relativeHeight;
public: void addDrawable(PDrawable* drawable);
//public: QVector<PLayer*> drawables;
public: QVector<PDrawable*> drawables;
protected: virtual void resizeEvent(QResizeEvent* event);
protected: virtual void mousePressEvent(QMouseEvent* event);
//public: void setMousePressCallback(void(*callback)(PWidget* p,QMouseEvent* e));
public: void (*onresize)(PWidget* p,PResizeEvent* e);
public: bool onresizeevent;
public: bool onclickevent;
public: void repaint();
//private: virtual void mousePressCallback(QMouseEvent* e);
public: void (*onclick)(PWidget* p,PClickEvent* e);
//protected: virtual void paintEvent(QPaintEvent *e);
public: void update();
public: void addItem(QGraphicsItem* item);
};

#endif // PWIDGET_H
