#ifndef LINE_H
#define LINE_H

#include <QString>
#include <QVector>
#include "lineblock.h"
#include "bytecodeinstruction.h"
//#include "heapstore.h"

class HeapStore;

class Line{
    //lines have an initial constructor...
public: Line(int lineNumber,QString data);
    //and a constructor for the result of concatenating two lines together
public:Line(Line* line1,Line* line2);
    //as well as a constructor for special lines...
public:Line(int type);
    //and for lexical markers
public:Line(int type,int marker,int parent);
    //as well as goto placeholders for conditional statements
public:Line(int type,int id);
    //and this one is for later on when building initialisers...
public:Line(QVector<Token*> data);
    //each line is composed of lineblocks...
public: QVector<LineBlock*> data;
private: bool explicitIndent;
private: int expectedIndent;
public: int lexicalID;
public: int lexicalChildMarker;
public: int lexicalParentMarker;
public: int goToID;
    //and may have children (which are other lines)...
public: QVector<Line*> childs;
public: QVector<Line*> getChilds();
public: void setExpectedIndentation(int newIndent);
    //utilities...
public:static Line* root;

public: QVector<ByteCodeInstruction*> _compile();
    //in order to process a vector of lines using the static methods above,
    //lines need a few instance methods.
    //each line has get indentation method
public: int getIndent();
    //and line number
public: int getLineNumber();
    //and can return a bool for whether or not it is valid complete
public: bool isValidComplete();
    //they also need to indicate the expected indentation of child lines.
public: int expectedIndentation();
    //and keep a track of whether or not they have been altered...
private: bool changed;
public: void setHasChanged(bool changed);
public: bool hasChanged();
public: QVector<LineBlock*> getData();
    //need to be able to fix up a line for concatenation process...
public: void fixNumberOfParentMarkers();
public: Line* getLastLeaf(int indent);
private:Line* parent;
public:Line* getParent();
public:void setParent(Line* parent);

public: QVector<int> getArgsForLambda(int lexicalID);
public: void _flattenControlFlows();
    //and perform validation all the way down...
public:void validateChilds();

    //each line needs to provide lexical tree information to the heap store

public: QVector<Line*> insertLexicalMarkers();
public:void setLexicalId(int id);
public:int getLexicalId();

    //There are non regular lines.
    //therefore there needs to be a system of differentating between regular
    //and non regular lines.
    //non regular lines serve as special nodes in the resulting tree
    //which indicate the relationships between other nodes.
    //first a type and gettype method...
public: int type;
public: int getType();
    //and now the types themselves:
public:static int constexpr TypeRegular=0;  //it doesnt mattter what these are as long as theses are all different
public:static int constexpr TypeLambdaParentMarker=1;
public:static int constexpr TypeIfParentMarker=2;
public:static int constexpr TypeElseIfParentMarker=3;
public:static int constexpr TypeElseParentMarker=4;
public:static int constexpr TypeWhileParentMarker=5;
public:static int constexpr TypeLiteralParentMarker=6;
public:static int constexpr TypeClassDefParentMarker=7;
public:static int constexpr TypeMethodParentMarker=8;
public:static int constexpr TypeConditionalParentMarker=9;
public:static int constexpr TypeRoot=10;
public:static int constexpr TypeLexicalMarker=11;
public:static int constexpr TypeGOTOMarker=12;
public:static int constexpr TypeGOTOLoopMarker=13;
public:static int constexpr TypeGOTOFCompMarker=14;

public:void _labelLexicalScopes();

public:void setComplexPointer(Line* child,int complexHeapAddress);
public:void setLambdaPointer(int complexHeapAddress);

    //finally a toString method
public: QString toString(int indent);
};

#endif // LINE_H
