#include "line.h"

#include <errorregistry.h>

#include <QDebug>

#include "heapstore.h"

#include "linehelper.h"
#include "linestatic.h"

//First, helper methods for validating an expression...

//Constructors...

Line* Line::root;

Line::Line(int lineNumber, QString data){
    this->expectedIndent=0;
    this->lexicalID=0;
    this->data.append(new LineBlock(lineNumber,data));
    this->type = Line::TypeRegular;
    this->changed=true;
    this->parent = Line::root;
    this->explicitIndent = false;
}

Line::Line(QVector<Token*> data){
    this->expectedIndent=0;
    this->type=Line::TypeRegular;
    this->data = {new LineBlock(data)};
    this->parent = Line::root;
}

void Line::setExpectedIndentation(int newIndent){
    qDebug()<<"Setting explixit indentation to:"<<QString::number(newIndent)<<", from current indent "<<QString::number(this->getIndent());
    this->expectedIndent = newIndent;
    this->explicitIndent = true;
}

void Line::fixNumberOfParentMarkers(){

    qDebug()<<"Called fix number of parent markers";

    if(this->type==Line::TypeRegular){
        int i=0;
        int c=0;
        while(i<this->data.size()){
            int e=0;
            while(e<this->data[i]->getData().size()){
                QString str = this->data[i]->getData()[e]->getString();
                if(QString::compare(str,"lambda")==0){
                    if(this->childs.size()<(c+1)){
                        this->childs.append(new Line(Line::TypeLambdaParentMarker));
                        this->childs.last()->setParent(this);
                    }
                    c++;
                }
                else if(QString::compare(str,"literal")==0){
                    if(this->childs.size()<(c+1)){
                        this->childs.append(new Line(Line::TypeLiteralParentMarker));
                        this->childs.last()->setParent(this);
                    }
                    c++;
                }
                else if(QString::compare(str,"classdef")==0){
                    if(this->childs.size()<(c+1)){
                        this->childs.append(new Line(Line::TypeClassDefParentMarker));
                        this->childs.last()->setParent(this);
                    }
                    c++;
                }
                e++;
            }
            i++;
        }
        if(this->data.size()>0 && this->data.first()->getData().size()>0 && this->data.last()->getData().size()>0 && QString::compare(this->data.last()->getData().last()->getString(),":")==0 &&
                (
           QString::compare(this->data.first()->getData().first()->getString(),"if")==0 ||
                    QString::compare(this->data.first()->getData().first()->getString(),"elseif")==0 ||
                    QString::compare(this->data.first()->getData().first()->getString(),"else")==0 ||
                    QString::compare(this->data.first()->getData().first()->getString(),"while")==0 ||
                    QString::compare(this->data.first()->getData().first()->getString(),"for")==0
                    )
                ){
            if(this->childs.size()!=1){
                if(this->childs.size()==0){
                    qDebug()<<"appendin conditional marker on "<<this->toString(0);
                    this->childs.append(new Line(Line::TypeConditionalParentMarker));
                    this->childs.last()->setParent(this);
                    qDebug()<<"range error?";
                }
                else{
                    qDebug()<<"Error?";
                }
            }
            else{
                qDebug()<<"Failed child size test for appending conditional marker.";
            }
        }
        else{
            qDebug()<<"Not a conditional marker, for:"<<this->toString(0);
        }
        int e=0;
        while(e<this->childs.size()){
            qDebug()<<"Iteration...";
            if(this->childs[e]->getType()==Line::TypeLiteralParentMarker || this->childs[e]->getType()==Line::TypeClassDefParentMarker){
                //methodparentmarkers to be added here.
                qDebug()<<"Lambda,literal or classdef found, looking for method signatures";
                int o=0;
                int mpcs=0;
                while(o<this->childs[e]->childs.size()){
                    qDebug()<<"checking child...";
                    int u=0;
                    while(u<this->childs[e]->childs[o]->data.size()){
                        qDebug()<<"checking lineblock...";
                        i=0;
                        while(i<this->childs[e]->childs[o]->data[u]->getData().size()){
                            qDebug()<<"checking token stream..."<<this->childs[e]->childs[o]->data[u]->getData()[i]->getString();
                            if(QString::compare(this->childs[e]->childs[o]->data[u]->getData()[i]->getString(),"(")==0){
                                qDebug()<<"method signature found.";

                                if(this->childs[e]->childs[o]->childs.size()<(mpcs+1)){
                                    qDebug()<<"Method signature found.";
                                    this->childs[e]->childs[o]->setExpectedIndentation(this->childs[e]->childs[o]->getIndent()+1);
                                    this->childs[e]->childs[o]->childs.append(new Line(Line::TypeMethodParentMarker));
                                    //new added
                                    this->childs[e]->childs[o]->childs.last()->setParent(this);
                                    qDebug().noquote()<<"Method Appended:\n\n"<<this->childs[e]->childs[o]->toString(0);
                                }
                                else{
                                    //this added
                                    this->childs[e]->childs[o]->fixNumberOfParentMarkers();
                                }
                                mpcs++;
                            }
                            i++;
                        }
                        u++;
                    }
                    o++;
                }
            }
            else{
                qDebug()<<"elseing...";
                this->childs[e]->fixNumberOfParentMarkers();
            }
            e++;
        }
    }
    else{
        qDebug()<<"called on nonregular...";
        int e=0;
        while(e<this->childs.size()){
            this->childs[e]->fixNumberOfParentMarkers();
            e++;
        }
        e++;
    }
    qDebug()<<"PMs fixed.";
}

