#include "linehelper.h"
#include <QDebug>

LineHelper::LineHelper(){

}

QVector<Token*> LineHelper::removeBrackets(QVector<Token*> data){
    QVector<Token*> res;
    int i=0;
    while(i<data.size()){
        if(QString::compare("(",data[i]->getString())!=0 &&
           QString::compare(")",data[i]->getString())!=0 &&
           QString::compare("[",data[i]->getString())!=0 &&
           QString::compare("]",data[i]->getString())!=0){
            res.append(data[i]);
        }
        i++;
    }
    return res;
}

int LineHelper::countOperators(QVector<Token*> data){
    int i=0;
    int res=0;
    while(i<data.size()){
        if(data[i]->isOperator()==true){
            res++;
        }
        i++;
    }
    return res;
}

bool LineHelper::hasDuplicateTokenTypes(QVector<Token*> data){
    int i=0;
    bool res=false;
    char last=' ';//T - OperaTor , N - OperaNd
    char current=' ';
    while(i<data.size() && res==false){
        if(data[i]->isOperator()==true){
            current='T';
        }
        else{
            current='N';
        }
        if(current==last){
            res=true;
        }
        else{
            last=current;
        }
        i++;
    }
    return res;
}

QVector<Token*> LineHelper::insertInvocationMarkers(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    while(i<data.size()){
        if(i>0){
            if(QString::compare(data[i]->getString(),"(")==0){
                if((QString::compare(data[i-1]->getString(),"]")==0 || data[i-1]->isOperator()==false) &&
                        QString::compare(data[i-1]->getString(),"lambda")!=0){
                    res.append(new Token(Token::TypeInvocationMarker));
                }
            }
        }
        if(QString::compare(data[i]->getString(),":")!=0){
            res.append(data[i]);
        }
        i++;
    }
    return res;
}

QVector<Token2*> LineHelper::insertInvocationMarkers2(QVector<Token2*> data){
    int i=0;
    QVector<Token2*> res;
    while(i<data.size()){
        if(i>0){
            if(QString::compare(data[i]->_string,"(")==0){
                if((QString::compare(data[i-1]->_string,"]")==0 || data[i-1]->isOperator()==false) &&
                        data[i-1]->isKeyWord()==false){
                    res.append(new Token2(Token2::TypeOperator,"$"));
                }
            }
        }
        if(QString::compare(data[i]->_string,":")!=0){
            res.append(data[i]);
        }
        i++;
    }
    return res;
}


QVector<Token*> LineHelper::insertNOARGMarkers(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    bool added=false;
    while(i<data.size()){
        added=false;
        if(i>1){
            if(QString::compare(data[i]->getString(),")")==0){
                if(data[i-2]->getType()==Token::TypeInvocationMarker){
                    if(QString::compare(data[i-1]->getString(),"(")==0){
                        res.removeLast();
                        res.append(new Token(Token::TypeNOARGSMarker));
                        added=true;
                    }
                }
            }
        }
        if(added==false){
            res.append(data[i]);
        }
        i++;
    }
    return res;
}
QVector<Token2*> LineHelper::insertNOARGMarkers2(QVector<Token2*> data){
    int i=0;
    QVector<Token2*> res;
    bool added=false;
    while(i<data.size()){
        added=false;
        if(i>1){
            if(QString::compare(data[i]->_string,")")==0){
                if(data[i-2]->type==Token2::TypeOperator && QString::compare(data[i-2]->_string,"$")==0){
                    if(data[i-1]->type==Token2::TypeCode && QString::compare(data[i-1]->_string,"(")==0){
                        res.removeLast();
                        res.append(new Token2(Token2::TypeCode,"NOARGS"));
                        added=true;
                    }
                }
            }
        }
        if(added==false){
            res.append(data[i]);
        }
        i++;
    }
    return res;
}
QVector<Token*> LineHelper::insertDUDRights(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    while(i<data.size()){
        res.append(data[i]);
        if(data[i]->isRightUnaryOperator()==true){
            res.append(new Token(Token::TypeDUDRight));
        }
        i++;
    }
    return res;
}
QVector<Token2*> LineHelper::insertDUDRights2(QVector<Token2*> data){
    int i=0;
    QVector<Token2*> res;
    while(i<data.size()){
        res.append(data[i]);
        if(data[i]->isRightUnaryOperator()==true){
            res.append(new Token2(Token2::TypeOperator,"DUDRIGHT"));
        }
        i++;
    }
    return res;
}

