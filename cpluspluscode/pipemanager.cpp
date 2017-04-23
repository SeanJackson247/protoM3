#include "pipemanager.h"

#include <QDebug>
#include <QMutex>

#include "widgetmanager.h"

#include "errorregistry.h"

QVector<Pipe*> PipeManager::Pipes={};

PipeManager::PipeManager(){
}

void PipeManager::doWork(){
    while(true){
        //qDebug()<<"Pipe Check!";
        int i=0;
        while(i<PipeManager::Pipes.size()){

            if(PipeManager::Pipes[i]->flagged==true){
                qDebug()<<"Found flagged pipe...";
                if(i==0 || i==1){

                    while(WidgetManager::flagged==true){
                        qDebug()<<"pipe manager wating...";
                    }
                    QMutex* mutex=new QMutex();
                    mutex->lock();
                    auto _data = PipeManager::Pipes[i]->data;
                    if(_data.size()>0){
                        WidgetManager::CurrentRequestValues.append({"app.debug"});
                        int e=0;
                        while(e<_data.size()){
                            WidgetManager::CurrentRequestValues.last().append(_data[e]);
                            e++;
                        }
                        if(e==0){
                            ErrorRegistry::logError("Pipe Manager received malformed app.debug request.");
                        }
                    }
                    qDebug()<<"emitting gui request...";

                    PipeManager::Pipes[i]->data={};
                    PipeManager::Pipes[i]->flagged=false;

                    mutex->unlock();
                    delete mutex;
                    qDebug()<<"unlocked...";
                    WidgetManager::flagged=true;
                    emit this->guiRequest();
                }
                else if(i==2){

                    while(WidgetManager::flagged==true){
                        qDebug()<<"pipe manager wating...";
                    }
                    QMutex* mutex=new QMutex();
                    mutex->lock();
                    auto _data = PipeManager::Pipes[i]->data;
                    if(_data.size()>0){
                        WidgetManager::CurrentRequestValues.append({"PWidget"});
                        int e=0;
                        while(e<_data.size()){
                            WidgetManager::CurrentRequestValues.last().append(_data[e]);
                            e++;
                        }
                        if(e==0){
                            ErrorRegistry::logError("Pipe Manager received malformed app.debug request.");
                        }
                    }
                    qDebug()<<"emitting gui request...";

                    PipeManager::Pipes[i]->data={};
                    PipeManager::Pipes[i]->flagged=false;

                    mutex->unlock();
                    delete mutex;
                    qDebug()<<"unlocked...";
                    WidgetManager::flagged=true;
                    emit this->guiRequest();
                }
            }
            i++;
        }
    }
}

void PipeManager::receiveGUIRequest(){

}
void PipeManager::receiveGUIEvent(){

}
void PipeManager::receivePipeMessage(){

}
