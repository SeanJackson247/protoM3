#ifndef PRESIZEEVENT_H
#define PRESIZEEVENT_H

#include "pwidget.h"

#include <QResizeEvent>

class PWidget;

class PResizeEvent{
public: PResizeEvent(PWidget* pWidget,QResizeEvent* e);
public: void post();
public: void preventDefault();
public: bool posted;
public: PWidget* origin;
};

#endif // PRESIZEEVENT_H
