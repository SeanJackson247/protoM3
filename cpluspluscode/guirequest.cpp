#include "guirequest.h"
/*
GUIRequest::GUIRequest(QString object, QString action, QVector<QString> data){

}*/

GUIRequest::GUIRequest(){

}

bool GUIRequest::hasRequest=false;
GUIRequest* GUIRequest::CurrentRequest=NULL;
