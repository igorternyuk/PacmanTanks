#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include "utils.hpp"

class Game;
class Trophy;
class Projectile;

class Tank
{
public:
    enum class Type { PACMAN, ENEMY, HUNTER, FANTOM };
    explicit Tank(Type type, int x_r, int y_r, int speed, int health,
                  ALLEGRO_BITMAP *sprite, ALLEGRO_SAMPLE *shotSound,
                  ALLEGRO_SAMPLE *explosionSound, Game *game);
    virtual ~Tank() {}
    Type getType() const;
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

    void handleUserInput(ALLEGRO_KEYBOARD_STATE *keyState);
    void turn(Direction dir);
    void move();
    void reverse();
    void brake();
    bool detectTrophy(int trophy_X, int trophy_Y);
    void accelerate();
    void decelerate();
    void resetPosition();
    void fire();
    void gatherTrophy(Trophy *trophy);
    void resetTrophiesAmount();
    bool isCollision(const Tank * const other);
    void generateRandDir();
    float calcDistanceToTheTarget(Direction dir, const Tank *const target);
    void chooseShortestWay(const Tank * const pacman);
    bool isOnCrossRoads();
    void explode();
    void repair();
    void render();
    void renderFired();
    void printDir();
    bool isBurned() const;

private:
    Tank::Type type_;
    int initX_r, initY_r;
    int speed_;
    int lowGearSpeed_;
    int topGearSpeed_;
    int health_;
    int score_;
    int sourceX_, sourceY_;
    ALLEGRO_BITMAP *bitmap_;
    ALLEGRO_SAMPLE *shotSound_;
    ALLEGRO_SAMPLE *explosionSound_;
    Game *game_;
    bool isAlive_ = true;    
    float imageAngle_;
    int tileSize_;
    int x_, y_;
    Direction dir_;
    int numGatheredThrophies_;
    int projectileSpeed_ = 10;
    int counter_ = 0;
    bool isBraked_ = false;
    bool isBurned_ = false;

    bool isPointOnTheField(int x_a, int y_a);
    void prepareToTheNextBarrier(char nextPositionOnTheMap, bool &isPossibleToMove);
    bool isPossibleDir(const Direction &dir);
    void synchronizeImgAndDir();
    void playShotSound();
    void playExplosionSound();

};
