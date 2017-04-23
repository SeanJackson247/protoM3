#ifndef LEXNODE_H
#define LEXNODE_H

#include <QVector>
#include "token2.h"
#include "bytecodeinstruction2.h"
#include "preobject2.h"

class ByteCodeInstruction2;

class PreObject2;

class LexNode{
public: LexNode(QVector<bool>isCode, QVector<QString> byString);
public: void appendCode(QString _data);
public: void appendString(QString _data);
public: void appendChild(QString _data);
public: QVector<Token2*> tokens;
public: QVector<LexNode*> childs;
public: QVector<QVector<Token2*>> data;
public: int openBrackets;
public: int closeBrackets;
public: int lexId;
public: int address;
public: void setAsAddress(int index);
public: QVector<QString> getIncludeURLs();
public: QVector<QString> tempChilds;
public: QVector<bool> tempChildTypes;
public: QString toString(int indent);
public: QVector<ByteCodeInstruction2*> toByteCode();
public: PByteArray* preHeapByteCode(bool top, PByteArray *res);
public: PByteArray* compile(bool top);
public: void setAsClass();
public: bool _class;
public: void setAsLiteral();
public: bool _literal;
public: void stripIncludes();
public: PByteArray* byteCode(bool top);
public: void compileToTokenStream(bool top);
public: void prepend(LexNode* child);
public: QVector<ByteCodeInstruction2*> precompile(bool top);
public: static int HighestRPN;
public: static int getFreshLexicalId();
public: static int freshLexicalId;
public: static QVector<Token2*> encounteredKeys;
public: static QVector<Token2*> encounteredStrings;
public: static QVector<Token2*> encounteredIntegers;
public: QVector<QVector<Token2*>> reorderTokenStream(bool isLambda, bool isIfStatement, bool isWhileStatement, bool isLiteralStatement, int &lexicalId, QVector<QString> args);
//public: QVector<QVector<Token2*>> preinsts;
public: QVector<ByteCodeInstruction2*> instructionSet;
public: PByteArray* getLexicalRules();
public: static int freshConditionalId;
public: static int getFreshConditionalId();

public: static QVector<LexNode*> LiteralStore;
public: PreObject2* toPreObject();
};

#endif // LEXNODE_H
