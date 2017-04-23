#include "lineblock.h"

#include<QDebug>

int getIndentation(QString str){
    int i=0;
    int tabs=0;
    int spaces=0;
    bool nonWS=false;
    while(nonWS==false && i<str.length()){
        if(str[i]!='\t' && str[i]!=' '){
            nonWS=true;
        }
        else{
            if(str[i]=='\t'){
                tabs++;
            }
            else{
                spaces++;
            }
            i++;
        }
    }
    if(nonWS==false && i==str.length()-1){
        return 0;
    }
    else{
        int res = spaces+(4*tabs);
        if(res==0){return 0;}
        else if(res%4!=0){
            qDebug()<<"ERROR!! Incorrect Indentation!";
            return 0;
        }
        else{res=res/4;}
        return res;
    }
}

LineBlock::LineBlock(QVector<Token*> data){
    this->lineNumber=-1;
    this->data =data;
}

bool LineBlock::isMethodSignature(){
    bool fail=false;
    QString method_address = "[PRIMITIVEADDRESS];";
    if(this->data.size()>1){
        int i=0;
        int meth=0;
        bool pass=false;
        while(i<this->data.size()-1){
            if(QString::compare(this->data[i]->getString(),")")==0 || QString::compare(this->data[i]->getString(),"(")==0){
                meth++;
            }
            if(QString::compare(this->data[i]->getString(),"lambda")==0 ||
               QString::compare(this->data[i]->getString(),"literal")==0 ||
               QString::compare(this->data[i]->getString(),"classdef")==0 ||
                    QString::compare(this->data[i]->getString(),":")==0){
                fail=true;
            }
            if(QString::compare(this->data[i]->getString(),method_address)==0){
                pass=true;
            }
            i++;
        }
        if(fail==true){
            return false;
        }
        if(meth==2 && pass==true && QString::compare(this->data.last()->getString(),":")==0){
            return true;
        }
    }
    return false;
}

