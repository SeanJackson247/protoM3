#include "errorregistry.h"

#include <QDebug>

ErrorRegistry::ErrorRegistry(){

}
int ErrorRegistry::errorCount=0;

QString ErrorRegistry::printLog(){
    int i=0;
    QString res = "\nError Log:\n";
   // while(i<ErrorRegistry::log.size()){
    while(i<ErrorRegistry::log.size()){
        res+=ErrorRegistry::log[i]+="<br>";
        i++;
    }
    return res;
}

QVector<QString> ErrorRegistry::log;

void ErrorRegistry::logError(QString data){
    qDebug().noquote()<<"Logging error:"<<data;
    ErrorRegistry::log.append(data);
    ErrorRegistry::errorCount++;
}
