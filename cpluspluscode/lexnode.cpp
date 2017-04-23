#include "lexnode.h"
#include "token2.h"
#include "preobject2.h"
#include "architecture.h"
#include "linehelper.h"

#include <QDebug>

int LexNode::freshLexicalId=-1;
int LexNode::freshConditionalId=-1;

int LexNode::getFreshLexicalId(){
    freshLexicalId++;
    return freshLexicalId;
}

int LexNode::getFreshConditionalId(){
    freshConditionalId++;
    return freshConditionalId;
}

LexNode::LexNode(QVector<bool> isCode, QVector<QString> byString){

    this->lexId = LexNode::getFreshLexicalId();

    qDebug()<<"Created lexnode with id:"<<this->lexId;

    int i=0;
    this->openBrackets = 0;
    this->closeBrackets = 0;
    this->tempChilds={};
    this->tempChildTypes={};

    this->_class=false;
    this->_literal=false;
    qDebug()<<"Constructing lex node...";

    while(i<isCode.size()){
        if(isCode[i]==true){
            auto _list = byString[i].split('\n');
            QStringList sub_list={};
            QStringList fin_list={};
            int i=0;
            while(i<_list.size()){
                sub_list = _list[i].split('\t');
                int e=0;
                while(e<sub_list.size()){
                    fin_list.append(sub_list[e]);
                    e++;
                }
                i++;
            }
            QStringList next_fin_list={};
            i=0;
            while(i<fin_list.size()){
                sub_list = fin_list[i].split(' ');
                int e=0;
                while(e<sub_list.size()){
                    next_fin_list.append(sub_list[e]);
                    e++;
                }
                i++;
            }
            i=0;
            while(i<next_fin_list.size()){
                if(next_fin_list[i].length()==0){
                    next_fin_list.removeAt(i);
                }
                i++;
            }
            QStringList last_fin_list = {};
            i=0;
            while(i<next_fin_list.size()){
                sub_list = next_fin_list[i].split(QRegExp("[^_a-zA-Z\\d\\s][.]"));
                int e=0;
                while(e<sub_list.size()){
                    last_fin_list.append(sub_list[e]);
                    e++;
                }
                i++;
            }
            i=0;
            while(i<last_fin_list.size()){
                this->appendCode(last_fin_list[i]);
                i++;
            }
        }
        else{
            this->appendString(byString[i]);
        }
        i++;
    }


    i=0;
    while(i<this->tokens.size()){
        if(QString::compare(this->tokens[i]->_string,"!")==0){
            if(QString::compare(this->tokens[i+1]->_string,"=")==0){
                this->tokens[i]->_string="!=";
                this->tokens.remove(i+1);
                i--;
            }
        }
        else if(QString::compare(this->tokens[i]->_string,"=")==0){
            if(QString::compare(this->tokens[i+1]->_string,"=")==0){
                this->tokens[i]->_string="==";
                this->tokens.remove(i+1);
                i--;
            }
        }
        else if(QString::compare(this->tokens[i]->_string,"<")==0){
            if(QString::compare(this->tokens[i+1]->_string,"=")==0){
                this->tokens[i]->_string="<=";
                this->tokens.remove(i+1);
                i--;
            }
        }
        else if(QString::compare(this->tokens[i]->_string,"+")==0){
            if(QString::compare(this->tokens[i+1]->_string,"+")==0){
                this->tokens[i]->_string="++";
                this->tokens.remove(i+1);
                i--;
            }
        }
        else if(QString::compare(this->tokens[i]->_string,"-")==0){
            if(QString::compare(this->tokens[i+1]->_string,"-")==0){
                this->tokens[i]->_string="++";
                this->tokens.remove(i+1);
                i--;
            }
        }
        else if(QString::compare(this->tokens[i]->_string,">")==0){
            if(QString::compare(this->tokens[i+1]->_string,"=")==0){
                this->tokens[i]->_string=">=";
                this->tokens.remove(i+1);
                i--;
            }
        }
        else if(QString::compare(this->tokens[i]->_string,"else")==0){
            if(QString::compare(this->tokens[i+1]->_string,"if")==0){
                this->tokens[i]->_string="elseif";
                this->tokens.remove(i+1);
                i--;
            }
        }
        i++;
    }


}

void LexNode::appendCode(QString _data){
    if(_data.length()>0){
        int o=0;
        QVector<char> operators = {'.','<','>',':',';','*','&','%','|','[','+','-','/',']',',','!','=','(',')','{','}'};
        QStringList _list = {_data};
        while(o<operators.size()){
            int i=0;
            QStringList _newList = {};
            while(i<_list.size()){
                auto _datlist = _list[i].split(operators[o]);
                int u=0;
                while(u<_datlist.size()){
                    _newList.append(_datlist[u]);
                    if(u!=_datlist.size()-1){
                        _newList.append(QString(operators[o]));
                    }
                    u++;
                }
                i++;
            }
            _list = _newList;
            o++;
        }
        o=0;
        while(o<_list.size()){
            if(_list[o].length()!=0){
                if(QString::compare(_list[o],"{")==0){
                    openBrackets++;
                }
                if(QString::compare(_list[o],"}")==0){
                    closeBrackets++;
                }
                if(closeBrackets==openBrackets){
                    //qDebug()<<"AppendingCode:"<<_list[o]<<"childs size:"<<tempChilds.size();
                    if(tempChilds.size()>0){
                       // qDebug()<<"Adding child list...";
                        this->childs.append(new LexNode(this->tempChildTypes,this->tempChilds));
                        this->tokens.append(new Token2(Token2::TypeChildMarker,this->childs.size()-1));
                        this->tempChilds={};
                        this->tempChildTypes={};
                    }
                    if(QString::compare(_list[o],"{")!=0 && QString::compare(_list[o],"}")!=0){
                        this->tokens.append(new Token2(Token2::TypeCode,_list[o]));
                    }
                }
                else{
                    if(QString::compare(_list[o],"{")!=0 && QString::compare(_list[o],"}")!=0){
                       // qDebug()<<"AppendingChild:"<<_list[o];
                        this->tempChilds.append(_list[o]);
                        this->tempChildTypes.append(true);
                    }
                    else{
                        if(this->tempChilds.size()>0){
                            this->tempChilds.append(_list[o]);
                            this->tempChildTypes.append(true);
                        }
                    }
                }
            }
            o++;
        }
    }
    int i=0;
    while(i<this->tokens.size()-2){
        if(this->tokens[i]->type==Token2::TypeCode && this->tokens[i+1]->type==Token2::TypeCode && this->tokens[i+2]->type==Token2::TypeCode){
            if(this->tokens[i]->isInteger()==true && QString::compare(this->tokens[i+1]->_string,".")==0 && this->tokens[i+2]->isInteger()==true){
                this->tokens[i]->_string=this->tokens[i]->_string+"."+this->tokens[i+2]->_string;
                this->tokens[i]->type = Token2::TypeCode;
                this->tokens.remove(i+1);
                this->tokens.remove(i+1);
            }
        }
        i++;
    }
}