Line* Line::getLastLeaf(int indent){
    qDebug()<<"Getting last leaf...";
    if(this->getIndent()==indent){
        qDebug()<<"indentation is equal, returning this...";
        return this;
    }
    else if(this->expectedIndentation()==indent){
        qDebug()<<"indentation is equal to expected indentation...";
        if(this->childs.size()>0){
            qDebug()<<"has childs...";
            if(this->childs.last()->childs.size()>0){
                qDebug()<<"has childs...";
                return this->childs.last()->childs.last();
            }
            return this->childs.last();
        }
        return this;
    }
    else if(this->expectedIndentation()>indent){
        return this;
    }
    else{
        if(this->childs.size()>0){
            if(this->childs.last()->childs.size()>0){
                return this->childs.last()->childs.last()->getLastLeaf(indent);
            }
            return this;
        }
        return this;
    }
}

Line* Line::getParent(){
    return this->parent;
}

void Line::setParent(Line* parent){
    this->parent=parent;
}

Line::Line(Line* line1,Line* line2){

    this->expectedIndent=0;
    this->lexicalID=0;

    qDebug().noquote()<<"\nconstructing a new line from:\nLine 1:\n"+line1->toString(0)+"\nLine 2:\n"+line2->toString(0)+"\n";
    this->explicitIndent = false;

    this->parent = line1->getParent();
    this->changed=true;
    this->type = line1->type;
    this->childs = line1->childs;
    this->data = line1->data;
    qDebug().noquote()<<"\nFixing number of parent markers.\n";

    this->fixNumberOfParentMarkers();
    qDebug().noquote()<<"\nFixed number of parent markers.\n";
    qDebug().noquote()<<"\nDetecting LPM childs....\n";

    bool hasLPMChilds=false;
    int i=0;
    while(i<this->childs.size() && hasLPMChilds==false){
        if(this->childs[i]->getType()==Line::TypeLambdaParentMarker ||
            this->childs[i]->getType()==Line::TypeLiteralParentMarker ||
            this->childs[i]->getType()==Line::TypeClassDefParentMarker ||
            this->childs[i]->getType()==Line::TypeConditionalParentMarker){
            hasLPMChilds=true;
        }
        i++;
    }
    if(hasLPMChilds==true){
        qDebug().noquote()<<"\nDetected LPM childs....\n";

        if(this->childs.last()->childs.size()==0){
            //line2->setParent(this);
            line2->setParent(this->childs.last());
            this->childs.last()->childs.append(line2);
        }
        else{
            Line* lastLeaf = this->getLastLeaf(line2->getIndent());
            qDebug().noquote()<<"\nLastLeaf:\n";
            qDebug().noquote()<<lastLeaf->toString(0);
            if(lastLeaf->getType()==Line::TypeRegular){
                if(lastLeaf->isValidComplete()==true){
                    Line* lastLeafParent = lastLeaf->getParent();
                    qDebug().noquote()<<"Got parent:"<<lastLeafParent->toString(0);
                    line2->setParent(lastLeafParent);
                    if(lastLeafParent->childs.size()>0){
                        //lastLeafParent->childs.last()->childs.append(line2);
                        lastLeafParent->childs.append(line2);
                    }
                    else{
                        lastLeafParent->childs.append(line2);
                    }
                }
                else{
                    lastLeaf->data.append(line2->data[0]);
                }
            }
            else{
                line2->setParent(lastLeaf);
                lastLeaf->childs.append(line2);
            }
        }
    }
    else{
        qDebug().noquote()<<"\nLPM marker not detected.\n";
        this->data.append(line2->data[0]);
    }
    //what happens depends on the type of line...
    qDebug().noquote()<<"\nResulting in:\n"+this->toString(0);
}
Line::Line(int type){
    this->expectedIndent=0;
    this->lexicalID=0;
    this->type=type;
    this->explicitIndent = false;
    this->parent = Line::root;
}

