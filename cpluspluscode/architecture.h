#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H


class Architecture{
public: Architecture();
public: static constexpr int StandardStringSize=16;
public: static constexpr int StandardObjectSize=16;
public: static constexpr int StandardStackFrameSize=64;
public: static constexpr int RawIntegerSize=4;
public: static constexpr int NULLLocation=0;
public: static constexpr int UnsetLocation=1;
public: static constexpr int TrueLocation=2;
public: static constexpr int FalseLocation=3;
public: static constexpr int IsNaNLocation=4;
public: static constexpr int InfinityLocation=5;
public: static constexpr int ArrayInitLocation=6;
public: static constexpr int NativeLocation=7;
public: static constexpr int ArrayLocation=9;
public: static constexpr int LambdaLocation=13;
public: static constexpr int StringLocation=17;
public: static constexpr int ObjectLocation=21;//=16;
public: static constexpr int ReturnAddress=25;//26;//=17;
public: static constexpr int ThisAddress=29;//31;//22;

public: static constexpr int CurrentStackFrameAddress=33;//36;
public: static constexpr int ArgumentsAddress=37;//41;
public: static constexpr int EventStackAddress=41;//46;

public: static int SIZE_OF_INITIAL_HEAP_STORE;
public: static constexpr int RPNAddress=45;//46;//was 42
public: static constexpr char NativeStoreSignature=0x00;
public: static constexpr char NonNativeStoreSignature=0x01;
public: static constexpr char TypeLettedScope=0x0a;
public: static constexpr char TypeFixedScope=0x0b;
public: static constexpr char ReturnBufferCode='3';

//heap flush
public: static constexpr int TypeHeapGutterToFlush=5;
public: static constexpr int TypeHeapFlushGutter=1;
public: static constexpr int TypeHeapGutter=5+5;

public: static constexpr int INITIAL_HEAP_RESERVATION=1024*1024*10;

//Object Size Schema

//Objects - reference counted, passed by reference.
//copied and initialized when encountered.
public: static int constexpr TypeConstObjectGutter = 17;

public: static int constexpr TypeObjectGutter = 21;
public: static int constexpr TypeObjectGutterToLexId = 1;
public: static int constexpr TypeObjectGutterToProto = 5;
public: static int constexpr TypeObjectGutterToInit = 9;
public: static int constexpr TypeObjectGutterToSize = 13;
public: static int constexpr TypeObjectGutterToReferenceCount = 17;

public: static int constexpr TypeObjectPropertySize = 9;
public: static int constexpr TypeObjectPropertyKeyIndent = 1;
public: static int constexpr TypeObjectPropertyValueIndent = 5;

//LexRule - Internal type. Invalid for most operations, non-reference counted.
//Constant and GC Exempt.
public: static int constexpr TypeLexRuleSize = 9;
public: static int constexpr TypeLexRuleGutterToParent=5;
public: static int constexpr TypeLexRuleGutterToChild=1;


//Integer - primitive. Passed by value, copied from constant when encountered.
//reference counted.
public: static int constexpr TypeConstIntegerGutter = 1;
public: static int constexpr TypeConstIntegerSize = 5;

public: static int constexpr TypeRunTimeIntegerGutterToReferenceCount = 1;
public: static int constexpr TypeRunTimeIntegerGutter = 5;//1;
public: static int constexpr TypeRunTimeIntegerSize = 9;//5;

//Floats - same rules as integers
public: static int constexpr TypeConstFloatGutter=1;
public: static int constexpr TypeConstFloatSize=5;

public: static int constexpr TypeRunTimeFloatGutterToReferenceCount = 1;
public: static int constexpr TypeRunTimeFloatGutter=5;
public: static int constexpr TypeRunTimeFloatSize=9;
public: static int constexpr RawFloatSize=4;

//Strings

public: static int constexpr TypeStringGutterToSize = 1;//5;//1;
public: static int constexpr TypeConstStringGutter = 5;
public: static int constexpr TypeStringGutterToReferenceCount = 5;
public: static int constexpr TypeRunTimeStringGutter = 9;

//Arrays
public: static int constexpr TypeArrayGutterToSize=5;//=9;
public: static int constexpr TypeArrayGutterToCapacity=1;//=5;
public: static int constexpr TypeArrayGutterToReferenceCount=9;//13;
public: static int constexpr TypeArrayGutter=13;//17;

public: static int constexpr TypeArrayEntrySize=4;
public: static int constexpr TypeArrayDefaultCapacity = 2;
public: static int constexpr TypeArrayDefaultSize = 0;

//ArgumentsList- note - not reference counted.
public: static int constexpr TypeArgumentsListGutterToSize = 5;//1;
public: static int constexpr TypeArgumentsListGutterToCapacity = 1;//5;
public: static int constexpr TypeArgumentsListGutter = 9;
public: static int constexpr TypeArgumentsListEntrySize = 4;
public: static int constexpr TypeArgumentsListDefaultCapacity = 2;
public: static int constexpr TypeArgumentsListDefaultSize = 1;

//EventList - also not reference counted.

public: static int constexpr TypeEventListGutterToSize=1;
public: static int constexpr TypeEventListGutter=5;
public: static int constexpr TypeEventListEntrySize=4;

//Heap Fragments - internal types which exist only to point to an object which has had to be moved due to an increase in size
public: static int constexpr TypeHeapFragmentGutterToReferenceCount = 1;
public: static int constexpr TypeHeapFragmentGutter = 5;
public: static int constexpr TypeHeapFragmentSize = 9;

//PropertyKeys and ArrayIndexs
//not reference counted, generating for expression resolution only
public: static int constexpr TypePropertyKeyGutter = 5;
public: static int constexpr TypePropertyKeyGutterToContext = 1;
public: static int constexpr TypePropertyKeySize = 9;
public: static int constexpr TypeArrayIndexSize = 9;
public: static int constexpr TypeArrayIndexGutter=5;
public: static int constexpr TypeArrayIndexGutterToContext=1;

//Stackframes
public: static int constexpr TypeStackFrameGutterToParent=1;
public: static int constexpr TypeStackFrameGutterToLexId=5;
public: static int constexpr TypeStackFrameGutterToSize=9;
public: static int constexpr TypeStackFrameGutter=13;

public: static int constexpr TypeStackFrameEntrySize=8;
public: static int constexpr TypeStackFrameEntryGutter=4;

//events
    //moduleready
public: static int constexpr TypeModuleReadyEventGutterToId=1;
public: static int constexpr TypeModuleReadyEventGutterToCallBack=5;
public: static int constexpr TypeModuleReadyEventGutterToState=9;
    //filewritten
public: static int constexpr TypeFileWrittenEventGutterToCallBack=5;
public: static int constexpr TypeFileWrittenEventGutterToState=9;
    //timerevent
public: static int constexpr TypeTimerEventGutterToTime = 1;
public: static int constexpr TypeTimerEventGutterToCallBack = 5;

//finally instructions themselves
public: static int constexpr TypeInstructionSize=13;
public: static int constexpr TypeHeapFlushSize=5;
//public: static int constexpr TypeHeapFlushGutter=1;
public: static int constexpr TypeInstructionGutterToA=1;
public: static int constexpr TypeInstructionGutterToB=5;
public: static int constexpr TypeInstructionGutterToC=9;

//lambdas
public: static int constexpr TypeLambdaGutterToInstructionSet=5;
public: static int constexpr TypeLambdaGutterToLexId=1;
};

#endif // ARCHITECTURE_H
