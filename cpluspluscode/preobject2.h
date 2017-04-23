#ifndef PREOBJECT2_H
#define PREOBJECT2_H

#include "lexnode.h"

class LexNode;

class PreObject2{
public: PreObject2(LexNode *lexNode);
public: PByteArray* compile();
};

#endif // PREOBJECT2_H