//toString...
QString Line::toString(int indent){
    QString res="";
    QString tabs="";
    int e=0;
    while(e<indent){
        tabs+="\t";
        e++;
    }
    res+="\n\nLine of type:"+QString::number(this->type)+"lexical scope id:"+QString::number(this->lexicalID);
    if(this->type==Line::TypeRegular){
 //       res+="\n"+tabs+"Line Number,:"+QString::number(this->lineNumber);
   //     res+=",Indent:"+QString::number(this->indent)+"\n"+tabs;
        res+="\n"+tabs+"Printing line at indent "+QString::number(indent)+"\n"+tabs+"Expected Indentation:"+QString::number(this->expectedIndentation())+"\n"+tabs+",\n"+tabs+"private expected indent: "+QString::number(this->expectedIndent)+"\n"+tabs+"private explicit indent: "+QString::number(this->explicitIndent)+"\n,"+tabs+"LineBlocks:"+QString::number(this->data.size())+"\n"+tabs;
        int i=0;
        while(i<this->data.size()){
            res+="["+this->data[i]->toString()+"]";
            i++;
        }
        res+="\n"+tabs+"";
        res+="Childs:["+QString::number(this->childs.size())+"]\n";
        i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else if(this->type==Line::TypeLambdaParentMarker){
        res+="\n"+tabs+"[LAMBDAPARENT id:"+QString::number(this->lexicalID)+"]";
        res+="\n"+tabs+"Childs:["+QString::number(this->childs.size())+"]\n";
        int i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else if(this->type==Line::TypeMethodParentMarker){
        res+="\n"+tabs+"[METHODPARENT id:"+QString::number(this->lexicalID)+"]";
        res+="\n"+tabs+"Childs:["+QString::number(this->childs.size())+"]\n";
        int i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else if(this->type==Line::TypeConditionalParentMarker){
        res+="\n"+tabs+"[CONDITIONALPARENT]";
        res+="\n"+tabs+"Childs:["+QString::number(this->childs.size())+"]\n";
        int i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else if(this->type==Line::TypeClassDefParentMarker){
        res+="\n"+tabs+"[CLASSDEFPARENT id:"+QString::number(this->lexicalID)+"]";
        res+="\n"+tabs+"Childs:["+QString::number(this->childs.size())+"]\n";
        int i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else if(this->type==Line::TypeLiteralParentMarker){
        res+="\n"+tabs+"[LITERALPARENT id:"+QString::number(this->lexicalID)+"]";
        res+="\n"+tabs+"Childs:["+QString::number(this->childs.size())+"]\n";
        int i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else if(this->type==Line::TypeLexicalMarker){
        res+="\n"+tabs+"[LEXMARKER]\n";
        res+="\n"+tabs+"[{lexchild:"+QString::number(this->lexicalChildMarker)+",parent:"+QString::number(this->lexicalParentMarker)+"}]\n";
    }
    else if(this->type==Line::TypeGOTOMarker){
        res+="\n"+tabs+"[GOTOMARKER]\n";
        res+="\n"+tabs+"[id:"+QString::number(this->goToID)+"]\n";
    }
    else if(this->type==Line::TypeGOTOFCompMarker){
        res+="\n"+tabs+"[GOTO-FCMP-MARKER]\n";
        res+="\n"+tabs+"[id:"+QString::number(this->goToID)+"]\n";
    }
    else if(this->type==Line::TypeGOTOLoopMarker){
        res+="\n"+tabs+"[GOTO-LOOP-MARKER]\n";
        res+="\n"+tabs+"[id:"+QString::number(this->goToID)+"]\n";
    }
    else if(this->type==Line::TypeRoot){
        res+="\n"+tabs+"[ROOTMARKER]";
        res+="\n"+tabs+"Childs:["+QString::number(this->childs.size())+"]\n";
        int i=0;
        while(i<this->childs.size()){
            res+=""+tabs+"["+QString::number(i)+"]:["+this->childs[i]->toString(indent+1)+tabs+"]\n";
            i++;
        }
    }
    else{
        qDebug()<<"Error!";
    }
    return res;
}
//public getters and setters....

int Line::getIndent(){
    if(this->data.size()>0){
        return this->data.first()->getIndent();
    }
    else{
        return 0;
    }
}
int Line::getLineNumber(){
    if(this->data.size()>0){
        this->data.first()->getLineNumber();
    }
    else{
        return 0;
    }
}
int Line::getType(){
    return this->type;
}
bool Line::hasChanged(){
    return this->changed;
}
void Line::setHasChanged(bool change){
    this->changed=change;
}
int Line::expectedIndentation(){
    if(this->explicitIndent==true){
        qDebug()<<"Explicit indentation!";
        return this->expectedIndent;
    }
    else if(this->data.size()>0 && this->data.first()->getData().size()>0 && this->data.last()->getData().size()>0 && QString::compare(this->data.last()->getData().last()->getString(),":")==0 &&
       (QString::compare(this->data.first()->getData().first()->getString(),"if")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"else")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"elseif")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"while")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"for")==0)){
        return this->getIndent()+1;
    }
    else{
        int i=0;
        bool containsLambdaLiteralOrClassDef=false;
        while(i<this->data.size() && containsLambdaLiteralOrClassDef==false){
            int e=0;
            while(e<this->data[i]->getData().size() && containsLambdaLiteralOrClassDef==false){
                if(QString::compare(this->data[i]->getData()[e]->getString(),"lambda")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"literal")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"classdef")==0
                        ){
                    containsLambdaLiteralOrClassDef=true;
                }
                e++;
            }
            i++;
        }
        if(containsLambdaLiteralOrClassDef==true){
            return this->data.last()->getIndent()+1;
        }
        else{
            return this->data.last()->getIndent();
        }
    }
}

