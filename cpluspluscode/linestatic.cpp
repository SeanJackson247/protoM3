#include "linestatic.h"
#include "errorregistry.h"
#include "heapstore.h"
#include <QDebug>
#include "lexnode.h"

LineStatic::LineStatic(){

}

QVector<Line*> LineStatic::validateAsClassPropertySignatures(int indent,QVector<Line*> lines){
    return lines;
}
QVector<Line*> LineStatic::validateAsLiteralPropertySignatures(int indent,QVector<Line*> lines){
    return lines;
}
QVector<Line*> LineStatic::validateAsFirstOrderLines(int indent,QVector<Line*> lines){
    /*This is the major validation algorithm for multi-line statements.*/
    bool cleanSweep=false;
    QVector<Line*> newLines;
    while(cleanSweep==false && ErrorRegistry::errorCount<1){
        int i=0;
        cleanSweep=true;
        while(i<lines.size() && ErrorRegistry::errorCount<1){
            if(cleanSweep==true){
                if(lines[i]->getIndent()==indent){
                    if(lines[i]->isValidComplete()==true){
                        newLines.append(lines[i]);
                        newLines.last()->setHasChanged(false);
                    }
                    else{
                        if(i!=lines.size()-1){
                            int expectedIndentation = lines[i]->expectedIndentation();
                            qDebug()<<"\nRaw line "+QString::number(i)+" has an expected indentation of "+QString::number(expectedIndentation)+". Lines has size of "+QString::number(lines.size())+", i is currently: "+QString::number(i)+"\n";
                            if(lines[i+1]->getIndent()>=expectedIndentation){
                                qDebug()<<"\nRaw line "+QString::number(i+1)+" has an actual indentation of "+QString::number(lines[i+1]->getIndent())+"\n";
                                cleanSweep=false;
                                newLines.append(new Line(lines[i],lines[i+1]));
                                i++;
                            }
                            else{
/*                                if(newLines.size()>0){
                                    if(newLines.last()->hasChanged()==false){
                                        ErrorRegistry::logError("Compiler error, could not resolve line "+QString::number(newLines.last()->getLineNumber()));
                                    }
                                    else{
                                        cleanSweep=false;
                                        newLines.append(new Line(lines[i],lines[i+1]));
                                        i++;
                                    }
                                }
                                else{*/
                                    cleanSweep=false;
                                    newLines.append(new Line(lines[i],lines[i+1]));
                                    i++;
  //                              }
                            }
                        }
                        else{
                         //   ErrorRegistry::logError("Error, Unexpected end of input.");
                        }
                    }
                }
                else{
                   /* if(i==0){
                        ErrorRegistry::logError("Error, first line may not be indented.");
                    }
                    else{*/
                        newLines.append(lines[i]);
                        newLines.last()->setHasChanged(false);
                    //}
                }
            }
            else{
                newLines.append(lines[i]);
                newLines.last()->setHasChanged(false);
            }
            i++;
        }
        if(cleanSweep==false){
            lines=newLines;
            newLines={};
        }
    }
    if(ErrorRegistry::errorCount<1){
        lines=newLines;
        newLines={};
        int i=0;
        while(i<lines.size()){
            if(lines[i]->getIndent()==indent){
                newLines.append(lines[i]);
            }
            else if(lines[i]->getIndent()>indent){
                if(newLines.size()==0){
                    //error
                }
                else{
                    Line* newLine = new Line(newLines.last(),lines[i]);
                    newLines.removeLast();
                    newLines.append(newLine);
                }
            }
            else{
                //error
            }
            i++;
        }
        i=0;
        while(i<newLines.size()){
     //       newLines[i]->validateChilds();
            i++;
        }
        return newLines;
    }
    else{
        return lines;
    }
}

QVector<Line*> LineStatic::stripIncludes(QVector<Line *> lines){
    int i=0;
    QVector<Line*> res;
    while(i<lines.size()){
        if(QString::compare(lines[i]->getData().first()->getData().first()->getString(),"include")!=0){
            res.append(lines[i]);
        }
        i++;
    }
    return res;
}