QVector<Token*> LineHelper::insertDUDLefts(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    while(i<data.size()){
        if(data[i]->isLeftUnaryOperator()==true){
            res.append(new Token(Token::TypeDUDLeft));
        }
        res.append(data[i]);
        i++;
    }
    return res;
}
QVector<Token2*> LineHelper::insertDUDLefts2(QVector<Token2*> data){
    int i=0;
    QVector<Token2*> res;
    while(i<data.size()){
        if(data[i]->isLeftUnaryOperator()==true){
            res.append(new Token2(Token2::TypeOperator,"DUDLEFT"));
        }
        res.append(data[i]);
        i++;
    }
    return res;
}

QVector<Token*> LineHelper::insertArrayMarkers(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    while(i<data.size()){
        res.append(data[i]);
        if(i==0){
            if(QString::compare(data[i]->getString(),"[")==0){
                res.append(new Token(Token::TypeArrayInitMarker));
                if(QString::compare(data[i+1]->getString(),"]")!=0){
                    res.append(new Token(","));
                }
            }
        }
        else{
            if(QString::compare(data[i]->getString(),"[")==0 && data[i-1]->isOperator()==true){
                res.append(new Token(Token::TypeArrayInitMarker));
                if(i!=data.size()-1){
                    if(QString::compare(data[i+1]->getString(),"]")!=0){
                        res.append(new Token(","));
                    }
                }
            }
            else if(QString::compare(data[i]->getString(),"[")==0){
                res.removeLast();
                res.append(new Token(Token::TypeArrayLookUpMarker));
                res.append(data[i]);
            }
        }
        i++;
    }
    return res;
}
QVector<Token2*> LineHelper::insertArrayMarkers2(QVector<Token2*> data){
    int i=0;
    QVector<Token2*> res;
    while(i<data.size()){
        res.append(data[i]);
        if(i==0){
            if(QString::compare(data[i]->_string,"[")==0){
                res.append(new Token2(Token2::TypeOperator,"ARRAY_INIT"));
                if(QString::compare(data[i+1]->_string,"]")!=0){
                    res.append(new Token2(Token2::TypeCode,","));
                }
            }
            else if(QString::compare(data[i]->_string,"]")==0){
                res.removeLast();
                res.append(new Token2(Token2::TypeCode,"]"));
          //      res.append(new Token2(Token2::TypeCode,","));
                res.append(new Token2(Token2::TypeOperator,"ARRAY_CLOSE"));
            }
        }
        else{
            if(QString::compare(data[i]->_string,"[")==0 && data[i-1]->isOperator()==true){
                res.append(new Token2(Token2::TypeOperator,"ARRAY_INIT"));
                if(i!=data.size()-1){
                    if(QString::compare(data[i+1]->_string,"]")!=0){
                        res.append(new Token2(Token2::TypeCode,","));
                    }
                }
            }
            else if(QString::compare(data[i]->_string,"[")==0){
                res.removeLast();
                res.append(new Token2(Token2::TypeOperator,"ARRAY_LOOK_UP"));
                res.append(data[i]);
            }
        }
        i++;
    }
    return res;
}

QVector<Token*> LineHelper::removeColons(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    while(i<data.size()){
        if(QString::compare(":",data[i]->getString())!=0){
            res.append(data[i]);
        }
        i++;
    }
    return res;
}

QVector<Token*> LineHelper::removeLambdaArgs(QVector<Token*> data){
    int i=0;
    QVector<Token*> res;
    bool inLambda = false;
    while(i<data.size()){
        if(inLambda==false){
            if(QString::compare("lambda",data[i]->getString())==0){
                inLambda = true;
            }
            res.append(data[i]);
        }
        else{
            if(QString::compare(")",data[i]->getString())==0){
                inLambda = false;
            }
        }
        i++;
    }
    return res;
}