//instance method for validation

bool Line::isValidComplete(){
    if(QString::compare(this->data.last()->getData().last()->getString(),":")==0 &&
       (QString::compare(this->data.first()->getData().first()->getString(),"if")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"else")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"elseif")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"while")==0 ||
        QString::compare(this->data.first()->getData().first()->getString(),"for")==0)){
        return true;
    }
    if(QString::compare(this->data.last()->getData().last()->getString(),";")!=0){
        QVector<Token*> newData;
        int i=0;
        int e=0;
        if(QString::compare(this->data.first()->getData().first()->getString(),"let")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"fix")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"return")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"for")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"while")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"do")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"if")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"else")==0 ||
           QString::compare(this->data.first()->getData().first()->getString(),"elseif")==0){
            e++;
        }
        while(i<this->data.size()){
            while(e<this->data[i]->getData().size()){
                newData.append(this->data[i]->getData()[e]);
                e++;
            }
            e=0;
            i++;
        }
        if(newData.size()>0){
            if(QString::compare(newData.last()->getString(),":")==0 || QString::compare(newData.last()->getString(),";")==0){
                newData.removeLast();
            }
        }
        return LineHelper::isValidPostFix(newData);
    }
    else{
        return true;
    }
}

//instance method to validate down the tree

void Line::validateChilds(){
    if(this->childs.size()>0){
        if(this->childs.first()->getType()!=Line::TypeRegular){
            int i=0;
            while(i<this->childs.size()){
                if(this->childs[i]->getType()==Line::TypeClassDefParentMarker){
                    QVector<Line*> newChilds = LineStatic::validateAsClassPropertySignatures(this->getIndent(),this->childs[i]->childs);
                    this->childs[i]->childs = newChilds;
                }
                else if(this->childs[i]->getType()==Line::TypeLiteralParentMarker){
                    QVector<Line*> newChilds = LineStatic::validateAsLiteralPropertySignatures(this->getIndent(),this->childs[i]->childs);
                    this->childs[i]->childs = newChilds;
                }
                else{
                    QVector<Line*> newChilds = LineStatic::validateAsFirstOrderLines(this->getIndent(),this->childs[i]->childs);
                    this->childs[i]->childs = newChilds;
                }
                i++;
            }
        }
        else{

            int i=0;
            while(i<this->childs.size()){
                this->childs[i]->validateChilds();
                i++;
            }

        }
    }
}

