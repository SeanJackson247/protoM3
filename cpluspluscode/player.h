#ifndef PLAYER_H
#define PLAYER_H

#include "pdrawable.h"

#include <QVector>

class PDrawable;

class PLayer{
public: PLayer();
public: QVector<PDrawable*> drawables;
public: void drawOn(PWidget* context);
};

#endif // PLAYER_H