QVector<Token2*> LineHelper::removeLambdaArgs2(QVector<Token2*> data){
    int i=0;
    QVector<Token2*> res;
    bool inLambda = false;
    while(i<data.size()){
        if(inLambda==false){
            if(i<data.size()-1 && QString::compare("lambda",data[i]->_string)==0 && QString::compare("(",data[i+1]->_string)==0){
                inLambda = true;
            }
            res.append(data[i]);
        }
        else{
            if(QString::compare(")",data[i]->_string)==0){
                inLambda = false;
            }
        }
        i++;
    }
    return res;
}

int LineHelper::countLeftBrackets(QVector<Token*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare("(",data[i]->getString())==0){
            e++;
        }
        i++;
    }
    return e;
}
int LineHelper::countRightBrackets(QVector<Token*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare(")",data[i]->getString())==0){
            e++;
        }
        i++;
    }
    return e;
}

int LineHelper::countLeftBrackets2(QVector<Token2*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare("(",data[i]->_string)==0){
            e++;
        }
        i++;
    }
    return e;
}
int LineHelper::countRightBrackets2(QVector<Token2*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare(")",data[i]->_string)==0){
            e++;
        }
        i++;
    }
    return e;
}

int LineHelper::countRightSquareBrackets2(QVector<Token2*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare("]",data[i]->_string)==0){
            e++;
        }
        i++;
    }
    return e;
}

int LineHelper::countLeftSquareBrackets2(QVector<Token2*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare("[",data[i]->_string)==0){
            e++;
        }
        i++;
    }
    return e;
}
int LineHelper::countRightSquareBrackets(QVector<Token*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare("]",data[i]->getString())==0){
            e++;
        }
        i++;
    }
    return e;
}

int LineHelper::countLeftSquareBrackets(QVector<Token*> data){
    int i=0;
    int e=0;
    while(i<data.size()){
        if(QString::compare("[",data[i]->getString())==0){
            e++;
        }
        i++;
    }
    return e;
}

//Here is the expression validation....

bool LineHelper::isValidPostFix(QVector<Token*> data){
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.
*/
    //qDebug()<<"Validating line";
    int i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    //qDebug()<<"remove lambda signatures:";
    data = removeLambdaArgs(data);
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    //qDebug()<<"remove colons:";
    data = removeColons(data);
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    //qDebug()<<"Insert Invocation Markers:";

    data = insertInvocationMarkers(data);
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    //qDebug()<<"Insert No Arguments:";
    data = insertNOARGMarkers(data);
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    data = insertDUDLefts(data);
    //qDebug()<<"Insert Dud Lefts:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    data = insertDUDRights(data);
    //qDebug()<<"Insert Dud Rights:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }
    data = insertArrayMarkers(data);
    //qDebug()<<"Insert Array Markers:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }
    int lBC = countLeftBrackets(data);
    int rBC = countRightBrackets(data);
    int lSBC = countLeftSquareBrackets(data);
    int rSBC = countRightSquareBrackets(data);
    //qDebug()<<"\nlBC:"+QString::number(lBC)+",rBC:"+QString::number(rBC)+",lSBC:"+QString::number(lSBC)+",rSBC:"+QString::number(rSBC)+"\n";
    if(lBC==rBC && lSBC==rSBC){
        data = removeBrackets(data);
        int operatorCount = countOperators(data);
        int operandCount = data.size()-operatorCount;
        bool dupes = hasDuplicateTokenTypes(data);
        //qDebug()<<"\noperator count:"+QString::number(operatorCount)+",operand count:"+QString::number(operandCount)+",dupes:"+QString::number(dupes)+"\n";
        if(dupes==false && (operatorCount == operandCount-1)){
            //qDebug()<<"Valid Line!";
            return true;
        }
        //qDebug()<<"Invalid Line!";
        return false;
    }
    //qDebug()<<"Invalid Line!";
    return false;
}