int Line::getLexicalId(){
    return this->lexicalID;
}

//three validation methods depending on expected line types.
//basically property signatures are a special case.

QVector<ByteCodeInstruction*> Line::_compile(){
    QVector<ByteCodeInstruction*> fininsts;
    if(this->type==Line::TypeRegular){
        if(!(this->childs.size()>0 && this->childs.first()->type==Line::TypeMethodParentMarker)){
            QVector<ByteCodeInstruction*> insts;
            QVector<ByteCodeInstruction*> preinsts;
            int i=0;
            QVector<Token*> newData;
            while(i<this->data.size()){
                int e=0;
                while(e<this->data[i]->getData().size()){
                    if(!(i==0 && e==0 && (QString::compare(this->data[i]->getData()[e]->getString(),"let")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"fix")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"return")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"if")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"elseif")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"else")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"while")==0 ||
                        QString::compare(this->data[i]->getData()[e]->getString(),"for")==0
                      ))){
                        newData.append(this->data[i]->getData()[e]);
                    }
                    else if(
                            QString::compare(this->data[i]->getData()[e]->getString(),"fix")==0 ||
                            QString::compare(this->data[i]->getData()[e]->getString(),"let")==0
                            ){
                        if(e<this->data[i]->getData().size()-1){
                            if(QString::compare(this->data[i]->getData()[e]->getString(),"let")==0){
                                preinsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLET,this->data[i]->getData()[e+1],NULL,NULL));
                            }
                            if(QString::compare(this->data[i]->getData()[e]->getString(),"fix")==0){
                                preinsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeFIX,this->data[i]->getData()[e+1],NULL,NULL));
                            }
                        }
                    }
                    e++;

                }
                i++;
            }
            newData.removeLast();
            if(LineHelper::isValidPostFix(newData)==true){
                newData = LineHelper::convertToPostFix(newData);
                //graft newData into bytecode instructions,
                //but for now just spit it out...
                qDebug()<<"New Data from postfix arrangement:";
                int i=0;
                while(i<newData.size()){
                    qDebug()<<"["+newData[i]->getString()+"]";
                    i++;
                }
                //separate out expressions...
                qDebug()<<"Separating expressions...";
                int rpn=1;

                int rpnCount=0;

                while(newData.size()>0){
                    int e=0;
                    int highestPreviousRPN = 0;
                    while(e<newData.size()){
                        if(newData[e]->getType()==Token::TypeRPNMarker){
                            int rpnData = newData[e]->idata;
                            if(rpnData>highestPreviousRPN){
                                highestPreviousRPN = rpnData;
                            }
                        }
                        e++;
                    }
                    qDebug()<<"Highest previous RPN"<<highestPreviousRPN;
                    int i=0;
                    bool esc=false;
                    while(i<newData.size()-2 && esc==false){
                        if(newData[i+2]->isOperator()==true &&
                           newData[i+1]->isOperator()==false &&
                           newData[i]->isOperator()==false){
                            qDebug()<<"Pattern found.";
                            int lowestRPN;
                            if(newData[i]->getType()==Token::TypeRPNMarker && newData[i+1]->getType()==Token::TypeRPNMarker){
                                if(newData[i]->idata<newData[i+1]->idata){
                                    lowestRPN = newData[i]->idata;
                                }
                                else{
                                    lowestRPN = newData[i+1]->idata;
                                }
                            }
                            else if(newData[i+1]->getType()==Token::TypeRPNMarker){
                                lowestRPN = newData[i+1]->idata;
                            }
                            else if(newData[i]->getType()==Token::TypeRPNMarker){
                                lowestRPN = newData[i]->idata;
                            }
                            else{
                                lowestRPN = highestPreviousRPN+1;
                            }
                            qDebug()<<"Lowest RPN "<<lowestRPN;

                            insts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeRPN,newData[i],newData[i+1],newData[i+2]));
                            insts.last()->setRPNMarker(lowestRPN);
                            newData.remove(i);
                            newData.remove(i);
                            newData.remove(i);
                            newData.insert(i,new Token(Token::TypeRPNMarker,lowestRPN));
                            esc=true;
                            rpnCount = highestPreviousRPN;
                            if(rpnCount<lowestRPN){
                                rpnCount = lowestRPN;
                            }
                        }
                        i++;
                    }
                    if(newData.size()==1){
                        if(newData[0]->getType()!=Token::TypeRPNMarker){
                            insts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeRPN,newData.first(),NULL,NULL));
                            insts.last()->setRPNMarker(1);
                            newData={};
                        }
                        else{
                            newData={};
                        }
                    }
                    else if(newData.size()==2){
                        qDebug()<<"Size is 2";
                        int o=0;
                        while(o<newData.size()){
                            qDebug()<<o<<"::"<<newData[o]->getString();
                            o++;
                        }
                    }
                }
            }

            qDebug()<<"returning "+QString::number(insts.size())+" worth of instructions.";
            i=0;
            while(i<preinsts.size()){
                fininsts.append(preinsts[i]);
                i++;
            }
            qDebug()<<"returning "+QString::number(fininsts.size())+" worth of instructions.";
            i=0;
            while(i<insts.size()){
                fininsts.append(insts[i]);
                i++;
            }
            qDebug()<<"returning "+QString::number(fininsts.size())+" worth of instructions.";
   //     return fininsts;
        }
    }
    else{
        if(this->type==Line::TypeGOTOMarker){
            return {new ByteCodeInstruction(ByteCodeInstruction::TypeGOTOMarker,new Token(QString::number(this->goToID)),NULL,NULL)};
        }
        else if(this->type==Line::TypeGOTOFCompMarker){
            return {new ByteCodeInstruction(ByteCodeInstruction::TypeGOTOFCompMarker,new Token(QString::number(this->goToID)),NULL,NULL)};
        }
        else if(this->type==Line::TypeGOTOLoopMarker){
            return {new ByteCodeInstruction(ByteCodeInstruction::TypeGOTOLoopMarker,new Token(QString::number(this->goToID)),NULL,NULL)};
        }
    }
    if(this->childs.size()>0){
        if(this->childs.first()->type==Line::TypeRegular){
            QVector<ByteCodeInstruction*> newinsts=LineStatic::compile(this->childs);
            int e=0;
            while(e<newinsts.size()){
                fininsts.append(newinsts[e]);
                e++;
            }
        }
        else{
            int i=0;
            while(i<this->childs.size()){
                /*if(this->childs[i]->type==Line::TypeLambdaParentMarker || this->childs[i]->type==Line::TypeMethodParentMarker){
                    fininsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXStartMarker,new Token(QString::number(this->childs[i]->lexicalID)),NULL,NULL));
                }
                QVector<ByteCodeInstruction*> newinsts=LineStatic::compile(this->childs[i]->childs);
                QVector<ByteCodeInstruction*> expectedArguments = LineStatic::compileArguments(this,i);
                int e=0;
                while(e<expectedArguments.size()){
                    fininsts.append(expectedArguments[e]);
                    e++;
                }
                e=0;
                while(e<newinsts.size()){
                    fininsts.append(newinsts[e]);
                    e++;
                }
                if(this->childs[i]->type==Line::TypeLambdaParentMarker || this->childs[i]->type==Line::TypeMethodParentMarker){
                    fininsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXEndMarker,new Token(QString::number(this->childs[i]->lexicalID)),NULL,NULL));
                }*/

                if(this->childs[i]->type==Line::TypeLambdaParentMarker || this->childs[i]->type==Line::TypeMethodParentMarker){
                    fininsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXStartMarker,new Token(QString::number(this->childs[i]->lexicalID)),NULL,NULL));
                }
                QVector<ByteCodeInstruction*> newinsts=LineStatic::compile(this->childs[i]->childs);

                //here expected arguments are not compiled for method parents, as LineStatic::compileArguments checks for the presence of a lmbda
                //but this is a safeguard, so perhaps do that guarding here
                //i.e. only call if method or lambda parent

                QVector<ByteCodeInstruction*> expectedArguments;

                if(this->childs[i]->type==Line::TypeLambdaParentMarker){
                    expectedArguments = LineStatic::compileArguments(this,i,false);
                }
                else if(this->childs[i]->type==Line::TypeMethodParentMarker){
                //    fininsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXStartMarker,new Token(QString::number(this->childs[i]->lexicalID)),NULL,NULL));
                    expectedArguments = LineStatic::compileArguments(this,i,true);
                }

                //QVector<ByteCodeInstruction*> expectedArguments = LineStatic::compileArguments(this,i);
                int e=0;
                while(e<expectedArguments.size()){
                    fininsts.append(expectedArguments[e]);
                    e++;
                }
                e=0;
                while(e<newinsts.size()){
                    fininsts.append(newinsts[e]);
                    e++;
                }
                if(this->childs[i]->type==Line::TypeLambdaParentMarker || this->childs[i]->type==Line::TypeMethodParentMarker){
                    fininsts.append(new ByteCodeInstruction(ByteCodeInstruction::TypeLEXEndMarker,new Token(QString::number(this->childs[i]->lexicalID)),NULL,NULL));
                }

                i++;
            }
        }
    }
    if(this->data.size()>0){
        if(this->data.first()->getData().size()>0){
            if(QString::compare(this->data.first()->getData().first()->getString(),"return")==0){
                if(fininsts.size()>0){
                    fininsts.last()->type = ByteCodeInstruction::TypeReturnMarker;
                }
            }
        }
    }