void LexNode::appendString(QString _data){
    if(_data.length()>0){
        if(closeBrackets==openBrackets){
            this->tokens.append(new Token2(Token2::TypeString,_data));
        }
        else{
            this->tempChilds.append(_data);
            this->tempChildTypes.append(false);
        }
    }
}

QString LexNode::toString(int indent){
    QString res = "";
    int i=0;
    while(i<this->tokens.size()){
        if(this->tokens[i]->type==Token2::TypeString){
            int e=0;
            while(e<indent){
                res+="\t";
                e++;
            }
            res+="<token type='string'>"+this->tokens[i]->_string+"</token>\n";
        }
        else if(this->tokens[i]->type==Token2::TypeCode){
            int e=0;
            while(e<indent){
                res+="\t";
                e++;
            }
            res+="<token type='code'>"+this->tokens[i]->_string+"</token>\n";
        }
        else{
            int e=0;
            while(e<indent){
                res+="\t";
                e++;
            }
            int index = this->tokens[i]->data;
            res+="<token type='child' index='"+QString::number(index)+"'>\n"+this->childs[index]->toString(indent+1);
            e=0;
            while(e<indent){
               res+="\t";
               e++;
            }
            res+="</token>\n";
        }
        i++;
    }
    return res;
}

QVector<ByteCodeInstruction2 *> LexNode::toByteCode(){
    QVector<ByteCodeInstruction2*> res;
    int i=0;
    while(i<this->tokens.size()){
        i++;
    }
    return res;
}

void LexNode::setAsClass(){
    this->_class=true;
}
void LexNode::setAsLiteral(){
    this->_literal=true;
}

void LexNode::setAsAddress(int index){
    this->address=index;
}

void LexNode::prepend(LexNode* child){
    int i=child->data.size()-1;
    while(i>=0){
        this->data.prepend(child->data[i]);
        i--;
    }

    //this just added

    i=child->childs.size()-1;
    while(i>=0){
        this->childs.prepend(child->childs[i]);
        i--;
    }
}

void LexNode::compileToTokenStream(bool top){
    qDebug().noquote()<<"getting bytecode... for lexnode:"<<this->toString(0);
    PByteArray* res = new PByteArray();
    QVector<QVector<Token2*>> codes = {};
    QVector<Token2*> temp = {};
    QVector<Token2*> childPointers = {};

    //QVector<int> ifs;

    int i=0;
    while(i<this->tokens.size()){
        qDebug()<<"loop, on token:"<<this->tokens[i]->_string;
        if(this->tokens[i]->type==Token2::TypeChildMarker){
            qDebug()<<"child marker:"<<this->tokens[i]->_string;
            if(LineHelper::isValidPostFix2(temp)==true){
                temp = LineHelper::convertToPostFix2(temp);
                //graft newData into bytecode instructions,
                //but for now just spit it out...
                qDebug()<<"New Data from postfix arrangement(1):";
                int e=0;
                while(e<temp.size()){
                    qDebug()<<"["+temp[e]->_string+"]";
                    e++;
                }
                codes.push_back(temp);
                temp={};
                childPointers.append(this->tokens[i]);
            }
            else{
                qDebug()<<"invalid line:"<<this->tokens[i]->_string;
                childPointers.append(this->tokens[i]);
            }
        }
        else if(this->tokens[i]->type==Token2::TypeCode){
            qDebug()<<"code:"<<this->tokens[i]->_string;
            if(QString::compare(this->tokens[i]->_string,";")==0){
                qDebug()<<"line terminator:"<<this->tokens[i]->_string;
                if(LineHelper::isValidPostFix2(temp)==true){
                    qDebug()<<"New Data from prior arrangement(2):";
                    int i=0;
                    while(i<temp.size()){
                        qDebug()<<"["+temp[i]->_string+"]";
                        i++;
                    }
                    temp = LineHelper::convertToPostFix2(temp);
                    //graft newData into bytecode instructions,
                    //but for now just spit it out...
                    qDebug()<<"New Data from postfix arrangement(2):";
                    i=0;
                    while(i<temp.size()){
                        qDebug()<<"["+temp[i]->_string+"]";
                        i++;
                    }
                    codes.push_back(temp);
                }
                else{
                    qDebug()<<"Invalid line:";
                    int i=0;
                    while(i<temp.size()){
                        qDebug()<<"["+temp[i]->_string+"]";
                        i++;
                    }
                }
                temp={};
            }
            else{
                qDebug()<<"other:"<<this->tokens[i]->_string;

                if(QString::compare(this->tokens[i]->_string,"let")==0){
                    auto temp2 = {this->tokens[i],this->tokens[i+1]};
                    codes.push_back(temp2);
                }
                else if(QString::compare(this->tokens[i]->_string,"lambda")==0){
                    QVector<QString> args = {};
                    int e=i;
                    i++;i++;
                    while(i<this->tokens.size() && QString::compare(this->tokens[i]->_string,")")!=0){
                        if(QString::compare(this->tokens[i]->_string,",")!=0){
                            args.append(this->tokens[i]->_string);
                        }
                        i++;
                    }
                    this->tokens[e]->expectedArguments=args;
                    temp.append(this->tokens[e]);
                }
                else{
                    temp.append(this->tokens[i]);
                }
            }
        }
        else{
            qDebug()<<"string:"<<this->tokens[i]->_string;
            temp.append(this->tokens[i]);
        }
        i++;
    }
    if(temp.size()>0){
        temp = LineHelper::convertToPostFix2(temp);
        //graft newData into bytecode instructions,
        //but for now just spit it out...
        qDebug()<<"New Data from postfix arrangement(3):";
        int i=0;
        while(i<temp.size()){
            qDebug()<<"["+temp[i]->_string+"]";
            i++;
        }
        codes.push_back(temp);
    }
/*
    i=0;
    while(i<ifs.size()){
        codes[ifs[i]].prepend(new Token2(Token2::TypeOperator,"if"));
        i++;
    }*/

    qDebug()<<"CODE";
    i=0;
    int childCount=0;
    while(i<codes.size()){
        int e=0;
        while(e<codes[i].size()){
     //       qDebug()<<"["<<":"<<codes[i][e]->type<<codes[i][e]->_string<<"]";
            if(codes[i][e]->type==Token2::TypeCode && (QString::compare(codes[i][e]->_string,"lambda")==0 ||QString::compare(codes[i][e]->_string,"literal")==0 ||QString::compare(codes[i][e]->_string,"class")==0)){

                qDebug()<<"compile to token stream, inserting child pointer for lambda:"<<childCount;

                codes[i].insert(e+1,childPointers[childCount]);
                childCount++;
            }
            else if(codes[i][e]->type==Token2::TypeCode && (QString::compare(codes[i][e]->_string,"if")==0)){
                qDebug()<<"compile to token stream, inserting child pointer for if statement:"<<childCount;

                codes[i].append(childPointers[childCount]);
                childCount++;
            }
            else if(codes[i][e]->type==Token2::TypeCode && (QString::compare(codes[i][e]->_string,"while")==0)){
                qDebug()<<"compile to token stream, inserting child pointer for while statement:"<<childCount;

                codes[i].append(childPointers[childCount]);
                childCount++;
            }
            else if(codes[i][e]->type==Token2::TypeCode && (QString::compare(codes[i][e]->_string,"else")==0)){
                codes[i][e]->_string = "elseif";
                codes[i].insert(e+1,new Token2(Token2::TypeCode,"true"));
                qDebug()<<"compile to token stream, inserting child pointer for else statement:"<<childCount;
                codes[i].append(childPointers[childCount]);
                childCount++;
            }
            else if(codes[i][e]->type==Token2::TypeCode && (QString::compare(codes[i][e]->_string,"elseif")==0)){
                qDebug()<<"compile to token stream, inserting child pointer for else statement:"<<childCount;
                codes[i].append(childPointers[childCount]);
                childCount++;
            }
            e++;
        }
        i++;
       // qDebug()<<"[END]";
    }
    //return res;
    this->data = codes;


    qDebug()<<"LexNode End of CompileToTokenStream";
    i=0;
    while(i<this->data.size()){
        int e=0;
        while(e<this->data[i].size()){
            qDebug()<<"["<<":"<<this->data[i][e]->type<<this->data[i][e]->_string<<"]";
            e++;
        }
        i++;
        qDebug()<<"[END]";
    }

   // return codes;
}

