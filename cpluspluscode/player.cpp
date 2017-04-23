#include "player.h"

PLayer::PLayer(){
    this->drawables={};
}
void PLayer::drawOn(PWidget* pWidget){
    int i=0;
    while(i<this->drawables.size()){
        //this->drawables[i]->drawOn(pWidget);
        i++;
    }
}
