#include "compiler2.h"

#include <QDebug>
#include <QTextEncoder>

#include <QFile>
#include <QTextStream>
#include <QIODevice>

#include <QElapsedTimer>
#include <QMutex>

#include "filereader.h"
#include "line.h"
#include "linestatic.h"
#include "errorregistry.h"
#include "heapstore.h"

Compiler2::Compiler2(){

}
void Compiler2::doWork(QString fileName, QVector<QString> data){
    qDebug()<<"Compiling...";
    this->exportDirectory = fileName;
    if(data.size()==2 && QString::compare(data.first(),"ERROR")==0){
        qDebug()<<"Error Encountered...";
        //emit exportToInclusion(fileName,{new Line(0,"ERROR"),new Line(0,data.last())});
        //emit exportToInclusion(fileName,new LexNode({true},{"native('Error.throw','Error during includes!');"}));
    }
    else{
        qDebug()<<"File read..";
        QElapsedTimer myTimer;
        myTimer.start();

        QString finalString="TEST\n";

        int i=0;
        while(i<data.size()){
            finalString+=data[i]+"\n";
            i++;
        }
/*
        i=0;
        int lineNumber=1;
        QVector<Line*> lines;
        Line::root = new Line(Line::TypeRoot);
        while(i<data.size()){
            lines.append(new Line(lineNumber,data[i]));
            lineNumber++;
            i++;
        }

        finalString+="\nRaw Lines:\n";

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }
        lines = LineStatic::removeEmptyLines(lines);

        finalString+="\nRemoved empty Lines:\n";

        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }
*/
        qDebug()<<finalString;

  //      lines = LineStatic::validateAsFirstOrderLines(0,lines);

        LexNode* lexNode = LineStatic::buildTree(data);

        //PByteArray* bytes = lexNode->compile(true);

        lexNode->compileToTokenStream(true);
        QVector<QVector<Token2*>> lines=lexNode->data;

        if(ErrorRegistry::errorCount==0){

            int i=0;
            while(i<lines.size()){
                //finalString+=lines[i]->toString(0);
//                qDebug().noquote()<<lines[i]->toString(0);
                i++;
            }

            //compile time inclusion

            qDebug()<<"Compile time inclusion...";

            //bit more complicated than static method. we'll be multithreading.

            QMutex* qMutex = new QMutex();

            qMutex->lock();

            this->linesStore = lexNode;

            qMutex->unlock();

            qDebug()<<"get includes urls...";

            QVector<QString> urls = lexNode->getIncludeURLs();//LineStatic::getIncludesURLS(lines);

            qDebug()<<"got includes urls...";
            qMutex->lock();

            this->includeURLs = urls;

            qMutex->unlock();

            delete qMutex;

            i=0;
            while(i<this->includeURLs.size()){

                qDebug()<<"Reslving includ URL:"<<this->includeURLs[i];

                Compiler2* compiler = new Compiler2();

                compiler->isRoot=false;

                FileReader* fileReader = new FileReader("C:/proto/src/");

                QObject::connect(fileReader,SIGNAL(readFile(QString,QVector<QString>)),compiler,SLOT(doWork(QString,QVector<QString>)));

                QObject::connect(compiler,SIGNAL(exportToInclusion(QString,LexNode*)),this,SLOT(recieveIncludes(QString,LexNode*)));

//                QObject::connect(compiler,SIGNAL(finalCompilation(QString,QVector<Line*>)),this,SLOT(recieveIncludes(QString,QVector<Line*>)));

                fileReader->loadFile(this->includeURLs[i]);
                i++;
            }
            if(i==0){
                qDebug()<<"No inclusions found...";
                this->includeURLs={};
                this->linesStore=lexNode;
                emit this->exportToInclusion(fileName,lexNode);
             //   finalCompilation(fileName,lines);
            }

           // lines = Line::pullIncludes(lines);

            /*
            lines = HeapStore::constructLexTree(lines);
            if(ErrorRegistry::errorCount==0){
                lines = Line::compile(lines);
                i=0;
                while(i<lines.size()){
                    finalString+=lines[i]->toString(0);
                    i++;
                }
                lines = HeapStore::extractConstants(lines);
                i=0;
                while(i<lines.size()){
                    finalString+=lines[i]->toString(0);
                    i++;
                }*/
      /*      }
            else{
                finalString+=ErrorRegistry::printLog();
            }*/
        }

        int ms = myTimer.elapsed();

        float secs = float(ms)/1000.00;

        qDebug() << "Compilation finished, took "<<ms<<" milliseconds.("<<secs<<") seconds.";
       // emit debugScript(fileName,finalString);
        //emit exportToInclusion(fileName,lexNode);
    }
}
void Compiler2::recieveIncludes(QString fileName, LexNode* lines){
    qDebug()<<"Recieving Includes...";

    qDebug()<<lines->toString(0);

    int e=0;
/*    while(e<lines.size()){
        qDebug()<<lines[e]->toString(0);
        e++;
    }*/
/*    if(lines.size()==2 && QString::compare(lines.first()->getData().first()->getData().first()->getString(),"ERROR")==0){
        qDebug()<<"FILEREADER ERROR";
        QString str = "ERROR:"+(lines.last()->toString(0));
        ErrorRegistry::logError(str);
        this->includeURLs={};
    }
    else{*/
        qDebug()<<"Appending includes lines...";

        QMutex* qMutex = new QMutex();

        qMutex->lock();

        this->linesStore->prepend(lines);
        if(this->includeURLs.size()>0){
            this->includeURLs.removeLast();
        }
        qDebug()<<"Escaped loop!";
        qMutex->unlock();

      //  delete qMutex;
   // }
    //QMutex* qMutex = new QMutex();

    qMutex->lock();

    if(this->includeURLs.size()==0){
        if(this->isRoot==true){
            qDebug()<<"Final Compilation...";
            emit this->finalCompilation(this->exportDirectory,this->linesStore);
        }
        else{
            qDebug()<<"Exporting to higher level...";
            emit this->exportToInclusion(this->exportDirectory,this->linesStore);
        }
    }

    qMutex->unlock();

    delete qMutex;

}
void Compiler2::finalCompilation(QString fileName, LexNode *lexNode){
    qDebug()<<"Final Compilation method...";
    if(ErrorRegistry::errorCount>0){
        QString finalString = ErrorRegistry::printLog();
        emit debugScript(fileName,finalString);
    }
    else{
        QString finalString="\nPost-File Inclusion Tree::\n";

        //lines = LineStatic::stripIncludes(lines);

        //lexNode->stripIncludes();

/*        finalString+="\nPre-Control Flow flattening::\n";

        int i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }*/

      //  lines = LineStatic::flattenControlFlows(lines);

        finalString+="\nPost-Control Flow flattening::\n";

/*        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }*/
        /*Now a bunch of data needs to be transferred from the instruction stream to the heap*/

   //     lines = LineStatic::labelLexicalScopes(lines);

    //    lines = LineStatic::flushToHeapStore(lines);

  /*      int i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }
*/
        int i=0;
        while(i<HeapStore::lexes.size()){
            finalString+="\nheap-rule[lex:'"+QString::number(HeapStore::lexes[i])+
            "' maps to : '"+
            QString::number(HeapStore::pars[i])+
            "'];\n";
            i++;
        }

        i=0;
        while(i<HeapStore::primitives.size()){
            qDebug()<<"has primitives...";
            finalString+="\n"+QString::number(i)+"heap-primitive["+HeapStore::primitives[i]+"];\n";
            i++;
        }

        i=0;
        while(i<HeapStore::literals.size()){
            qDebug()<<"has literals...";
            finalString+="\n"+QString::number(i)+"heap-literal["+HeapStore::literals[i]->toString(0)+"];\n";
            i++;
        }

/*        i=0;
        while(i<lines.size()){
            finalString+=lines[i]->toString(0);
            i++;
        }*/

        /*WE are finally here!! time to finally compile...*/

       // QVector<ByteCodeInstruction*> byteCodeInstructions = LineStatic::compile(lines);
/*
        finalString+="\nBYTECODE INSTRUCTIONS:\n";

        qDebug()<<"debugging...";

        i=0;
        while(i<byteCodeInstructions.size()){
            finalString+=QString::number(i)+":"+byteCodeInstructions[i]->toString(0);
            i++;
        }

        //final byte code clean-up

        byteCodeInstructions = ByteCodeInstruction::cleanup(byteCodeInstructions);

        byteCodeInstructions = ByteCodeInstruction::ammendControlFlows(byteCodeInstructions);

        finalString+="\nCLEANED-UP BYTECODE INSTRUCTIONS:\n";

        qDebug()<<"debugging...";

        i=0;
        while(i<byteCodeInstructions.size()){
            finalString+=QString::number(i)+":"+byteCodeInstructions[i]->toString(0);
            i++;
        }

        qDebug()<<"Passed bytecode instructions to final string...";
*/
        //now the heapstore pastes it together for us:

     //   PByteArray* bytes = HeapStore::encode(byteCodeInstructions);

        qDebug()<<"Final Compilation being called...";

        PByteArray* bytes = lexNode->compile(true);



 /*       qDebug()<<"Got bytes!";

        //we spit out the debug info...

        qDebug()<<"Emitting debug...";

        emit debugScript(fileName,finalString);
        //and the compiled version...
        qDebug()<<"Emitted debug...";
*/
        QString finalFileName = "";

        i=0;
        bool esc=false;
        while(i<fileName.size() && esc==false){
            if(fileName[i]=='.'){
                esc=true;
            }
            else{
                finalFileName.append(fileName[i]);
            }
            i++;
        }
        finalFileName+=".prt";
        qDebug()<<"Emitting work complete...";

        emit workCompleted(finalFileName,*bytes);

        qDebug()<<"Emitted work complete...";
    }
}