QVector<Token2*> LexNode::encounteredKeys={};
QVector<Token2*> LexNode::encounteredStrings={};
QVector<Token2*> LexNode::encounteredIntegers={};

int LexNode::HighestRPN=0;

QVector<LexNode*> LexNode::LiteralStore={};

PreObject2* LexNode::toPreObject(){
    PreObject2* pre = new PreObject2(this);
    return pre;
}

PByteArray* LexNode::preHeapByteCode(bool top,PByteArray* res){

    //should be changed to iterating over instructions...

    if(top==true){
        res = new PByteArray();
        qDebug()<<"Getting preheap bytecode...";
 //       QVector<PreObject2*> preObjects = {};
        res->encodeRawIntegerAt(0,LexNode::HighestRPN);
        res->prepend(ByteCodeInstruction2::TypeRPNReserve);
        res->setByte(Architecture::RawIntegerSize+1,ByteCodeInstruction2::TypeHeapFlush);
        res->encodeRawIntegerAt(Architecture::RawIntegerSize+2,0);
   }

    QString lastKeyword="";

    QStringList encounteredFloats={};

    int i=0;


    while(i<this->childs.size()){
        res->encodeLexRuleAt(res->size(),this->lexId,this->childs[i]->lexId);
        PByteArray* sub = this->childs[i]->getLexicalRules();
        res->copyInto(res->size(),sub,sub->size());
        i++;
    }
    i=0;
    while(i<this->instructionSet.size()){

        if(instructionSet[i]->type==ByteCodeInstruction2::TypeReturnMarker ||instructionSet[i]->type==ByteCodeInstruction2::TypeJMP ||instructionSet[i]->type==ByteCodeInstruction2::TypeEARG || instructionSet[i]->type==ByteCodeInstruction2::TypeCOMP  || instructionSet[i]->type==ByteCodeInstruction2::TypeFCOMP || instructionSet[i]->type==ByteCodeInstruction2::TypeRPN || instructionSet[i]->type==ByteCodeInstruction2::TypeLET){
            int e=0;
            while(e<3){
                Token2* candidate;
                if(e==0){
                    candidate = instructionSet[i]->A;
                }else if(e==1){
                    candidate = instructionSet[i]->B;
                }else{
                    candidate = instructionSet[i]->C;
                }
                if(candidate!=NULL){
                    qDebug()<<"Extracting preheap bytecode from Token:"<<candidate->_string<<" on res of size:"<<res->size();
                    if(candidate->type==Token2::TypeString){
                        qDebug()<<"Setting address for preheap bytecode:"<<res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4);
                        int e2=0;
                        bool found=false;
                        while(e2<encounteredStrings.size() && found==false){
                            if(QString::compare(encounteredStrings[e2]->_string,candidate->_string)==0){
                                found=true;
            //                        e=res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2);//wrong
                                e2=encounteredStrings[e2]->address;
                            }
                            else{
                                e2++;
                            }
                        }
                        if(found==false){
                            candidate->setAsAddress(res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2));
                            res->encodeAsStringAt(res->size(),candidate->_string,true);
                            encounteredStrings.append(candidate);
                        }
                        else{
                            candidate->setAsAddress(e2);
                        }
                    }
                    else if(candidate->type==Token2::TypeCode){
                        qDebug()<<"is code";
                        if(QString::compare(candidate->_string,"lambda")==0){
                           // int index = candidate->data;
                            candidate->setAsAddress(res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2));
                            //qDebug()<<"Setting address of lambda..."<<candidate->_string<<","<<candidate->address<<" inst pointer"<<i<<",lexid:"<<this->childs[index]->lexId<<", child index:"<<index;
                            res->encodeLambdaAt(res->size(),candidate->lexId,(i+1));
                        }
                        if(QString::compare(candidate->_string,"literal")==0){
                           // int index = candidate->data;
                            candidate->setAsAddress(res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2));
                            //qDebug()<<"Setting address of lambda..."<<candidate->_string<<","<<candidate->address<<" inst pointer"<<i<<",lexid:"<<this->childs[index]->lexId<<", child index:"<<index;
                            PreObject2* pre = LexNode::LiteralStore[candidate->data]->toPreObject();
                            PByteArray* sub = pre->compile();
                            res->copyInto(res->size(),sub,sub->size());
                            //res->encodeLambdaAt(res->size(),candidate->lexId,(i+1));
                        }
                        else if(!(candidate->isKeyWord()) && !(candidate->isOperator())){
                            if(QString::compare(candidate->_string,"(")!=0 &&
                                    QString::compare(candidate->_string,")")!=0 &&
                                    QString::compare(candidate->_string,";")!=0){
                                candidate->setAsAddress(res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2));

                                qDebug()<<"Setting token as...";

                                if(candidate->isInteger()==true){
                                    qDebug()<<"Encoding raw integer"<<candidate->toInteger();


//                                    QStringList encounteredIntegers={};

                                    qDebug()<<"Setting address for preheap bytecode:"<<res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4);
                                    int e2=0;
                                    bool found=false;
                                    while(e2<encounteredIntegers.size() && found==false){
                                        if(encounteredIntegers[e2]->toInteger()==candidate->toInteger()){
                                            found=true;
                        //                        e=res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2);//wrong
                                            e2=encounteredIntegers[e2]->address;
                                        }
                                        else{
                                            e2++;
                                        }
                                    }
                                    if(found==false){
                                        candidate->setAsAddress(res->size()+Architecture::RPNAddress+(LexNode::HighestRPN*4)-((Architecture::RawIntegerSize*2)+2));
                                        res->append(ByteCodeInstruction2::TypeConstIntegerType);
                                        res->encodeRawIntegerAt(res->size(),candidate->toInteger());
                                        encounteredIntegers.append(candidate);
                                    }
                                    else{
                                        candidate->setAsAddress(e2);
                                    }
                                }
                                else if(candidate->isFloat()==true){
                                    qDebug()<<"Encoded raw float";
                                    res->append(ByteCodeInstruction2::TypeConstFloatType);
                                    res->encodeRawFloatAt(res->size(),candidate->toFloat());
                                }
                                else{
                                    if(QString::compare(candidate->_string,"native")==0){
                                        qDebug()<<"Encoded native...";
                                        candidate->setAsAddress(Architecture::NativeLocation);
                                    }
                                    else if(QString::compare(candidate->_string,"null")==0){
                                        qDebug()<<"Encoded native...";
                                        candidate->setAsAddress(Architecture::NULLLocation);
                                    }
                                    else if(QString::compare(candidate->_string,"unset")==0){
                                        qDebug()<<"Encoded native...";
                                        candidate->setAsAddress(Architecture::UnsetLocation);
                                    }
                                    else if(QString::compare(candidate->_string,"true")==0){
                                        qDebug()<<"Encoded native...";
                                        candidate->setAsAddress(Architecture::TrueLocation);
                                    }
                                    else if(QString::compare(candidate->_string,"false")==0){
                                        qDebug()<<"Encoded native...";
                                        candidate->setAsAddress(Architecture::FalseLocation);
                                    }
                                    else{
                                        qDebug()<<"Encoded key ("<<candidate->_string<<")at...";

                                        int e2=0;
                                        bool found=false;
                                        while(e2<encounteredKeys.size() && found==false){
                                            qDebug()<<e2;
                                            if(QString::compare(encounteredKeys[e2]->_string,candidate->_string)==0){
                                                found=true;
                                                e2=encounteredKeys[e2]->address;
                                            }
                                            else{
                                                e2++;
                                            }
                                        }
                                        if(found==false){
                                            res->encodeAsKeyAt(res->size(),candidate->_string);
                                            encounteredKeys.append(candidate);
                                        }
                                        else{
                                            candidate->setAsAddress(e2);
                                        }

                                        qDebug()<<"finished encoding key ("<<candidate->_string<<")at...";


                                    }
                                }
                            }
                        }
                        qDebug()<<"end of else if";
                    }
                }
                e++;
            }
            qDebug()<<"end of loop:"<<e;
        }
        qDebug()<<"end of loop:"<<i;
        i++;
    }

    return res;

}

