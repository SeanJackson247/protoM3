#include "pipe.h"

Pipe::Pipe(QString processName, QString pipeName){
    this->name=processName;
    this->flagged=false;
}

void Pipe::out(QString data){
    //this->flagged=true;
    //this->dataOut.append(data);
    //this->data.append(data);
}
/*
void Pipe::in(QString data){

}
*/
