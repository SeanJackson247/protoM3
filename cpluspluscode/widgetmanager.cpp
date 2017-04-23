#include "widgetmanager.h"
#include "errorregistry.h"

#include <QDebug>

WidgetManager::WidgetManager(){

}

//QString WidgetManager::CurrentRequestAction="";

//QString WidgetManager::CurrentRequestObject="";

QVector<QVector<QString> > WidgetManager::CurrentRequestValues={};

//QVector<PWidget*> WidgetManager::pwidgets={};

bool WidgetManager::flagged=false;
int WidgetManager::pwidgetIds=0;

void WidgetManager::recieveGUIRequest(){
    qDebug()<<"recieved gui request...";
    WidgetManager::flagged=true;
    QMutex* mutex = new QMutex();

    QString terminalOut;
    mutex->lock();
    qDebug()<<"mutex locked...";
    int e=0;
    while(e<WidgetManager::CurrentRequestValues.size()){
        qDebug()<<"looping...";
        if(WidgetManager::CurrentRequestValues[e].size()>=2){
            if(QString::compare(WidgetManager::CurrentRequestValues[e][0],"heap.visualizer")==0){
                if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"launch")==0){
                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
                else if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"size")==0){
                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
                else if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"draw")==0){
                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
                else if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"init-threshold")==0){
                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
            }
            else if(QString::compare(WidgetManager::CurrentRequestValues[e][0],"app.debug")==0){
                if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"out")==0){
                    if(this->Terminals.size()==0){
                        this->Terminals.append(new Terminal("app.debug"));
                    }
//                    this->Terminals[0]->out(WidgetManager::CurrentRequestValues[e][2]);
                    terminalOut+=WidgetManager::CurrentRequestValues[e][2];
                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
            }
            else if(QString::compare(WidgetManager::CurrentRequestValues[e][0],"PWidget")==0){
                if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"create")==0){

                    qDebug()<<"PWidget Request!!!!!!!";
                    bool ok;
                    int w = WidgetManager::CurrentRequestValues[e][2].toInt(&ok,10);
                    if(!ok){
                        w = 800;
                    }
                    int h = WidgetManager::CurrentRequestValues[e][3].toInt(&ok,10);
                    if(!ok){
                        h = 800;
                    }
                    this->pwidgets.append(new PWidget(w,h));
                    WidgetManager::pwidgetIds++;
                   // this->pwidgets.last()->setTitle(WidgetManager::CurrentRequestValues[e][4]);


            //        this->pwidgets.append(new PWidget(300,300));
                    this->pwidgets.last()->show();

                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
                else if(QString::compare(WidgetManager::CurrentRequestValues[e][1],"show")==0){

                    qDebug()<<"Pwidget show!!!!!!!!!!!!!!!!!!!!!";

                    bool ok;
                    int id = WidgetManager::CurrentRequestValues[e][2].toInt(&ok,10);
                    if(!ok){
                        ErrorRegistry::logError("Could not get id - Pwidget.show");
                    }
                    this->pwidgets[id]->show();

                    qDebug()<<"PWidget Request!!!!!!!";

            //        this->pwidgets.append(new PWidget(300,300));
            //        this->pwidgets.last()->show();

                    WidgetManager::CurrentRequestValues.remove(e);
                    e--;
                }
            }
            else{
                ErrorRegistry::logError("Widget Manager received malformed request value:{"+WidgetManager::CurrentRequestValues[e][0]+","+WidgetManager::CurrentRequestValues[e][1]+"}");
            }
        }
        else{
            if(WidgetManager::CurrentRequestValues[e].size()==1){
                ErrorRegistry::logError("Widget Manager received malformed request value:{"+WidgetManager::CurrentRequestValues[e][0]+"}");
            }
            else{
                ErrorRegistry::logError("Widget Manager received malformed request value:{"+WidgetManager::CurrentRequestValues[e][0]+","+WidgetManager::CurrentRequestValues[e][1]+"}");
            }
            //WidgetManager::CurrentRequestValues.remove(e);
        }
        e++;
    }

    if(QString::compare(terminalOut,"")!=0){
        this->Terminals[0]->out(terminalOut);
    }

    WidgetManager::CurrentRequestValues={};
    qDebug()<<"Mutex unlocking...";
    WidgetManager::flagged=false;
    mutex->unlock();
    qDebug()<<"Mutex unlocked...";
    delete mutex;
    qDebug()<<"finished gui request...";
}
