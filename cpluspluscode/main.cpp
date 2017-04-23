#include <QApplication>

#include <QDebug>

#include "compiler2.h"
#include "filereader.h"
#include "filewriter.h"
#include "interpreter.h"
#include "widgetmanager.h"
#include "guirequest.h"
#include "pipemanager.h"
#include "eventhandler.h"
#include "pwidget.h"
#include "presizeevent.h"
#include "prect.h"
#include "pline.h"

#include "pclickevent.h"

#include "garbagecollector.h"
//class PResizeEvent;

void myResizeCallback(PWidget* p,PResizeEvent* e){
    qDebug()<<"my callback!";
   // e->preventDefault();
    e->origin->repaint();
    e->post();
}

void myClickCallback(PWidget* p,PClickEvent* e){
    qDebug()<<"my resize callback!";
   // e->preventDefault();
    e->post();
}

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    //QTypemetaregistration...

    //no i dont knowwhat thaat means either
  /*  PWidget* pwidget= new PWidget(400,400);
    pwidget->setRelativeWidth(100);
    pwidget->setRelativeHeight(100);
    pwidget->setTitle("PWidget Test");
    pwidget->addDrawable(new PRect(pwidget,0,0,100,100,"#aaaa78",0));
    pwidget->addDrawable(new PRect(pwidget,25,25,50,50,"#ffffff",0));
    pwidget->addDrawable(new PRect(pwidget,20,20,30,30,"#fedcef",6));
    pwidget->addDrawable(new PText(pwidget,"Test Text!",20,20,30,"#f777ef",200));
  //  pwidget->addDrawable(new PLine(0,0,100,100,"#aa78aa",6));
    //pwidget->addDrawable(new PLine(0,100,100,0,"#aa78aa",6));
    //pwidget->addDrawable(new PLine(0,50,100,50,"#000000",3));
/*
    int i=1;
    int len = 1000;
    while(i<len){
        int x=(i/100)*5;
        int y=(i%100)*5;
        qDebug()<<x<<","<<y;
        int w=5;
        int h=5;
        QString colour;
        bool f=false;
        if(i%2==0){
            colour="#ff7777";
            f=true;
        }
        else{
            colour="#77ff77";
            f=false;
            w--;
            h--;
        }
        pwidget->addDrawable(new PRect(pwidget,x,y,w,h,colour,f));
        i++;
    }*/
//    pwidget->drawRect(1,1,100,100,"#aaaa78");
//    pwidget->drawLine(0,0,pwidget->innerWidth(),pwidget->innerHeight(),"#ffffff");
/*    pwidget->repaint();
    pwidget->show();
    pwidget->onresize = myResizeCallback;
    pwidget->onresizeevent=true;

    pwidget->onclick = myClickCallback;
    pwidget->onclickevent=true;*/

    QThread::currentThread()->setObjectName("Proto GUI Thread...");

    typedef QVector<QString> StringVector;
    qRegisterMetaType<StringVector>("StringVector");
    typedef GUIRequest GUIRequest;
    qRegisterMetaType<GUIRequest>("GUIRequest");

    typedef PByteArray PByteArray;
    qRegisterMetaType<PByteArray>("PByteArray");


    qDebug()<<"Launched compiler...";

    Compiler2* compiler = new Compiler2();

    Interpreter* interpreter = new Interpreter(-1,0);

    GarbageCollector* garbageCollector = new GarbageCollector(interpreter);

    compiler->isRoot=true;

    FileWriter* fileWriter = new FileWriter("C:/proto/src/compiled/");

    FileReader* fileReader = new FileReader("C:/proto/src/");

    PipeManager* pipeManager = new PipeManager();

    QObject::connect(fileReader,SIGNAL(readFile(QString,QVector<QString>)),compiler,SLOT(doWork(QString,QVector<QString>)));

    QObject::connect(compiler,SIGNAL(debugScript(QString,QString)),fileWriter,SLOT(debugRequest(QString,QString)));

    //a bit weird but the compiler must connect to itself to recursively handle inclusions...

    QObject::connect(compiler,SIGNAL(exportToInclusion(QString,LexNode*)),compiler,SLOT(finalCompilation(QString,LexNode*)));

    //and now final compilation

    QObject::connect(compiler,SIGNAL(workCompleted(QString,PByteArray)),fileWriter,SLOT(writeRequest(QString,PByteArray)));

    QObject::connect(compiler,SIGNAL(workCompleted(QString,PByteArray)),interpreter,SLOT(launch(QString,PByteArray)));

    //now to connect the interpreted environment with the GUI

    WidgetManager* widgetManager = new WidgetManager();

    interpreter->widgetManager = widgetManager;
    interpreter->pipeManager = pipeManager;

    //no actually i mean connect the pipe manager to the gui

    //request to the gui

    QObject::connect(pipeManager,SIGNAL(guiRequest()),widgetManager,SLOT(recieveGUIRequest()));

    //and from the gui

    EventHandler* eventHandler = new EventHandler();
    //and we want to start the pipe manager running...
    QThread* pipeManagerThread = new QThread();

    QObject::connect(pipeManagerThread,SIGNAL(started()),pipeManager,SLOT(doWork()));