//    finists->prepend(ByteCodeInstruction::TypeLineNumberSetType);
    fininsts.prepend(
         new ByteCodeInstruction(
             ByteCodeInstruction::TypeLineNumberSetType,
             new Token(Token::TypeLineNumberSetType,this->getLineNumber()),
             NULL,
             NULL
         )
    );
    return fininsts;
}

QVector<int> Line::getArgsForLambda(int lexicalID){
    int i=0;
    int r=0;
    while(i<this->childs.size()){
        qDebug()<<"getting args for lambda...";
      /*  if(this->childs[i]->lexicalID==lexicalID){
            r=i;
        }*/
        i++;
    }
    QVector<int> res;
/*  bool inArgs=false;
    int x=0;
    while(i<this->data.size()){
        int e=0;
        while(e<this->data[i]->getData().size()){
            if(QString::compare(this->data[i]->getData()[e]->getString(),"lambda")==0){
                x++;
                if(x==r){
                    inArgs = true;
                    e++;
                }
            }
            if(inArgs==true){
                if(QString::compare(this->data[i]->getData()[e]->getString(),")")==0){
                    return res;
                }
                else{
                    res.append(this->data[i]->getData()[e]->idata);
                }
            }
            e++;
        }
        i++;
    }*/
    return res;
}

