#include "nativestore.h"

#include "qwidget.h"
#include "qlayout.h"
#include "qtextbrowser.h"
#include "qicon.h"

#include "pipemanager.h"

NativeStore::NativeStore(Interpreter* interpreter){
    this->interpreter = interpreter;
}
int NativeStore::lookUpProperty(QString object, QString key){
    if(QString::compare(object,"sys")==0){
        if(QString::compare(key,"out")==0){
            return 1;
        }
    }
    return 0;
}

void NativeStore::wirePipe(int pipe, int NativeAddress){
    this->pipes.append(new PipeConnection(pipe,NativeAddress));
}


bool NativeStore::SysLaunched=false;

//Terminal* NativeStore::SysTerminal=NULL;

void NativeStore::SysLaunch(){
/*    int ow = 480;
    int h = 480;
    QWidget* window = new QWidget();
    QIcon qicon("./images/icon.png");
    window->setWindowIcon(qicon);
    window->resize(ow, h);
    window->setWindowTitle("sys.out");

    auto layout = new QVBoxLayout(window);
    auto *label = new QTextEdit("");

    QTextBrowser* browser = new QTextBrowser();

    browser->setDocument(label->document());
    layout->addWidget(browser);
    window->setStyleSheet("background-color: black;color:blue;text-decoration: bold;font-family:Courier New;font-size:16px;");
    window->show();

    NativeStore::browser=browser;
*/
    NativeStore::SysLaunched=true;
    //NativeStore::SysTerminal = new Terminal("sys.out");
}

void NativeStore::SysOut(QString str){
//    NativeStore::browser->setText(browser->toPlainText()+str+"\n");
  //  NativeStore::SysTerminal->out(str);
}

void NativeStore::resolve(int _address, QVector<QString> argument){

 //   QMutex* mutex=new QMutex();

//    mutex->lock();

    if(_address==NativeStore::SysOutAddress){
        qDebug()<<QThread::currentThread()<<"Hello! Resolving native sys.out code...";
        int i=0;
        while(i<this->pipes.size()){
      //      qDebug()<<QThread::currentThread()<<"Found pipe!";
            qDebug()<<"checking pipe...";
            if(this->pipes[i]->id==_address){
                int pipeAddress = this->pipes[i]->pipe;
                QMutex* mutex=new QMutex();
                while(PipeManager::Pipes[pipeAddress]->flagged==true){
                //    qDebug()<<"native store waits...";
                }
                mutex->lock();
        //        qDebug()<<"mutex is locked!";
                qDebug()<<"adding sys.out from native store..."<<pipeAddress;

                auto outputlist = argument[0].split("\\n");

                auto output = outputlist.join("</font><font color='#aaaaaa'>\\n</font><br><font color='#ffffff'>");

                outputlist = output.split("\\t");

                output = outputlist.join("</font><font color='#aaaaaa'>&gt;&gt;&gt;&gt;</font><font color='#ffffff'>");

                //outputlist = output.split("\\\n");

              //  output = outputlist.join("\\n");

                PipeManager::Pipes[pipeAddress]->data={"<font color='#ffffff'>"+output+"</font>"};
                PipeManager::Pipes[pipeAddress]->data.prepend("out");
                PipeManager::Pipes[pipeAddress]->flagged=true;
                qDebug()<<QThread::currentThread()<<"Appended data to debug pipe:"<<argument<<",on pipe"<<pipeAddress;
                mutex->unlock();
                delete mutex;
            }
            i++;
        }
    }
    else if(_address==NativeStore::DebugAddress){
     //   qDebug()<<QThread::currentThread()<<"Hello! Resolving native debug code...";
       // qDebug()<<QThread::currentThread()<<"Number of pipes to check..."<<this->pipes.size();
        int i=0;
        while(i<this->pipes.size()){
         //   qDebug()<<"Checking pipe..."<<i;
            if(this->pipes[i]->id==_address){
           //     qDebug()<<"Found pipe...";
                int pipeAddress = this->pipes[i]->pipe;
             //   qDebug()<<QThread::currentThread()<<"Waiting on pipe...";
                QMutex* mutex=new QMutex();

               // qDebug()<<"created mutex!";
                while(PipeManager::Pipes[pipeAddress]->flagged==true){
                //    qDebug()<<"native store debug waits...";
                }
                mutex->lock();
        //        qDebug()<<"mutex is locked!";
//                argument.prepend("app.debug");
                PipeManager::Pipes[pipeAddress]->data=argument;
                PipeManager::Pipes[pipeAddress]->flagged=true;
          //      qDebug()<<QThread::currentThread()<<"Appended data to debug pipe:"<<argument<<",on pipe"<<pipeAddress;
                mutex->unlock();
                delete mutex;
            }
            i++;
        }
        qDebug()<<"Checked pipes...";
    }

    else if(_address==NativeStore::PWidgetManager){
     //   qDebug()<<QThread::currentThread()<<"Hello! Resolving native debug code...";
       // qDebug()<<QThread::currentThread()<<"Number of pipes to check..."<<this->pipes.size();
        int i=0;
        while(i<this->pipes.size()){
         //   qDebug()<<"Checking pipe..."<<i;
            if(this->pipes[i]->id==_address){
           //     qDebug()<<"Found pipe...";
                int pipeAddress = this->pipes[i]->pipe;
             //   qDebug()<<QThread::currentThread()<<"Waiting on pipe...";
                QMutex* mutex=new QMutex();

               // qDebug()<<"created mutex!";
                while(PipeManager::Pipes[pipeAddress]->flagged==true){
                //    qDebug()<<"native store debug waits...";
                }
                mutex->lock();
        //        qDebug()<<"mutex is locked!";
//                argument.prepend("app.debug");
                PipeManager::Pipes[pipeAddress]->data=argument;
                PipeManager::Pipes[pipeAddress]->flagged=true;
          //      qDebug()<<QThread::currentThread()<<"Appended data to debug pipe:"<<argument<<",on pipe"<<pipeAddress;
                mutex->unlock();
                delete mutex;
            }
            i++;
        }
        qDebug()<<"Checked pipes...";
    }

    qDebug()<<"finished native...";
//    mutex->unlock();
 //   delete mutex;
}

void NativeStore::resolve(int _address, int argument){
    this->resolve(_address,{QString::number(argument)});
}

void NativeStore::resolveString(int _address, QString argument){
    this->resolve(_address,{argument});
}