PByteArray* LexNode::getLexicalRules(){
    PByteArray* res = new PByteArray();
    int i=0;
    while(i<this->childs.size()){
        res->encodeLexRuleAt(res->size(),this->lexId,this->childs[i]->lexId);
        PByteArray* sub = this->childs[i]->getLexicalRules();
        res->copyInto(res->size(),sub,sub->size());
        i++;
    }
    return res;
}

QVector<QVector<Token2*>> LexNode::reorderTokenStream(bool isLambda,bool isIfStatement,bool isWhileStatement,bool isLiteralStatement,int &lexicalId,QVector<QString> args){
    if(!isLiteralStatement){
        int i=0;
        QVector<QVector<Token2*> > preinsts={};
        QVector<QVector<Token2*> > newData = {{}};

        qDebug()<<"getting reordered tokens for node:"<<this->toString(0)<<", lambda:"<<isLambda;

        lexicalId = this->lexId;

        int lastChildIndex=0;

        while(i<this->data.size()){
            qDebug()<<"outer loop...";
            int e=0;
            QVector<QVector<Token2*>> subExpressions = {{}};
            while(e<this->data[i].size()){

               qDebug()<<"inner loop...on token:"<<this->data[i][e]->_string;
                if(this->data[i][e]->type!=Token2::TypeChildMarker && !(e==0 && (QString::compare(this->data[i][e]->_string,"let")==0 ||
                    QString::compare(this->data[i][e]->_string,"fix")==0 ||
                    QString::compare(this->data[i][e]->_string,"return")==0 ||
                    QString::compare(this->data[i][e]->_string,"if")==0 ||
                    QString::compare(this->data[i][e]->_string,"elseif")==0 ||
                    QString::compare(this->data[i][e]->_string,"else")==0 ||
                    QString::compare(this->data[i][e]->_string,"while")==0 ||
                    QString::compare(this->data[i][e]->_string,"for")==0
                  ))){
                    qDebug()<<"adding this->data["<<i<<"]["<<e<<"]("<<this->data[i][e]->_string<<")";
                    newData.last().append(this->data[i][e]);
                }
                else if(this->data[i][e]->type==Token2::TypeChildMarker){
                    qDebug()<<"lossing child marker in rpnreordering... this->data["<<i<<"]["<<e<<"]("<<this->data[i][e]->_string<<")";

                    qDebug().noquote()<<"getting instruction set from:\n";//<<this->childs[this->data[i][e]->data]->toString(0);

                    lastChildIndex = this->data[i][e]->data;
                    this->childs[this->data[i][e]->data]->compileToTokenStream(false);

                    int lambdaAddressE = newData.last().size()-1;
                    int lambdaAddressI = newData.size()-1;
                    int lexId;
                    bool f=false;
                    bool isLam=false;
                    bool isIf=false;
                    bool isWhile=false;
                    bool isLit=false;
                    while(lambdaAddressI>=0 && f==false){

                        lambdaAddressE = newData[lambdaAddressI].size()-1;
                        while(f==false && lambdaAddressE>=0){
                            auto candidate = newData[lambdaAddressI][lambdaAddressE];

                            qDebug()<<"Checking candidate:"<<candidate->_string<<" of parentage.";

                            if(candidate->type==Token2::TypeCode || candidate->type==Token2::TypeOperator){
                                if(QString::compare(candidate->_string,"lambda")==0 || QString::compare(candidate->_string,"if")==0 || QString::compare(candidate->_string,"elseif")==0 || QString::compare(candidate->_string,"while")==0){
                                    if(QString::compare(candidate->_string,"lambda")==0){
                                        qDebug()<<"parent is lam...";
                                        isLam=true;
                                    }
                                    else if(QString::compare(candidate->_string,"while")==0){
                                        isWhile = true;
                                    }
                                    else{
                                        qDebug()<<"parent is if...";
                                        isIf=true;
                                    }
                                    f=true;
                                    lambdaAddressE++;
                                    lambdaAddressI++;
                                }
                                else if(QString::compare(candidate->_string,"literal")==0){
                                    f=true;
                                    isLit=true;
                                    qDebug()<<"literal...";
                                    lambdaAddressE++;
                                    lambdaAddressI++;
                                }
                            }
                            lambdaAddressE--;
                        }
                        lambdaAddressI--;
                    }

                    subExpressions = this->childs[this->data[i][e]->data]->reorderTokenStream(isLam,isIf,isWhile,isLit,lexId,newData[lambdaAddressI][lambdaAddressE]->expectedArguments);

                    qDebug()<<"got sub expressions...";

                    newData[lambdaAddressI][lambdaAddressE]->lexId=lexId;

                    if(isLit==true){
                        LexNode::LiteralStore.append(this->childs[this->data[i][e]->data]);
                        newData[lambdaAddressI][lambdaAddressE]->data = LexNode::LiteralStore.size()-1;
                    }
                    qDebug()<<"inserting children...("<<subExpressions.size()<<")";
                    int u=0;
                    while(u<subExpressions.size()){
                        int k=0;
                        while(k<subExpressions[u].size()){
                            qDebug()<<"child["<<u<<"]:"<<subExpressions[u][k]->_string;
                            k++;
                        }
                        qDebug()<<">>>>>end";
                        u++;
                    }
                }
                else if(
                        QString::compare(this->data[i][e]->_string,"fix")==0 ||
                        QString::compare(this->data[i][e]->_string,"let")==0
                        ){

                    qDebug()<<"Compiling let statement!";

                    if(e<this->data[i].size()-1){
                        if(QString::compare(this->data[i][e]->_string,"let")==0){
                            //this->preinsts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeLET,this->data[i][e+1],NULL,NULL));

                            qDebug()<<"Compiling let statement!";
                            QVector<Token2*> sub = {};
                            sub.append(new Token2(Token2::TypeCode,"let"));
                            sub.append(new Token2(Token2::TypeCode,this->data[i][e+1]->_string));

                            preinsts.append(sub);
                        }
                        if(QString::compare(this->data[i][e]->_string,"fix")==0){
                            QVector<Token2*> sub = {};
                            sub.append(new Token2(Token2::TypeCode,"fix"));
                            sub.append(new Token2(Token2::TypeCode,this->data[i][e+1]->_string));

                            preinsts.append(sub);
                        }
                        e = this->data[i].size()-1;
                    }
                }
                else if(QString::compare(this->data[i][e]->_string,"if")==0){
                    newData.append({this->data[i][e]});
                }
                else if(QString::compare(this->data[i][e]->_string,"return")==0){
                    newData.append({this->data[i][e]});
                }
                else if(QString::compare(this->data[i][e]->_string,"while")==0){
                    newData.append({this->data[i][e]});
                }
                else if(QString::compare(this->data[i][e]->_string,"elseif")==0){
                    newData.append({this->data[i][e]});
                }
                e++;
            }
            if(subExpressions.size()>0 && this->childs.size()>0){

                qDebug()<<"Compiling sub expressions, for last child index";
                newData.append(subExpressions);
                subExpressions={{}};

            }
            newData.append(QVector<Token2*>());
            i++;
        }

        i=0;
        while(i<preinsts.size()){

            qDebug()<<"Apending preinst:"<<preinsts[i].size();

            newData.prepend(preinsts[i]);
            i++;
        }

        if(isLambda==true){
            QVector<QVector<Token2*> > preArgs = {};

            int i=0;
            while(i<args.size()){
                preArgs.append({new Token2(Token2::TypeOperator,"EARG"),new Token2(Token2::TypeCode,args[i])});
                i++;
            }
            i=preArgs.size()-1;
            while(i>=0){
                newData.prepend(preArgs[i]);
                i--;
            }

            QVector<Token2*> pre = {};
            pre.append(new Token2(Token2::TypeOperator,"LEX-START",this->lexId));
            newData.prepend(pre);
          //  qDebug()<<"Compiling lexical start marker:";


            QVector<Token2*> post = {};
            post.append(new Token2(Token2::TypeOperator,"LEX-END",this->lexId));
            newData.append(post);
        }
        else if(isIfStatement==true){

            qDebug()<<"Compiling if statement....";

            int id = getFreshConditionalId();
            QVector<Token2*> pre = {};
            pre.append(new Token2(Token2::TypeOperator,"JMP",id));
            newData.prepend(pre);
            QVector<Token2*> post = {};
            post.append(new Token2(Token2::TypeOperator,"END-IF",id));
            newData.append(post);
        }
        else if(isWhileStatement==true){

            qDebug()<<"Compiling if statement....";

            int id = getFreshConditionalId();
            QVector<Token2*> pre = {};
            pre.append(new Token2(Token2::TypeOperator,"JMP",id));
            newData.prepend(pre);
            QVector<Token2*> post = {};
            post.append(new Token2(Token2::TypeOperator,"BJMP",id));
            newData.append(post);
        }
        else if(isLiteralStatement==true){
            qDebug()<<"Compiling object...";
        }
    /*    else if(isIfStatement==true){

            qDebug()<<"Compiling if statement....";

            int id = getFreshConditionalId();
            QVector<Token2*> pre = {};
            pre.append(new Token2(Token2::TypeOperator,"JMP",id));
            newData.prepend(pre);
            QVector<Token2*> post = {};
            post.append(new Token2(Token2::TypeOperator,"END-IF",id));
            newData.append(post);
        }*/

        i=0;
        while(i<newData.size()){
            if(newData[i].size()==0){
                newData.remove(i);
                i--;
            }
            i++;
        }

        this->data = newData;//added

        qDebug()<<"Reordered TokenStream:";

        i=0;
        while(i<this->data.size()){
            int e=0;
            while(e<this->data[i].size()){
                qDebug()<<this->data[i][e]->_string;
                e++;
            }
            qDebug()<<">>END";
            i++;
        }

        return newData;
    }
    else{
        QVector<QVector<Token2*>> insts;
        int i=0;
        while(i<this->childs.size()){
            QVector<QVector<Token2*>> res = this->childs[i]->reorderTokenStream(true,false,false,false,lexicalId,{});
            int e=0;
            while(e<res.size()){
                insts.append(res[e]);
                e++;
            }
        }
        this->data = insts;
        return this->data;
    }
}