LexNode* LineStatic::buildTree(QVector<QString> data){
    qDebug()<<"Received in build tree:";
    int i=0;
    while(i<data.size()){
        qDebug()<<data[i];
        i++;
    }
    QString line="";
    i=0;
    while(i<data.size()){
        data[i] = data[i].split("//")[0];
        line+=data[i]+"\n";
        i++;
    }

    QVector<Line*> res = {};

    int o=0;
    QString container="";
    char separator=' ';
    QVector<bool> isCode;
    bool inString=false;

    QVector<QString> byString;

    while(o<line.length()){
     //   qDebug()<<o;
        if(line[o]=='\'' || line[o]=='"'){
            bool dontDo=false;
            //qDebug()<<"Separator encountered...";
            if(separator==' '){
              //  qDebug()<<"First separator...";
                if(inString==true){
                    inString=false;
                }
                else{
                    inString=true;
                }
                separator = line.at(o).toLatin1();
            }
            else{
                //qDebug()<<"Not first separator...";
                if(o>=2){
                  //  qDebug()<<"O greater than 1";
                    if(line.at(o).toLatin1()==separator && (line.at(o-1).toLatin1()!='\\' && (line.at(o-2).toLatin1()!='\\'))){
                        qDebug()<<"Non-escaped string container.";
                        if(inString==true){
                            inString=false;
                            separator=' ';
                        }
                        else{
                            inString=true;
                        }
                    }
                    else if(line.at(o).toLatin1()==separator && (line.at(o-1).toLatin1()!='\\' && (line.at(o-2).toLatin1()=='\\'))){
                 //       qDebug()<<"Non-escaped string container.";
                        if(inString==true){
                            inString=false;
                            separator=' ';
                        }
                        else{
                            inString=true;
                        }
                    }
                    else{
                        container+=line[o];
                        dontDo=true;
                    }
                }
                else if(o==1){
                    if(line.at(o).toLatin1()==separator && line.at(o-1).toLatin1()!='\\'){
                   //     qDebug()<<"Non-escaped string container.";
                        if(inString==true){
                            inString=false;
                            separator=' ';
                        }
                        else{
                            inString=true;
                        }
                    }
                    else{
                        container+=line[o];
                        dontDo=true;
                    }
                }
                else{//o==0
                    qDebug()<<"Non-escaped string container.";
                    if(line.at(o).toLatin1()==separator){
                        if(inString==true){
                            inString=false;
                            separator=' ';
                        }
                        else{
                            inString=true;
                        }
                    }
                }
            }
            //if(container.length()>0 && dontDo==false){
            if(dontDo==false){
                byString.append(container);
                isCode.append(inString);
                container="";
            }
        }
        else{
            container+=line[o];
        }
        /*
        QString str= "current bystring container:size(";
        str+=QString::number(byString.size());
        str+=")";
        qDebug()<<str;
        int q=0;
        while(q<byString.size()){
            str="["+byString[q]->getString();
            str+="][";
            str+=QString::number(
                 byString[q]->getString().size()
            );
            str+="]";
            qDebug()<<str;
            q++;
        }*/
        o++;
    }
    if(container.length()>0){
        byString.append(container);
        isCode.append(!inString);
        container="";
    }
    o=0;

    i=0;

    LexNode* lexNode = new LexNode(isCode,byString);

//    qDebug()<<"AFTER INITIAL BREAKUP:";

//    qDebug().noquote()<<lexNode->toString(0);

    return lexNode;
}

QVector<Line*> LineStatic::removeEmptyLines(QVector<Line*> lines){
   QVector<Line*> res;
   int i=0;
   while(i<lines.size()){
       int e=0;
       int size=0;
       while(e<lines[i]->data.size()){
           size+=lines[i]->data[e]->size();
           e++;
       }
       if(size>0 || lines[i]->childs.size()>0){
           res.append(lines[i]);
       }
       i++;
   }
   return res;
}

QVector<ByteCodeInstruction*> LineStatic::compileArguments(Line* parent,int childIndex, bool isMethod){
    qDebug()<<"LineStatic::Compiling arguments..."<<isMethod;
    QVector<ByteCodeInstruction*> res = {};

    if(isMethod==false){
        qDebug()<<"not method...";

        if(childIndex>=parent->getData().size()){
            qDebug()<<"Error - child index must be wrong...";
        }
        else{
            int i=0;
            while(i<parent->getData()[childIndex]->getData().size()-2){
                if(QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),"lambda")==0){
                    i++;
                    if(QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),"(")==0){
                        i++;
                        while(i<parent->getData()[childIndex]->getData().size() && QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),")")!=0){

                            if(QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),",")!=0){
                                res.append(new ByteCodeInstruction(ByteCodeInstruction::TypeEARG,parent->getData()[childIndex]->getData()[i],NULL,NULL));
                            }

                            i++;
                        }
                    }
                }
                i++;
            }
        }
    }
    else{
       // ErrorRegistry::logError("Compile Error:Unwritten Code: Method has arguments, for line:"+parent->getData()[childIndex]->toString());
        //qDebug()<<"Method has arguments, for line:"+parent->getData()[childIndex]->toString();
        if(childIndex>=parent->getData().size()){
            qDebug()<<"Error - child index must be wrong...";
        }
        else{
            int i=0;
            while(i<parent->getData()[childIndex]->getData().size()){
                if(QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),"(")==0){
                    i++;
                    while(i<parent->getData()[childIndex]->getData().size() && QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),")")!=0){
                        if(QString::compare(parent->getData()[childIndex]->getData()[i]->getString(),",")!=0){
                            qDebug()<<"adding argument for Method";
                            res.append(new ByteCodeInstruction(ByteCodeInstruction::TypeEARG,parent->getData()[childIndex]->getData()[i],NULL,NULL));
                        }

                        i++;
                    }
                }
                i++;
            }
        }
    }
    return res;
}