QVector<Token2*> LineHelper::convertToPostFix2(QVector<Token2*> data){
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.
*/
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.

An Array is not the same as an arguments list.
(), like [], declare precedence and are not retained in RPN.
We also need therefore an [EMPTY_ARRAY] marker, and to substitute [,] with [ARRAY_COMMA] when within an array (unless within an expression within an array.)
An array with 1 argument is detected by the compiler and [DUDLEFT][ARRAY_COMMA] inserted beforehand, again before RPN.
*/
    data = insertInvocationMarkers2(data);
    //qDebug()<<"Validating line:";
    int i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->_string;
        i++;
    }
    data = insertNOARGMarkers2(data);
    //qDebug()<<"Insert No Arguments:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->_string;
        i++;
    }


    //qDebug()<<"remove lambda signatures:";
    data = removeLambdaArgs2(data);
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->_string;
        i++;
    }

    data = insertDUDLefts2(data);
    //qDebug()<<"Insert Dud Lefts:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->_string;
        i++;
    }

    data = insertDUDRights2(data);
    //qDebug()<<"Insert Dud Rights:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->_string;
        i++;
    }
    data = insertArrayMarkers2(data);
    //qDebug()<<"Insert Array Markers:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->_string;
        i++;
    }
    int leftBracketCount=countLeftBrackets2(data);
    int rightBracketCount=countRightBrackets2(data);
    int rightSquareBracketCount=countRightSquareBrackets2(data);
    int leftSquareBracketCount=countLeftSquareBrackets2(data);
    if(leftBracketCount==rightBracketCount && leftSquareBracketCount==rightSquareBracketCount){
        bool res=false;
        QVector<Token2*> operandStack;
        QVector<Token2*> operatorStack;
        int i=0;
        bool fatalError=false;
        while(i<data.size()){
            //qDebug()<<"Considering:"<<data[i]->_string;
            if(data[i]->isOperator()==true){
/*                if(data[i]->isLeftSquareBracket()==true){
                    operatorStack.append(data[i]);
                }
                else{*/
                    if(operatorStack.size()==0){
                        qDebug()<<"Adding '"<<data[i]->_string<<"' to the operator stack.";
                        operatorStack.append(data[i]);
                    }
                    else if(QString::compare(data[i]->_string,"(")==0 || QString::compare(data[i]->_string,"[")==0){
                        qDebug()<<"Adding '"<<data[i]->_string<<"' to the operator stack.";
                        operatorStack.append(data[i]);
                    }
                    else{
                        int topPrecedence = operatorStack.last()->operatorPrecedence();
                        int currentPrecedence = data[i]->operatorPrecedence();
                        bool isRightBracket = false;
                        if(QString::compare(data[i]->_string,")")==0){
                            isRightBracket=true;
                        }
                        bool isRightSquareBracket = false;
                        if(QString::compare(data[i]->_string,"]")==0){
                            isRightSquareBracket=true;
                        }
                        if(currentPrecedence<=topPrecedence){
                     //   if(currentPrecedence<topPrecedence){
                            do{
                                qDebug()<<"Popping "<<operatorStack.last()->_string<<" of operator stack.";
                               // currentPrecedence = operatorStack.last()->operatorPrecedence();
                                if(isRightBracket==true && QString::compare(operatorStack.last()->_string,"(")==0){
                                    operatorStack.removeLast();
                                    if(operatorStack.size()>0 && (operatorStack.last()->type==Token2::TypeOperator)  && (QString::compare(operatorStack.last()->_string,"$")==0)){
                                        operandStack.append(operatorStack.last());
                                        operatorStack.removeLast();
                                    }
                                    topPrecedence=-1;
                                }
                                else if(isRightSquareBracket==true && QString::compare(operatorStack.last()->_string,"[")==0){
                                    operatorStack.removeLast();
                                    if(operatorStack.last()->type==Token::TypeArrayLookUpMarker){
                                        operandStack.append(operatorStack.last());
                                        operatorStack.removeLast();
                                    }
                                    topPrecedence=-1;
                                }
                                else if(QString::compare(operatorStack.last()->_string,",")==0){
                                   // operatorStack.removeLast();
                                 /*   if(operatorStack.last()->type==Token::TypeArrayLookUpMarker){
                                        operandStack.append(operatorStack.last());
                                        operatorStack.removeLast();
                                    }*/
                                    topPrecedence=-1;
                                }
                                else{
                                    if(
                                            (QString::compare(operatorStack.last()->_string,"(")!=0) &&
                                            (QString::compare(operatorStack.last()->_string,")")!=0) &&
                                            (QString::compare(operatorStack.last()->_string,"[")!=0) &&
                                            (QString::compare(operatorStack.last()->_string,"]")!=0)
                                    ){
                                        operandStack.append(operatorStack.last());
                                    }
                                    operatorStack.removeLast();
                                    if(operatorStack.size()>0){
                                        topPrecedence = operatorStack.last()->operatorPrecedence();
                                    }
                                    else{
                                        topPrecedence=-1;
                                    }
                                }
                                qDebug()<<"end of while...";
                        //    }while(currentPrecedence<topPrecedence);
                            }while(currentPrecedence<=topPrecedence);
                            qDebug()<<"Adding '"<<data[i]->_string<<"' to the operator stack.";
                            operatorStack.append(data[i]);
                        }
                        else{
                            qDebug()<<"Adding '"<<data[i]->_string<<"' to the operator stack.";
                            operatorStack.append(data[i]);
                        }
                    }
                }
         //   }
            else{
                qDebug()<<"Adding '"<<data[i]->_string<<"' to the operand stack.";
                operandStack.append(data[i]);
            }
            i++;
        }
        if(fatalError==false){
            res=true;
            i=operatorStack.size()-1;
            while(i>=0 && i<operatorStack.size()){
                qDebug()<<operatorStack.last()->_string<<" on top of stack.";
                if(QString::compare(operatorStack.last()->_string,"(")==0 ||
                        QString::compare(operatorStack.last()->_string,")")==0 ||
                        QString::compare(operatorStack.last()->_string,"[")==0 ||
                        QString::compare(operatorStack.last()->_string,"]")==0){
                //   res=false;
                }
                else{
                    //qDebug()<<operatorStack.last()->_string<<" is NOT a bracket";
                    operandStack.append(operatorStack.last());
                }
                //qDebug()<<"removing from operatorStack:"<<operatorStack.last()->_string;
                operatorStack.removeLast();
                i--;
            }
            i=0;
            //qDebug()<<"Operand Stack:";
            while(i<operandStack.size()){
                ////qDebug()<<"OS:"<<operandStack[i]->_string;
                if(QString::compare(operandStack[i]->_string,"(")==0 ||
                   QString::compare(operandStack[i]->_string,")")==0 ||
                   QString::compare(operandStack[i]->_string,"[")==0 ||
                   QString::compare(operandStack[i]->_string,"]")==0){
                    operandStack.remove(i);
                    i--;
                }
                i++;
            }
//            //qDebug()<<"And Res:"<<res;
            //qDebug()<<"Result of rpn: "<<operandStack;
            return operandStack;
        }
        else{
            i=0;
            //qDebug()<<"Operand Stack:";
            while(i<operandStack.size()){
                //qDebug()<<"OS:"<<operandStack[i]->_string;
                i++;
            }
            //qDebug()<<"Encountered Fatal Error in RPN conversion";
            return {};
        }
    }
    else{
        return {};
    }
}
bool LineHelper::isValidPostFix2(QVector<Token2*> data){
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.
*/
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.

An Array is not the same as an arguments list.
(), like [], declare precedence and are not retained in RPN.
We also need therefore an [EMPTY_ARRAY] marker, and to substitute [,] with [ARRAY_COMMA] when within an array (unless within an expression within an array.)
An array with 1 argument is detected by the compiler and [DUDLEFT][ARRAY_COMMA] inserted beforehand, again before RPN.
*/

    if(QString::compare(data.first()->_string,"if")==0){
        data.removeFirst();
    }
    else if(QString::compare(data.first()->_string,"elseif")==0){
        data.removeFirst();
    }
    else if(QString::compare(data.first()->_string,"while")==0){
        data.removeFirst();
    }

    data = insertInvocationMarkers2(data);
    qDebug()<<"Validating line:";
    int i=0;
    while(i<data.size()){
        qDebug()<<data[i]->_string;
        i++;
    }
    data = insertNOARGMarkers2(data);
    qDebug()<<"Insert No Arguments:";
    i=0;
    while(i<data.size()){
        qDebug()<<data[i]->_string;
        i++;
    }


    qDebug()<<"remove lambda signatures:";
    data = removeLambdaArgs2(data);
    i=0;
    while(i<data.size()){
        qDebug()<<data[i]->_string;
        i++;
    }

    data = insertDUDLefts2(data);
    qDebug()<<"Insert Dud Lefts:";
    i=0;
    while(i<data.size()){
        qDebug()<<data[i]->_string;
        i++;
    }

    data = insertDUDRights2(data);
    qDebug()<<"Insert Dud Rights:";
    i=0;
    while(i<data.size()){
        qDebug()<<data[i]->_string;
        i++;
    }
    data = insertArrayMarkers2(data);
    qDebug()<<"Insert Array Markers:";
    i=0;
    while(i<data.size()){
        qDebug()<<data[i]->_string;
        i++;
    }
    qDebug()<<"Finished marker insertion...";
    int leftBracketCount=countLeftBrackets2(data);
    int rightBracketCount=countRightBrackets2(data);
    int rightSquareBracketCount=countRightSquareBrackets2(data);
    int leftSquareBracketCount=countLeftSquareBrackets2(data);
    qDebug()<<"Counted brackets...";

    if(leftBracketCount==rightBracketCount && leftSquareBracketCount==rightSquareBracketCount){
        return true;
    }
    else{
        return false;
    }
}

