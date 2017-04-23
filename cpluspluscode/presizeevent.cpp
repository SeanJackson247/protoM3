#include "presizeevent.h"

PResizeEvent::PResizeEvent(PWidget* pWidget,QResizeEvent* e){
    this->origin=pWidget;
   // this->e = e;
    this->posted=false;
}

void PResizeEvent::post(){
    this->posted=true;
}
void PResizeEvent::preventDefault(){
}