LineBlock::LineBlock(int lineNumber, QString line){

    //need to fix to handle empty strings

    qDebug()<<"Making a lineblock from:";
    qDebug()<<"linenumber:"+QString::number(lineNumber);
    qDebug()<<"Making a lineblock from:"+line;
    this->lineNumber=lineNumber;
    this->indent = getIndentation(line);
    int o=0;
    QString container="";
    char separator=' ';
    QVector<bool> isCode;
    bool inString=false;

    QVector<Token*> byString;

    QVector<Token*> final;
    while(o<line.length()){
        if(line[o]=='\'' || line[o]=='"'){
            bool dontDo=false;
            qDebug()<<"Separator encountered...";
            if(separator==' '){
                qDebug()<<"First separator...";
                if(inString==true){
                    inString=false;
                }
                else{
                    inString=true;
                }
                separator = line.at(o).toLatin1();
            }
            else{
                qDebug()<<"Not first separator...";
                if(o>=2){
                    qDebug()<<"O greater than 1";
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
                        qDebug()<<"Non-escaped string container.";
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
                        qDebug()<<"Non-escaped string container.";
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
                byString.append(new Token(container));
                isCode.append(inString);
                container="";
            }
        }
        else{
            container+=line[o];
        }
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
        }
        o++;
    }
    if(container.length()>0){
        byString.append(new Token(container));
        isCode.append(!inString);
        container="";
    }
    o=0;
    while(o<isCode.size()){
        if(isCode[o]==true){
            QStringList list = byString[o]->getString().split(QRegExp("\\s"));
            int i=0;
            while(i<list.size()){
                if(list[i].length()>0){

                    QRegExp reg = QRegExp("[^_a-zA-Z\\d\\s]");

                    int x = reg.indexIn(list[i]);
                    if(x>=0){
                        int e=0;
                        container="";
                        while(e<list[i].length()){
                            //if(list[i].at(e).isLetter() || list[i].at(e).isDigit()){
                            if(list[i].at(e).toLatin1() =='_' || list[i].at(e).isLetter() || list[i].at(e).isDigit()){
                                std::string stdStr = (list[i].toStdString());
                                char a = stdStr[e];
                                container+=a;
                            }
                            else{
                                if(container.length()>0){
                                    final.append(new Token(container));
                                    container="";
                                }
                                final.append(new Token(QString(QChar(list[i].toStdString()[e]))));
                            }
                            e++;
                        }
                        if(container.length()>0){
                            final.append(new Token(container));
                        }
                    }
                    else{
                        final.append(new Token(list[i]));
                    }
                }
                i++;
            }
        }
        else{

            QString finData = byString[o]->getString();

            int u=0;
            while(u<finData.length()){
                if(finData.at(u).toLatin1()=='\\'){
              //      finData = finData.remove(u,1);
                }
                u++;
            }

            final.append(new Token("\""+finData+"\""));
        }
        o++;
    }
    QVector<Token*>res;
    int i=0;
    bool esc=false;
    while(i<final.size()-1 && esc==false){
        bool isDecimal=false;
        bool isSingleDecimal=false;
        if(i<final.size()-2){
            qDebug()<<"checking for decimal!";
            QString str = final[i]->getString();
            bool ok;
            int v = str.toInt(&ok, 16);

            if(ok){
                qDebug()<<str<<" is an integer...";
            }
            else{
                qDebug()<<str<<" is NOT an integer...";
            }

            QString str2 = final[i+2]->getString();
            bool ok2;
            int v2 = str2.toInt(&ok2, 16);

            if(ok2){
                qDebug()<<str2<<" is an integer...";
            }
            else{
                qDebug()<<str2<<" is NOT an integer...";
            }

            if(ok==true && ok2==true && QString::compare(".",final[i+1]->getString())==0){
                qDebug()<<"encountered decimal!";
                isDecimal=true;
            }
            if(!isDecimal){
                qDebug()<<"Checking for single decimal... on "<<str;
                QStringList str3 = str.split('.');

                int q=0;
                while(q<str3.length()){
                    qDebug()<<str3[q];
                    q++;
                }

                if(str3.length()==2){
                    qDebug()<<"Correct length...";
                    QString str4 = str3[0];
                    bool ok;
                    int v3 = str4.toInt(&ok, 16);

                    if(ok){
                        qDebug()<<str4<<" is an integer...";
                    }
                    else{
                        qDebug()<<str4<<" is NOT an integer...";
                    }

                    QString str5 = str3[1];
                    bool ok2;
                    int v4 = str5.toInt(&ok2, 16);

                    if(ok2){
                        qDebug()<<str5<<" is an integer...";
                    }
                    else{
                        qDebug()<<str5<<" is NOT an integer...";
                    }

                    if(ok==true && ok2==true){
                        qDebug()<<"encountered decimal!";
                        isSingleDecimal=true;
                    }
                }
            }
        }
        if(isDecimal==true){
            QString str="";
            str.append(final[i]->getString());
            str.append(final[i+1]->getString());
            str.append(final[i+2]->getString());
            Token* t = new Token(str);
            res.append(t);
            i++;
            i++;
        }
        else if(isSingleDecimal==true){
            QString str="";
            str.append(final[i]->getString());
            Token* t = new Token(str);
            res.append(t);
            i++;
        }
        else if(
          QString::compare(final[i]->getString(),"!")==0
                &&
                QString::compare(final[i+1]->getString(),"=")==0
                ){
            if(i<final.size()-1 && QString::compare(final[i+2]->getString(),"=")==0){
                Token* t = new Token("!==");
                res.append(t);
                i++;
                i++;
            }
            else{
                Token* t = new Token("!=");
                res.append(t);
                i++;
            }
        }
        else if(
          QString::compare(final[i]->getString(),"+")==0
                &&
                QString::compare(final[i+1]->getString(),"+")==0
                ){
            Token* t = new Token("++");
            res.append(t);
            i++;
        }
        else if(
          QString::compare(final[i]->getString(),"<")==0
                &&
                QString::compare(final[i+1]->getString(),"=")==0
                ){
            Token* t = new Token("<=");
            res.append(t);
            i++;
        }
        else if(
          QString::compare(final[i]->getString(),">")==0
                &&
                QString::compare(final[i+1]->getString(),"=")==0
                ){
            Token* t = new Token(">=");
            res.append(t);
            i++;
        }
        else if(  QString::compare(final[i]->getString(),"-")==0
                  &&
                  QString::compare(final[i+1]->getString(),"-")==0
                  ){
              Token* t = new Token("--");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"+")==0
                  &&
                  QString::compare(final[i+1]->getString(),"=")==0
                  ){
              Token* t = new Token("+=");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"-")==0
                  &&
                  QString::compare(final[i+1]->getString(),"=")==0
                  ){
              Token* t = new Token("-=");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"/")==0
                  &&
                  QString::compare(final[i+1]->getString(),"=")==0
                  ){
              Token* t = new Token("/=");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"*")==0
                  &&
                  QString::compare(final[i+1]->getString(),"=")==0
                  ){
              Token* t = new Token("*=");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"%")==0
                  &&
                  QString::compare(final[i+1]->getString(),"=")==0
                  ){
              Token* t = new Token("%=");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"=")==0
                  &&
                  QString::compare(final[i+1]->getString(),"=")==0
                  ){
              Token* t = new Token("==");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"&")==0
                  &&
                  QString::compare(final[i+1]->getString(),"&")==0
                  ){
              Token* t = new Token("&&");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"|")==0
                  &&
                  QString::compare(final[i+1]->getString(),"|")==0
                  ){
              Token* t = new Token("||");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),":")==0
                  &&
                  QString::compare(final[i+1]->getString(),":")==0
                  ){
              Token* t = new Token("::");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"*")==0
                  &&
                  QString::compare(final[i+1]->getString(),"*")==0
                  ){
              Token* t = new Token("**");
              res.append(t);
              i++;
        }
        else if(  QString::compare(final[i]->getString(),"/")==0
                  &&
                  QString::compare(final[i+1]->getString(),"/")==0
                  ){
            esc=true;
        }
        else if(  QString::compare(final[i]->getString(),"else")==0
                  &&
                  QString::compare(final[i+1]->getString(),"if")==0
                  ){
            Token* t = new Token("elseif");
            res.append(t);
            i++;
        }
        else{
            res.append(final[i]);
        }
        i++;
    }
    if(i>=0 && i<final.size() && esc==false){
        res.append(final.last());
    }
    this->data=res;
    if(this->data.size()==2 && QString::compare(this->data.first()->getString(),"else")==0){
        this->data = {
            new Token("elseif"),
            new Token("("),
            new Token("true"),
            new Token(")"),
            new Token(":")
        };
    }

    qDebug()<<"And finished with:"+this->toString();
}

int LineBlock::size(){
    return this->data.size();
}

int LineBlock::getIndent(){
   // qDebug()<<"Lineblock retrieving identation:"+QString::number(this->indent);
    return this->indent;
}

QVector<Token*> LineBlock::getData(){
    return this->data;
}

int LineBlock::getLineNumber(){
    return this->lineNumber;
}

QString LineBlock::toString(){
    int i=0;
    QString res="[ln:"+QString::number(this->lineNumber)+",id:"+QString::number(this->indent)+"]";
    while(i<this->data.size()){
        res+="{"+this->data[i]->getString()+"}";
        i++;
    }
    return res;
}