//final compilation method...
QVector<ByteCodeInstruction*> LineStatic::compile(QVector<Line*> lines){
    QVector<ByteCodeInstruction*> insts;
    int i=0;
    while(i<lines.size()){
        QVector<ByteCodeInstruction*> res = lines[i]->_compile();
        int e=0;
        while(e<res.size()){
            insts.append(res[e]);
            e++;
        }
        i++;
    }
    qDebug()<<"returning "+QString::number(insts.size())+" worth of instructions.";

    //now to hoist
    i=0;
    QVector<ByteCodeInstruction*> finalinsts;
    QVector<ByteCodeInstruction*> lettedinsts;
    QVector<ByteCodeInstruction*> fixedinsts;
    QVector<ByteCodeInstruction*> otherinsts;
    while(i<insts.size()){
        otherinsts.append(insts[i]);
        i++;
    }
    i=0;
    while(i<fixedinsts.size()){
        finalinsts.append(fixedinsts[i]);
        i++;
    }
    i=0;
    while(i<lettedinsts.size()){
        finalinsts.append(lettedinsts[i]);
        i++;
    }
    i=0;
    while(i<otherinsts.size()){
        finalinsts.append(otherinsts[i]);
        i++;
    }

    return finalinsts;
}


QVector<Line*> LineStatic::labelLexicalScopes(QVector<Line*> lines){
    int i=0;
    while(i<lines.size()){
        lines[i]->_labelLexicalScopes();
        i++;
    }
    return lines;
}

QVector<Line*> LineStatic::flushToHeapStore(QVector<Line*> lines){
    int i=0;
    while(i<lines.size()){
        qDebug()<<"i:="<<QString::number(i)<<";";
        if(lines[i]->type==Line::TypeLambdaParentMarker){
        }
        else if(lines[i]->type==Line::TypeLiteralParentMarker){
        }
        else if(lines[i]->type==Line::TypeClassDefParentMarker){
        }
        else if(lines[i]->type==Line::TypeMethodParentMarker){
            qDebug()<<"ClassDef parent marker...";
            if(lines[i]->getParent()!=NULL){
                HeapStore::recieveLambda(lines[i]->lexicalID);
            }
        }
        else{
            int e=0;
            while(e<lines[i]->getData().size()){
                int o=0;
                while(o<lines[i]->getData()[e]->getData().size()){
                    if(lines[i]->getData()[e]->getData()[o]->isOperator()==false){
                        qDebug()<<"Sending primitive:";
                        int index = HeapStore::recievePrimitive(lines[i]->getData()[e]->getData()[o]->getString());
                        qDebug()<<"Index:"<<QString::number(index);
                        if(index>=0){
                            //adjust existing token
                            lines[i]->getData()[e]->getData()[o]->setAsHeapAddress(index);
                            qDebug()<<"Set as heap address.";
                        }
                    }
                    o++;
                }
                e++;
            }
            e=0;
            int complexCounter=0;
           while(e<lines[i]->getData().size()){
               int o=0;
               while(o<lines[i]->getData()[e]->getData().size()){
                   if(lines[i]->getData()[e]->getData()[o]->isOperator()==false){
                       if(QString::compare(lines[i]->getData()[e]->getData()[o]->data,"lambda")==0 ||
                          QString::compare(lines[i]->getData()[e]->getData()[o]->data,"literal")==0 ||
                               QString::compare(lines[i]->getData()[e]->getData()[o]->data,"classdef")==0 ||
                               lines[i]->getData()[e]->isMethodSignature()==true){
                           qDebug().noquote()<<"\n\n\nn\nMethod Signature, lambda literal or classdef to be recieved by heap.\n\n\n\n\n";
                            if(complexCounter<=lines[i]->childs.size()){
                                if(lines[i]->getData()[e]->isMethodSignature()==true){
                                    qDebug()<<"Following line-block considered to be message signature:"<<lines[i]->getData()[e]->toString();
                                    qDebug()<<"Complex Pointer:"<<QString::number(complexCounter);
                                    HeapStore::recieveLambda(lines[i]->childs[complexCounter]->lexicalID);
                                }
                                else if(QString::compare(lines[i]->getData()[e]->getData()[o]->data,"lambda")==0){
                                    qDebug()<<"Receiving Lambda:"<<QString::number(complexCounter);
                                    HeapStore::recieveLambda(lines[i]->childs[complexCounter]->lexicalID);
                                }
                                else if(QString::compare(lines[i]->getData()[e]->getData()[o]->data,"literal")==0){
                                    qDebug()<<"Receiving Literal:"<<QString::number(complexCounter);
                                    HeapStore::recieveLiteral(lines[i]->childs[complexCounter]->lexicalID,lines[i]->childs[complexCounter]->childs);
                                }
                                else{
                                    //must be classdef
                                    qDebug()<<"Receiving Classdef:"<<QString::number(complexCounter);
                                    if(i<=lines.size() && complexCounter<lines[i]->childs.size()){
                                        HeapStore::recieveClassDef(lines[i]->childs[complexCounter]->lexicalID,lines[i]->childs[complexCounter]->childs);
                                    }
                                    qDebug()<<"Sent off the classdef";
                                }
                                qDebug()<<"Setting as complex pointer";
                                lines[i]->getData()[e]->getData()[o]->setAsComplexPointer(HeapStore::literals.size()-1);
                                qDebug()<<"Complex pointer set.";
                            }
                            complexCounter++;
                       }
                   }
                   o++;
               }
               e++;
           }
        }
        qDebug()<<"survive loop...";
        i++;
    }
    qDebug()<<"Finished loop.";
    i=0;
    while(i<lines.size()){
        qDebug()<<"Flushing children to loop";
        if(lines[i]->childs.size()>0){
            lines[i]->childs = LineStatic::flushToHeapStore(lines[i]->childs);
        }
        qDebug()<<"Flushed children to loop";
        i++;
    }
    return lines;
}

