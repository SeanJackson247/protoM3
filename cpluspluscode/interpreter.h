#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <QString>
#include <QObject>

//#include "terminal.h"
#include "guirequest.h"
#include "heap.h"
#include "pipe.h"
#include "nativestore.h"
#include "pbytearray.h"
#include "alu.h"
#include "eru.h"
#include "nru.h"

#include "pipemanager.h"
#include "widgetmanager.h"

class Heap;

class GUIRequest;

class NativeStore;

class ALU;
class ERU;
class NRU;

class WidgetManager;

class Interpreter:public QObject{
    Q_OBJECT
private: bool halt;
private: bool jumped;
private: bool failedLastComp;
//private: int lexicalID;
public: int getLexicalID();
private: PByteArray* contents;
public: Interpreter(int parent,int id);
//public: Terminal* DebugWindow;
public: Heap* heap;
public: NativeStore* nativeStore;
public: void launchDebugger();
public: static QVector<Interpreter*> runtimeInstances;
public: int parent;
public: int id;
public: static QVector<Pipe*> Pipes;
public: QVector<int> pipes;
public: void debuggerOut(QString string);
public: void debuggerOut(QString string,bool debug);
public: void executeRPN(PByteArray* contents,char buffer,int i);
public: void executeLambda(int lambdaAddress, int argumentsAddress, int contextAddress);
public: void executeNative(int argsAddress);
public: void executeInstructionSetFrom(int i);
public: void yield();
public: void checkEventStack();
public: ALU* alu;
public: int lineNumber;
public: ERU* eru;
public: NRU* nru;
public: QString threadName;
public: PipeManager* pipeManager;
public: WidgetManager* widgetManager;
public: void connectRuntimeInstances(int parentid,int childid,QString propertyKey,int objectAddress);
public: QStringList args;

public slots:
    void launch(QString filename, PByteArray contents);
signals:
    void guiRequest();
    void toPipeManagerSignal();
};

#endif // INTERPRETER_H
