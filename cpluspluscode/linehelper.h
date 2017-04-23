#ifndef LINEHELPER_H
#define LINEHELPER_H

#include <QVector>
#include "token.h"
#include "token2.h"

class LineHelper{
public: LineHelper();
public: static QVector<Token*> removeBrackets(QVector<Token*> data);
public: static int countOperators(QVector<Token*> data);
public: static bool hasDuplicateTokenTypes(QVector<Token*> data);
public: static QVector<Token*> insertInvocationMarkers(QVector<Token*> data);
public: static QVector<Token*> insertNOARGMarkers(QVector<Token*> data);
public: static QVector<Token*> insertDUDRights(QVector<Token*> data);
public: static QVector<Token*> insertDUDLefts(QVector<Token*> data);
public: static QVector<Token*> insertArrayMarkers(QVector<Token*> data);
public: static QVector<Token*> removeColons(QVector<Token*> data);
public: static QVector<Token*> removeLambdaArgs(QVector<Token*> data);

public: static QVector<Token2*> insertInvocationMarkers2(QVector<Token2*> data);
public: static QVector<Token2*> insertNOARGMarkers2(QVector<Token2*> data);
public: static QVector<Token2*> insertDUDRights2(QVector<Token2*> data);
public: static QVector<Token2*> insertDUDLefts2(QVector<Token2*> data);
public: static QVector<Token2*> insertArrayMarkers2(QVector<Token2*> data);
public: static QVector<Token2*> removeColons2(QVector<Token2*> data);
public: static QVector<Token2*> removeLambdaArgs2(QVector<Token2*> data);

public: static int countLeftBrackets(QVector<Token*> data);
public: static int countRightBrackets(QVector<Token*> data);
public: static int countRightSquareBrackets(QVector<Token*> data);
public: static int countLeftSquareBrackets(QVector<Token*> data);

public: static int countLeftBrackets2(QVector<Token2*> data);
public: static int countRightBrackets2(QVector<Token2*> data);
public: static int countRightSquareBrackets2(QVector<Token2*> data);
public: static int countLeftSquareBrackets2(QVector<Token2*> data);

//Here is the expression validation....
public: static bool isValidPostFix(QVector<Token*> data);
public: static QVector<Token*> convertToPostFix(QVector<Token*> data);
public: static QVector<Token2*> convertToPostFix2(QVector<Token2*> data);
public: static bool isValidPostFix2(QVector<Token2*> data);
/*public: static QVector<Token2*> insertInvocationMarkers2(QVector<Token2*> data);
public: static QVector<Token2*> insertNOARGMarkers2(QVector<Token2*> data);*/
};

#endif // LINEHELPER_H