QVector<Line*> Line::getChilds(){
    return this->childs;
}

void Line::setComplexPointer(Line* child,int complexIndex){
    int i=0;
    int x=0;
    qDebug()<<"Set Complex Pointer!"<<this->toString(0);
    while(i<this->childs.size()){
        qDebug()<<"Checking child...";
        if(this->childs[i]==child){
            qDebug()<<"Detected child!";
            int e=0;
            while(e<this->data.size()){
                qDebug()<<"checking data...";
                int o=0;
                while(o<this->data[e]->getData().size()){
                    qDebug()<<"checking data...";
                    if(QString::compare(this->data[e]->getData()[o]->getString(),"lambda")==0 ||
                            QString::compare(this->data[e]->getData()[o]->getString(),"literal")==0 ||
                            QString::compare(this->data[e]->getData()[o]->getString(),"classdef")==0){
                        x++;
                        if(x==i){
                            this->data[e]->getData()[o]->setAsComplexPointer(complexIndex);
                        }
                    }
                    o++;
                }
            }
            e++;
        }
        qDebug()<<"child has been checked...";
        i++;
    }
    qDebug()<<"Set pointer...";
}

void Line::setLambdaPointer(int complexIndex){
    qDebug()<<"Launching instance set lambda methd!"<<this->toString(0);
    int i=0;
    while(i<this->data.size()){
        int e=0;
        while(e<this->data[i]->getData().size()){
            if(QString::compare(this->data[i]->getData()[e]->getString(),"lambda")==0 ||
               QString::compare(this->data[i]->getData()[e]->getString(),"literal")==0 ||
               QString::compare(this->data[i]->getData()[e]->getString(),"classdef")==0){
                qDebug()<<"first comparison...";
                if(this->data[i]->getData()[e]->getType()!=Token::TypeComplexAddress){
                    qDebug()<<"second comparison...";
                    this->data[i]->getData()[e]->setAsComplexPointer(complexIndex);
                    qDebug()<<"end of control flow?";
                }
            }
            qDebug()<<"end inner loop...";
            e++;
        }
        qDebug()<<"end outer loop...";
        i++;
    }
    qDebug()<<"Set lambda...";
}

