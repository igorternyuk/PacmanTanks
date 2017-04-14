#ifndef TANK_H
#define TANK_H

#include "defines.h"

class Game;
class Trophy;
class Projectile;

class Tank
{
public:
    explicit Tank(TankType type, int x_r, int y_r, int speed, int health,
                  ALLEGRO_SAMPLE *shotSound, ALLEGRO_SAMPLE *explosionSound, Game *game);
    virtual ~Tank();
    TankType getType() const;
    int getX() const;
    int getY() const;
    int getSpeed() const;
    int getScore() const;
    void resetScore();
    bool isAlive() const;
    bool isBraked() const;
    Direction getDirection() const;
    int getHealth() const;
    int getTileSize() const;
    int getNumGatheredThrophies() const;
    void turn(Direction dir);
    void move();
    void reverse();
    void brake();
    bool detectTrophy(int trophy_X, int trophy_Y);
    void accelerate();
    void decelerate();
    void resetPosition();
    void fire(std::vector<Projectile*> &projectiles);
    void gatherTrophy(Trophy *trophy);
    void resetTrophiesAmount();
    bool isCollision(const Tank * const other);
    void generateRandDir();
    void chooseShortestWay(const Tank * const pacman);
    bool isOnCrossRoads();
    void explode();
    void repair();
    void draw();
    void drawFired();
    void printDir();
private:
    TankType type_;
    int initX_r, initY_r;
    int speed_;
    int lowGearSpeed_;
    int topGearSpeed_;
    int health_;
    int score_;
    //charMatrix map_;
    int sourceX_, sourceY_;
    ALLEGRO_SAMPLE *shotSound_;
    ALLEGRO_SAMPLE *explosionSound_;
    Game *game_;
    bool isAlive_ = true;
    ALLEGRO_BITMAP *sprite_;
    float imageAngle_;
    int tileSize_;
    int x_, y_;
    Direction dir_;
    int numGatheredThrophies_;
    int projectileSpeed_ = 10;
    int counter_ = 0;
    bool isBraked_ = false;
    bool isPointOnTheField(int x_a, int y_a);
    void prepareToTheNextBarrier(char nextPositionOnTheMap, bool &isPossibleToMove);
    bool isPossibleDir(const Direction &dir);
    void synchronizeImgAndDir();
    void playShotSound();
    void playExplosionSound();

};

#endif // TANK_H