QVector<ByteCodeInstruction2 *> LexNode::precompile(bool top){

    //this->compileToTokenStream(false);

    qDebug()<<"precompilation...";
    QVector<ByteCodeInstruction2*> fininsts;

    QVector<ByteCodeInstruction2*> insts;
    QVector<ByteCodeInstruction2*> preinsts;
    int i=0;
    QVector<QVector<Token2*> > newData = this->data;//this->reorderTokenStream(false);

    i=0;
    while(i<newData.size()){
        if(newData[i].size()==0){
            newData.remove(i);
            i--;
        }
        i++;
    }
    i=0;

    qDebug()<<"precompilation end of first nested loop...";

    //separate out expressions...
    qDebug()<<"Separating expressions...";
    int rpn=1;

    int rpnCount=0;

    bool done=false;

    QVector<int> ifs={};
    QVector<int> elseifs={};
    QVector<int> whiles={};
    QVector<int> returns={};
    int o=0;
    while(o<newData.size()){
        qDebug()<<"NEWDATA enry:";
        int p=0;
        while(p<newData[o].size()){
            qDebug()<<newData[o][p]->_string;
            p++;
        }
        qDebug()<<">>end";
        if(newData[o].size()==1 && newData[o][0]->type==Token2::TypeOperator && QString::compare(newData[o][0]->_string,"LEX-START")==0){
                qDebug()<<"Detected lex - start...";
            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeLEXStartMarker,newData[o][0],NULL,NULL));
        }
        else if(newData[o].size()==1 && newData[o][0]->type==Token2::TypeOperator && QString::compare(newData[o][0]->_string,"LEX-END")==0){
                qDebug()<<"Detected lex - end...";
            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeLEXEndMarker,newData[o][0],NULL,NULL));
        }
        else if(newData[o].size()==2 && newData[o][0]->type==Token2::TypeCode && QString::compare(newData[o][0]->_string,"let")==0){
                qDebug()<<"Detected let...";
            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeLET,newData[o][1],NULL,NULL));
        }
        else if(newData[o].size()==2 && newData[o][0]->type==Token2::TypeOperator && QString::compare(newData[o][0]->_string,"EARG")==0){
            qDebug()<<"Detected ARG:..."<<newData[o][1]->_string;
            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeEARG,newData[o][1],NULL,NULL));
        }
        else if(newData[o][0]->type==Token2::TypeOperator && QString::compare(newData[o][0]->_string,"JMP")==0){
            qDebug()<<"Detected JUMP:..."<<newData[o][0]->_string;
            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeJMP,newData[o][0],NULL,NULL));

            qDebug()<<"Instruction set after adding jump:";


            int p2=0;
            while(p2<insts.size()){
                qDebug()<<insts[p2]->toString(0);
                p2++;
            }
        }
        else if(newData[o][0]->type==Token2::TypeOperator && QString::compare(newData[o][0]->_string,"BJMP")==0){
            qDebug()<<"Detected BJUMP:..."<<newData[o][0]->_string;
            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeBackJMP,newData[o][0],NULL,NULL));

            qDebug()<<"Instruction set after adding back-jump:";


            int p2=0;
            while(p2<insts.size()){
                qDebug()<<insts[p2]->toString(0);
                p2++;
            }
        }
        else if(newData[o].size()==1 && newData[o][0]->type==Token2::TypeOperator && QString::compare(newData[o][0]->_string,"END-IF")==0){
            qDebug()<<"Detected End-if:..."<<newData[o][0]->_string;
            //insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeEndIfMarker,newData[o][0],NULL,NULL));
            insts.last()->isEndOfIf=true;
            insts.last()->conditionalId=newData[o][0]->data;
        }
        else if(o<newData.size()){


            if(newData[o][0]->type==Token2::TypeCode && QString::compare(newData[o][0]->_string,"if")==0){
                qDebug()<<"Detected if:..."<<newData[o][0]->_string;
                ifs.append(insts.size() + (((newData[o].size()-1)-1)/2)-1);
                qDebug()<<"set if at:"<<insts.size();
                // + ((newData[o].size()-1)-1)/2);
                qDebug()<<"tho the formula says:"<<insts.size() + (((newData[o].size()-1)-1)/2)-1;
                newData[o].remove(0);
            }
            else if(newData[o][0]->type==Token2::TypeCode && QString::compare(newData[o][0]->_string,"elseif")==0){
                qDebug()<<"Detected else-if:..."<<newData[o][0]->_string<<", appending:"<<insts.size();
                elseifs.append(insts.size() + (((newData[o].size()-1)-1)/2)-1);
                newData[o].remove(0);
            }
            else if(newData[o][0]->type==Token2::TypeCode && QString::compare(newData[o][0]->_string,"return")==0){
                qDebug()<<"Detected else-if:..."<<newData[o][0]->_string<<", appending:"<<insts.size();
                if(newData[o].size()<=3){
                    returns.append(insts.size());
                }
                else{
                    returns.append(insts.size() + (((newData[o].size()-1)-1)/2)-1);
                }
                    newData[o].remove(0);
            }
            else if(newData[o][0]->type==Token2::TypeCode && QString::compare(newData[o][0]->_string,"while")==0){
                qDebug()<<"Detected while:..."<<newData[o][0]->_string<<", appending:"<<insts.size();
                whiles.append(insts.size() + (((newData[o].size()-1)-1)/2)-1);
                newData[o].remove(0);
            }
            if(newData[o].size()>1){
                while(newData[o].size()>0 && done==false){
                    qDebug()<<"Repurposing....";
                    int oX = 0;
                    while(oX<newData[o].size()){
                        qDebug()<<newData[o][oX]->_string;
                        oX++;
                    }
                    qDebug()<<">end";

                    int e=0;
                    int highestPreviousRPN = 0;
               //     qDebug()<<"Looping on data of size:"<<newData[o].size();
                    while(e<newData[o].size()){
                      //  qDebug()<<"Looping on data of size:"<<newData[o][e]->_string<<",data:"<<newData[o][e]->data;
                        if(newData[o][e]->type==Token2::TypeOperator && QString::compare(newData[o][e]->_string,"RPN")==0){
                            int rpnData = newData[o][e]->data;
                            if(rpnData>highestPreviousRPN){
                                highestPreviousRPN = rpnData;
                            }
                        }
                        e++;
                    }
                  //  qDebug()<<"Highest previous RPN"<<highestPreviousRPN;
                    int i=0;
                    bool esc=false;
                    while(i<newData[o].size()-2 && esc==false){
                        if(newData[o][i+2]->isOperator()==true &&
                           newData[o][i+1]->isOperator()==false &&
                           newData[o][i]->isOperator()==false){
                    //        qDebug()<<"Pattern found.";
                            int lowestRPN;
                            if(newData[o][i]->type==Token2::TypeOperator && QString::compare(newData[o][i]->_string,"RPN")==0 && newData[o][i+1]->type==Token2::TypeOperator && QString::compare(newData[o][i+1]->_string,"RPN")==0){
                                if(newData[o][i]->data<newData[o][i+1]->data){
                                    lowestRPN = newData[o][i]->data;
                                }
                                else{
                                    lowestRPN = newData[o][i+1]->data;
                                }
                            }
                            else if(newData[o][i+1]->type==Token2::TypeOperator && QString::compare(newData[o][i+1]->_string,"RPN")==0){
                                lowestRPN = newData[o][i+1]->data;
                            }
                            else if(newData[o][i]->type==Token2::TypeOperator && QString::compare(newData[o][i]->_string,"RPN")==0){
                                lowestRPN = newData[o][i]->data;
                            }
                            else{
                                lowestRPN = highestPreviousRPN+1;
                            }
                            //qDebug()<<"Lowest RPN "<<lowestRPN;

                            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeRPN,newData[o][i],newData[o][i+1],newData[o][i+2]));
                            insts.last()->setRPNMarker(lowestRPN);
                            newData[o].remove(i);
                            newData[o].remove(i);
                            newData[o].remove(i);
    //                            newData.insert(i,new Token2(Token2::TypeRPNMarker,lowestRPN));
                            newData[o].insert(i,new Token2(Token2::TypeOperator,"RPN",lowestRPN));
                            esc=true;
                            rpnCount = highestPreviousRPN;
                            if(rpnCount<lowestRPN){
                                rpnCount = lowestRPN;
                            }
                        }
                        i++;
                    }
                    if(newData[o].size()==1){
                        if(newData[o][0]->type!=Token2::TypeOperator && QString::compare(newData[o][0]->_string,"RPN")==0){
                            insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeRPN,newData[o].first(),NULL,NULL));
                            insts.last()->setRPNMarker(1);
                            newData[o]={};
                        }
                        else{
                            newData[o]={};
                      //      done=true;
                        }
                    }
                    else if(newData[o].size()==2){
                      //  qDebug()<<"Size is 2";
                        int o2=0;
                        while(o2<newData[o].size()){
                        //    qDebug()<<o2<<"::"<<newData[o][o2]->_string;
                            o2++;
                        }
                        insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeRPN,newData[o].first(),newData[o][1],NULL));
                        if(newData[o].first()->data<newData[o][1]->data){
                            insts.last()->idata=newData[o].first()->data;
                        }
                        else{
                            insts.last()->idata=newData[o].last()->data;
                        }
                        newData[o]={};
                    }
                    else if(esc==false){
                        done=true;
                    }
                }
            }
            else{
                insts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeRPN,newData[o].first(),NULL,NULL));
            }
            qDebug()<<"End of iteration, instruction set:";
            int o2 = 0;
            while(o2<insts.size()){
                qDebug()<<"inst["<<o2<<"]"<<insts[o2]->toString(0);
                o2++;
            }
            qDebug()<<"end";
        }
        o++;
    }

    qDebug()<<"setting ifs...";

    if(rpnCount>HighestRPN){
        HighestRPN=rpnCount;
    }

    i=0;
    while(i<ifs.size()){
        insts[ifs[i]]->type = ByteCodeInstruction2::TypeCOMP;
        i++;
    }

    qDebug()<<"setting whiles...";

    i=0;
    while(i<whiles.size()){
        insts[whiles[i]]->type = ByteCodeInstruction2::TypeCOMP;
        i++;
    }

    qDebug()<<"setting elseifs...";

    i=0;
    while(i<elseifs.size()){
        insts[elseifs[i]]->type = ByteCodeInstruction2::TypeFCOMP;

        qDebug()<<"Turned to FComp:"<<insts[elseifs[i]]->toString(0);

        i++;
    }

    qDebug()<<"setting elseifs...";

    i=0;
    while(i<returns.size()){
        insts[returns[i]]->type = ByteCodeInstruction2::TypeReturnMarker;

        qDebug()<<"Turned to FComp:"<<insts[returns[i]]->toString(0);

        i++;
    }

    i=insts.size()-1;

    i=0;
    while(i<insts.size()-1){
        if(insts[i]->type==ByteCodeInstruction2::TypeJMP){
            int id = insts[i]->A->data;
            int e=i;
            int count=0;
            bool found=false;
            while(e<insts.size() && found==false){
                //if(insts[e]->type==ByteCodeInstruction2::TypeEndIfMarker || insts[e]->type==ByteCodeInstruction2::TypeBackJMP){
                if(insts[e]->isEndOfIf==true || insts[e]->type==ByteCodeInstruction2::TypeBackJMP){
                    int compId;
                    if(insts[e]->isEndOfIf!=true){
                        compId = insts[e]->A->data;
                    }
                    else{
                        compId = insts[e]->conditionalId;
                    }
                    if(compId==id){
                        found=true;
                        qDebug()<<"found e:"<<e;
                        e--;
                    }
                    else{
                        qDebug()<<"Skipping over:"<<insts[e]->toString(0);
                        count++;
                    }
                }
                else{
                    qDebug()<<"Skipping over:"<<insts[e]->toString(0);
                    count++;
                }
                e++;
            }
            qDebug()<<"repointing...";
            qDebug()<<"found e:"<<e;
            if(found){
                if(insts[e]->isEndOfIf){
                    qDebug()<<"type end-if...";
//                    insts.remove(e);
                    insts[i]->A = new Token2(Token2::TypeCode,"",count+1);
                }
                else{
                    qDebug()<<"type back jump...";
                    insts[e]->A = new Token2(Token2::TypeCode,"",count+1);
                    insts[i]->A = new Token2(Token2::TypeCode,"",count+1);
                }
            }
            else{
                qDebug()<<"ERROR! not found match for:"<<i;
            }
            qDebug()<<"Jump Count:"<<count;

            qDebug()<<"End of instruction set conditional modification";
            int e3=0;
            while(e3<insts.size()){
                qDebug()<<insts[e3]->toString(0);
                e3++;
            }
        }
        //i--;
        i++;
    }

    i=0;
    while(i<preinsts.size()){

        //this->preinsts.append(new ByteCodeInstruction2(ByteCodeInstruction2::TypeLET,this->data[i][e+1],NULL,NULL));

        fininsts.append(preinsts[i]);
        i++;
    }
   // qDebug()<<"returning "+QString::number(fininsts.size())+" worth of instructions.";
    i=0;
    while(i<insts.size()){
        fininsts.append(insts[i]);
        i++;
    }
    i=0;

    qDebug().noquote()<<"final instruction set:";
    while(i<fininsts.size()){
        qDebug().noquote()<<"finists["<<i<<"]:"<<fininsts[i]->toString(0);
        i++;
    }

    qDebug()<<"returning "+QString::number(fininsts.size())+" worth of instructions.";

    this->instructionSet = fininsts;
    return fininsts;
}