//    QObject::connect(eventHandler,SIGNAL(sendGUIEvent()),pipeManager,SLOT(receiveGUIEvent()));

    //and now from the interpreted environment to the pipeManager

    //QObject::connect(interpreter,SIGNAL(toPipeManagerSignal()),pipeManager,SLOT(receivePipeMessage()));

    //and back again

 //   QObject::connect(pipeManager,SIGNAL(toPipeSignal()),interpreter,SLOT(receivePipeMessage()));
    //now to distribute work among threads

    QThread* interpreterThread = new QThread();
    QThread* compilerThread = new QThread();
    QThread* fileIOThread = new QThread();
    QThread* gcThread = new QThread();

    interpreterThread->setObjectName("Proto Main Interpreter Thread");
    interpreter->threadName="Proto Main Interpreter Thread";
    interpreter->args = app.arguments();
    compilerThread->setObjectName("Proto Compiler Thread");
    fileIOThread->setObjectName("Proto File I/O Thread");
    pipeManagerThread->setObjectName("Proto Pipe Management Thread");

    compiler->moveToThread(compilerThread);
    interpreter->moveToThread(interpreterThread);
    fileReader->moveToThread(fileIOThread);
    fileWriter->moveToThread(fileIOThread);
    pipeManager->moveToThread(pipeManagerThread);
    garbageCollector->moveToThread(gcThread);

    Interpreter::runtimeInstances.append(interpreter);

    QObject::connect(gcThread,SIGNAL(started()),garbageCollector,SLOT(doWork()));

    gcThread->start();

    interpreterThread->start();
    compilerThread->start();

    fileIOThread->start();
    pipeManagerThread->start();

    fileReader->loadFile("main.proto");




    //and now we don't block the GUI Thread...

    //WidgetManager* widgetManager = new WidgetManager();
/*
    while(true){
   //     qDebug()<<"Running main GUILoop";
        while(GUIRequest::hasRequest==false){
            qDebug()<<"No GUI Request.";
        }
        QMutex* mutex = new QMutex();
        mutex->lock();
        QString object = GUIRequest::CurrentRequest->object;
        QString action = GUIRequest::CurrentRequest->action;
        QVector<QString> values = GUIRequest::CurrentRequest->values;
        mutex->unlock();
        qDebug()<<"RECEIVING GUIRequest!!";
        if(QString::compare(object,"terminal")==0){
            if(QString::compare(action,"launch")==0){
                widgetManager->Terminals.append(new Terminal(values[0]));
            }
        }
        mutex->lock();
        GUIRequest::hasRequest=false;
     //   QString object = GUIRequest::CurrentRequest.object;
       // QString action = GUIRequest::CurrentRequest.action;
       // QVector<QString> values = GUIRequest::CurrentRequest.values;
        mutex->unlock();
        delete mutex;
    }
*/
    return app.exec();
}
