#ifndef TROPHY_H
#define TROPHY_H

#include "defines.h"

class Trophy
{
public:
    Trophy(int x, int y);
    int getX() const;
    int getY() const;
    TrophyType getType() const;
    void gather();
    bool isGathered();
    void draw();
private:
    int tileSize_;
    int x_, y_; // Координаты не в пикселях, а в тайлах!
    TrophyType type_;
    bool isGathered_;
};

#endif // TROPHY_H
