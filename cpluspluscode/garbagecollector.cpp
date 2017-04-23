#include "garbagecollector.h"
GarbageCollector::GarbageCollector(Interpreter* i){
    this->p=i;
}
void GarbageCollector::doWork(){
    qDebug()<<"gc thread...";
    /*while(this->p->heap==NULL){}
    while(this->p->heap->data==NULL){}
    while(this->p->heap->initThreshold==0){}
    while(true){
        int i=this->p->heap->initThreshold;
        int nullsInARow=0;
        int nullsInARowStart=0;
        int greatestNullsInARow=0;
        int greatestNullsInARowStart=0;
        while(i<this->p->heap->data->size()){
            unsigned char dat = this->p->heap->data->getByte(i);
            if(dat!=NULL){
            //    qDebug()<<"Nulls in a row:"<<nullsInARow;
                if(nullsInARow>greatestNullsInARow){
                    greatestNullsInARow = nullsInARow;
                    greatestNullsInARowStart=nullsInARowStart;
                }
                nullsInARow=0;
            }
            else{
                if(nullsInARow==0){
                    nullsInARowStart=i;
                }
                nullsInARow++;
            }
            i++;
        }
        qDebug()<<"greatest nulls in a row: "<<greatestNullsInARow<<", at "<<greatestNullsInARowStart;
    }*/
}