QVector<Token*> LineHelper::convertToPostFix(QVector<Token*> data){
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.
*/
    //from slideshow notes:
    /*To make straightforward RPN conversion work you need:
An [INVOCATION] marker for function calls between the function name and the paranthesis
A [NOARGS] marker inserted in function calls without arguments (where arguments would go).
A [DUDRIGHT] marker inserted after each unary operator.
Before converting to RPN.

An Array is not the same as an arguments list.
(), like [], declare precedence and are not retained in RPN.
We also need therefore an [EMPTY_ARRAY] marker, and to substitute [,] with [ARRAY_COMMA] when within an array (unless within an expression within an array.)
An array with 1 argument is detected by the compiler and [DUDLEFT][ARRAY_COMMA] inserted beforehand, again before RPN.
*/
    data = insertInvocationMarkers(data);
    //qDebug()<<"Validating line:";
    int i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }
    data = insertNOARGMarkers(data);
    //qDebug()<<"Insert No Arguments:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }


    //qDebug()<<"remove lambda signatures:";
    data = removeLambdaArgs(data);
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    data = insertDUDLefts(data);
    //qDebug()<<"Insert Dud Lefts:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }

    data = insertDUDRights(data);
    //qDebug()<<"Insert Dud Rights:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }
    data = insertArrayMarkers(data);
    //qDebug()<<"Insert Array Markers:";
    i=0;
    while(i<data.size()){
        //qDebug()<<data[i]->getString();
        i++;
    }
    int leftBracketCount=countLeftBrackets(data);
    int rightBracketCount=countRightBrackets(data);
    int rightSquareBracketCount=countRightSquareBrackets(data);
    int leftSquareBracketCount=countLeftSquareBrackets(data);
    if(leftBracketCount==rightBracketCount && leftSquareBracketCount==rightSquareBracketCount){
        bool res=false;
        QVector<Token*> operandStack;
        QVector<Token*> operatorStack;
        int i=0;
        bool fatalError=false;
        while(i<data.size()){
            //qDebug()<<"Considering:"<<data[i]->getString();
            if(data[i]->isOperator()==true){
/*                if(data[i]->isLeftSquareBracket()==true){
                    operatorStack.append(data[i]);
                }
                else{*/
                    if(operatorStack.size()==0){
                        //qDebug()<<"Adding '"<<data[i]->getString()<<"' to the operator stack.";
                        operatorStack.append(data[i]);
                    }
                    else if(QString::compare(data[i]->getString(),"(")==0 || QString::compare(data[i]->getString(),"[")==0){
                        //qDebug()<<"Adding '"<<data[i]->getString()<<"' to the operator stack.";
                        operatorStack.append(data[i]);
                    }
                    else{
                        int topPrecedence = operatorStack.last()->operatorPrecedence();
                        int currentPrecedence = data[i]->operatorPrecedence();
                        bool isRightBracket = false;
                        if(QString::compare(data[i]->getString(),")")==0){
                            isRightBracket=true;
                        }
                        bool isRightSquareBracket = false;
                        if(QString::compare(data[i]->getString(),"]")==0){
                            isRightSquareBracket=true;
                        }
                        if(currentPrecedence<=topPrecedence){
                     //   if(currentPrecedence<topPrecedence){
                            do{
                                //qDebug()<<"Popping "<<operatorStack.last()->getString()<<" of operator stack.";
                               // currentPrecedence = operatorStack.last()->operatorPrecedence();
                                if(isRightBracket==true && QString::compare(operatorStack.last()->getString(),"(")==0){
                                    operatorStack.removeLast();
                                    if(operatorStack.size()>0 && operatorStack.last()->getType()==Token::TypeInvocationMarker){
                                        operandStack.append(operatorStack.last());
                                        operatorStack.removeLast();
                                    }
                                    topPrecedence=-1;
                                }
                                else if(isRightSquareBracket==true && QString::compare(operatorStack.last()->getString(),"[")==0){
                                    operatorStack.removeLast();
                                    if(operatorStack.last()->getType()==Token::TypeArrayLookUpMarker){
                                        operandStack.append(operatorStack.last());
                                        operatorStack.removeLast();
                                    }
                                    topPrecedence=-1;
                                }
                               /* else if(QString::compare(operatorStack.last()->getString(),",")==0 &&
                                        QString::compare(data[i]->getString(),",")==0){
                                    operandStack.append(operatorStack.last());
                                    operatorStack.removeLast();
                                 //   if(operatorStack.size()>0){
                                   //     topPrecedence = operatorStack.last()->operatorPrecedence();
                                  //  }
                                 //   else{
                                        topPrecedence=-1;
                                   // }
                                }*/
                                else{
                                    if(
                                            (QString::compare(operatorStack.last()->getString(),"(")!=0) &&
                                            (QString::compare(operatorStack.last()->getString(),")")!=0) &&
                                            (QString::compare(operatorStack.last()->getString(),"[")!=0) &&
                                            (QString::compare(operatorStack.last()->getString(),"]")!=0)
                                    ){
                                        operandStack.append(operatorStack.last());
                                    }
                                    operatorStack.removeLast();
                                    if(operatorStack.size()>0){
                                        topPrecedence = operatorStack.last()->operatorPrecedence();
                                    }
                                    else{
                                        topPrecedence=-1;
                                    }
                                }
                                //qDebug()<<"end of while...";
                        //    }while(currentPrecedence<topPrecedence);
                            }while(currentPrecedence<=topPrecedence);
                            //qDebug()<<"Adding '"<<data[i]->getString()<<"' to the operator stack.";
                            operatorStack.append(data[i]);
                        }
                        else{
                            //qDebug()<<"Adding '"<<data[i]->getString()<<"' to the operator stack.";
                            operatorStack.append(data[i]);
                        }
                    }
                }
         //   }
            else{
                //qDebug()<<"Adding '"<<data[i]->getString()<<"' to the operand stack.";
                operandStack.append(data[i]);
            }
            i++;
        }
        if(fatalError==false){
            res=true;
            i=operatorStack.size()-1;
            while(i>=0 && i<operatorStack.size()){
                //qDebug()<<operatorStack.last()->getString()<<" on top of stack.";
                if(QString::compare(operatorStack.last()->getString(),"(")==0 ||
                        QString::compare(operatorStack.last()->getString(),")")==0 ||
                        QString::compare(operatorStack.last()->getString(),"[")==0 ||
                        QString::compare(operatorStack.last()->getString(),"]")==0){
                //   res=false;
                }
                else{
                    //qDebug()<<operatorStack.last()->getString()<<" is NOT a bracket";
                    operandStack.append(operatorStack.last());
                }
                //qDebug()<<"removing from operatorStack:"<<operatorStack.last()->getString();
                operatorStack.removeLast();
                i--;
            }
            i=0;
            //qDebug()<<"Operand Stack:";
            while(i<operandStack.size()){
                //qDebug()<<"OS:"<<operandStack[i]->getString();
                i++;
            }
//            //qDebug()<<"And Res:"<<res;
            //qDebug()<<"Result of rpn: "<<operandStack;
            return operandStack;
        }
        else{
            i=0;
            //qDebug()<<"Operand Stack:";
            while(i<operandStack.size()){
                //qDebug()<<"OS:"<<operandStack[i]->getString();
                i++;
            }
            //qDebug()<<"Encountered Fatal Error in RPN conversion";
            return {};
        }
    }
    else{
        return {};
    }
}
