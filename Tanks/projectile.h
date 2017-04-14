#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "defines.h"

class Tank;
class Projectile
{
public:
    explicit Projectile(int x, int y, int speed, Direction dir);
    ~Projectile();
    int getX() const;
    int getY() const;
    Direction getDirection() const;
    int getTileSize() const;
    void move();
    void explode();
    bool isCollision(Tank *target);
    bool isActive() const;
    void draw() const;
private:
    int x_, y_;
    int speed_;
    Direction dir_;
    bool isActive_ = true;
    ALLEGRO_BITMAP *bitmap_;
    int tileSize_;
    float imageAngle_;
    mutable int counter_ = 0;
    void drawExplosion() const;
};

#endif // PROJECTILE_H
