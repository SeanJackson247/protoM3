#ifndef LINESTATIC_H
#define LINESTATIC_H

#include <QVector>

#include "line.h"
#include "lexnode.h"

class LineStatic{
public: LineStatic();
public: static QVector<ByteCodeInstruction*> compile(QVector<Line*> lines);
public: static QVector<Line*> validateAsFirstOrderLines(int indent,QVector<Line*> lines);
public: static QVector<Line*> validateAsClassPropertySignatures(int indent,QVector<Line*> lines);
public: static QVector<Line*> validateAsLiteralPropertySignatures(int indent,QVector<Line*> lines);
public: static QVector<Line*> removeEmptyLines(QVector<Line*> lines);
public: static QVector<Line*> flushToHeapStore(QVector<Line*> lines);
public:static QVector<Line*> labelLexicalScopes(QVector<Line*> lines);
public: static QVector<QString> getIncludesURLS(QVector<Line*> lines);
public: static QVector<Line*> stripIncludes(QVector<Line*> lines);
public: static QVector<Line*> flattenControlFlows(QVector<Line*> lines);
public: static QVector<ByteCodeInstruction*> compileArguments(Line* parent, int childIndex, bool isMethod);
public: static LexNode *buildTree(QVector<QString> data);
};

#endif // LINESTATIC_H