PByteArray* LexNode::byteCode(bool top){
    PByteArray* res = new PByteArray();
    int i=0;
    while(i<this->instructionSet.size()){
        PByteArray* res3 = this->instructionSet[i]->compile(this);
        res->copyInto(res->size(),res3,res3->size());
        i++;
    }
    return res;
}

QVector<QString> LexNode::getIncludeURLs(){

    qDebug()<<"Checking include urls from lexnode, data of size:"<<this->data.size();

    QVector<QString> res={};
    int e=0;
    while(e<this->data.size()){
        int i=0;
        while(i<this->data[e].size()){
            if(this->data[e][i]->type==Token2::TypeCode){
                if(QString::compare(this->data[e][i]->_string,"include")==0){
                    qDebug()<<"Found include statement!!";
                    res.push_back(this->data[e][i+1]->_string);
                    this->data.remove(e);
                }
            }
            i++;
        }
        e++;
    }
    return res;
}

void LexNode::stripIncludes(){

}

PByteArray* LexNode::compile(bool top){
    PByteArray* result = new PByteArray();
    if(top){
        LexNode::HighestRPN=0;
        //this->compileToTokenStream(top);
        int discard=0;
        this->reorderTokenStream(!top,false,false,false,discard,{});
        this->precompile(top);
        LexNode::HighestRPN++;

        PByteArray* preHeap = this->preHeapByteCode(top,new PByteArray());
        result->copyInto(0,preHeap,preHeap->size());

        PByteArray* sub = new PByteArray();
        qDebug()<<"RPN Count:"<<LexNode::HighestRPN;
        sub->setNum(preHeap->size()-((Architecture::RawIntegerSize*2)+2));
        result->copyInto(Architecture::RawIntegerSize+2,sub,4);

        PByteArray* instCodes = this->byteCode(top);
        result->copyInto(result->size(),instCodes,instCodes->size());
    }
    qDebug()<<"Compiled top lex node...";
    return result;
}