QVector<Line*> LineStatic::flattenControlFlows(QVector<Line*> lines){
    QVector<Line*> res;
    int i=0;
    while(i<lines.size()){
        if(lines[i]->type==Line::TypeConditionalParentMarker){
            res.append(lines[i]);
            int e=0;
            while(e<lines[i]->childs.size()){
                lines[i]->childs[e]->_flattenControlFlows();
                res.append(lines[i]->childs[e]);
                e++;
            }
            lines[i]->childs.clear();
        }
        else{
            lines[i]->_flattenControlFlows();
            res.append(lines[i]);
        }
        i++;
    }
    QVector<Line*> newLines;
    i=0;
    while(i<res.size()){
        if(
           (res[i]->getData().size()>0 && res[i]->getData().first()->getData().size()>0) &&
           (QString::compare(res[i]->getData().first()->getData().first()->getString(),"if")==0 ||
           QString::compare(res[i]->getData().first()->getData().first()->getString(),"else")==0 ||
           QString::compare(res[i]->getData().first()->getData().first()->getString(),"elseif")==0 ||
           QString::compare(res[i]->getData().first()->getData().first()->getString(),"while")==0)
        ){
            newLines.append(res[i]);
            int id = HeapStore::newGoToId();
            if(QString::compare(res[i]->getData().first()->getData().first()->getString(),"if")==0){
                newLines.append(new Line(Line::TypeGOTOMarker,id));
            }
            else if(QString::compare(res[i]->getData().first()->getData().first()->getString(),"while")==0){
                newLines.append(new Line(Line::TypeGOTOLoopMarker,id));
            }
            else{
                newLines.append(new Line(Line::TypeGOTOFCompMarker,id));
            }
            int e=1;
            while(e<res[i]->childs.size()){
                newLines.append(res[i]->childs[e]);
                e++;
            }
            newLines.append(new Line(Line::TypeGOTOMarker,id));
            res[i]->childs={};
        }
        else{
            newLines.append(res[i]);
        }
        i++;
    }
    res = newLines;
    return res;
}


QVector<QString> LineStatic::getIncludesURLS(QVector<Line*> lines){
    QVector<QString> r;
    int i=0;
    while(i<lines.size()){
        if(lines[i]->data.size()==1){
            if(lines[i]->data.first()->getData().size()==3){
                if(QString::compare(lines[i]->data.first()->getData().first()->getString(),"include")==0 &&
                   QString::compare(lines[i]->data.first()->getData().last()->getString(),";")==0){
                    r.append(lines[i]->data.first()->getData()[1]->getString());
                }
            }
        }
        i++;
    }
    return r;
}