void Line::_labelLexicalScopes(){
    qDebug()<<"_Labelling lexical scope...";
    int i=0;
    while(i<this->childs.size()){
        if(this->childs[i]->type==Line::TypeClassDefParentMarker ||
                this->childs[i]->type==Line::TypeLambdaParentMarker ||
                this->childs[i]->type==Line::TypeLiteralParentMarker||
                this->childs[i]->type==Line::TypeMethodParentMarker){
            int id=HeapStore::getFreshLexicalId();
            this->childs[i]->setLexicalId(id);            

            Line* parent = this->getParent();

            //Line* parent = this->getParent();
/*
            bool esc=false;

            while(esc==false && parent->type!=Line::TypeRoot && parent->type!=Line::TypeClassDefParentMarker &&
                            parent->type!=Line::TypeLambdaParentMarker &&
                            parent->type!=Line::TypeLiteralParentMarker&&
                            parent->type!=Line::TypeMethodParentMarker){
                if(parent->getParent()==NULL){
                    esc=true;
                }
                else{
                    parent = parent->getParent();
                }
            }*/

/*            bool esc=false;

            while(esc==false && parent->type!=Line::TypeRoot &&
                  (parent->type==TypeRegular || parent->type==TypeConditionalParentMarker)/*
                  parent->type!=Line::TypeClassDefParentMarker &&
                                  parent->type!=Line::TypeLambdaParentMarker &&
                                  parent->type!=Line::TypeLiteralParentMarker&&
                                  parent->type!=Line::TypeMethodParentMarker*//*){
                qDebug()<<"Looking for parent for lexical rule..."<<parent->toString(0);
                if(parent->getParent()==NULL){
                    esc=true;
                }
                else{
                    parent = parent->getParent();
                }
            }*/

            while(parent->type==Line::TypeRegular || parent->type==Line::TypeConditionalParentMarker){
                parent = parent->getParent();
                qDebug().noquote()<<"reset lexical scope parent to : "<<parent->toString(0);
            }

            qDebug().noquote()<<"Found lexical scope marker to label... for: "<<this->toString(0)<<", with parent:"<<parent->toString(0);

            HeapStore::addLexicalRule(id,parent->lexicalID);
            qDebug()<<this->childs[i]->toString(0);
        }
        else{
//            qDebug()<<"Error";
        }
        i++;
    }

    i=0;
    while(i<this->childs.size()){
        this->childs[i]->_labelLexicalScopes();
        i++;
    }
    qDebug()<<"_Labelling lexical scope finished";
}

void Line::_flattenControlFlows(){
    this->childs = LineStatic::flattenControlFlows(this->childs);
}

QVector<Line*> Line::insertLexicalMarkers(){
}

Line::Line(int type,int marker,int parent){
    this->expectedIndent=0;
    this->lexicalID=0;
    this->type = type;
    this->lexicalChildMarker = marker;
    this->lexicalParentMarker = parent;
    this->parent = Line::root;
}

Line::Line(int type,int id){
    this->expectedIndent=0;
    this->type=type;
    this->goToID = id;
    this->parent = Line::root;
}

void Line::setLexicalId(int id){
    qDebug()<<"within/setting lexical id...";
    this->lexicalID = id;
    qDebug()<<"within/set lexical id...";
}

QVector<LineBlock*> Line::getData(){
    return this->data;
}
