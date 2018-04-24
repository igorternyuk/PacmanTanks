#include "tank.hpp"
#include "game.hpp"
#include "trophy.hpp"
#include "projectile.hpp"

#include <cmath>
#include <map>
#include <iostream>

Tank::Tank(Type type, int x_r, int y_r, int speed, int health, ALLEGRO_BITMAP *bitmap,
           ALLEGRO_SAMPLE *shotSound, ALLEGRO_SAMPLE *explosionSound, Game *game) :
    type_(type), initX_r(x_r), initY_r(y_r), speed_(speed), lowGearSpeed_(0.5 * speed),
    topGearSpeed_(speed_), health_(health), score_(0), sourceX_(0),
    sourceY_(static_cast<int>(type)), bitmap_(bitmap),shotSound_(shotSound),
    explosionSound_(explosionSound), game_(game)
{
    const int numFrames = 5;
    tileSize_ = al_get_bitmap_width(bitmap_) / numFrames;
    x_ = x_r * tileSize_;
    y_ = y_r * tileSize_;
    generateRandDir();
    synchronizeImgAndDir();
    numGatheredThrophies_ = 0;
}

Tank::Type Tank::getType() const
{
    return type_;
}

int Tank::getX() const
{
    return x_;
}

int Tank::getY() const
{
    return y_;
}

int Tank::getSpeed() const
{
    return speed_;
}

int Tank::getScore() const
{
    return score_;
}

void Tank::resetScore()
{
    score_ = 0;
}

bool Tank::isAlive() const
{
    return isAlive_;
}

bool Tank::isBraked() const
{
    return isBraked_;
}

Direction Tank::getDirection() const
{
    return dir_;
}

int Tank::getHealth() const
{
    return health_;
}

int Tank::getTileSize() const
{
    return tileSize_;
}

int Tank::getNumGatheredThrophies() const
{
    return numGatheredThrophies_;
}

void Tank::handleUserInput(ALLEGRO_KEYBOARD_STATE *keyState)
{
    if(type_ == Type::PACMAN)
    {
        if(al_key_down(keyState, ALLEGRO_KEY_LEFT) ||
           al_key_down(keyState, ALLEGRO_KEY_A))
        {
            turn(Direction::LEFT);
            move();
        }
        else if(al_key_down(keyState, ALLEGRO_KEY_RIGHT) ||
                al_key_down(keyState, ALLEGRO_KEY_D))
        {
            turn(Direction::RIGHT);
            move();
        }
        else if(al_key_down(keyState, ALLEGRO_KEY_UP) ||
                al_key_down(keyState, ALLEGRO_KEY_W))
        {
            turn(Direction::UP);
            move();
        }
        else if(al_key_down(keyState, ALLEGRO_KEY_DOWN) ||
                al_key_down(keyState, ALLEGRO_KEY_S))
        {
            turn(Direction::DOWN);
            move();
        }
    }
}

void Tank::turn(Direction dir)
{
    if(isOnCrossRoads())
    {
        dir_ = dir;
        synchronizeImgAndDir();
    }
}

void Tank::reverse()
{
    switch(dir_)
    {
        case Direction::LEFT:
            dir_ = Direction::RIGHT;
            break;
        case Direction::RIGHT:
            dir_ = Direction::LEFT;
            break;
        case Direction::UP:
            dir_ = Direction::DOWN;
            break;
        case Direction::DOWN:
            dir_ = Direction::UP;
            break;
    }
    synchronizeImgAndDir();
}

void Tank::playShotSound()
{
    if(shotSound_ != nullptr)
        al_play_sample(shotSound_, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
}

void Tank::playExplosionSound()
{
    if(explosionSound_ != nullptr)
        al_play_sample(explosionSound_, 1.0f, 0.0f, 1.0, ALLEGRO_PLAYMODE_ONCE, 0);
}

void Tank::move()
{
    charMatrix map_ = game_->getMap();
    if(!isAlive())
        return;
    int bottomLimit = map_.size() * tileSize_, rightLimit;
    bool isMovePossible = true;
    switch(dir_)
    {
        case Direction::UP :
            if(y_ >= tileSize_)
            {
                if(y_ % tileSize_ == 0)
                    prepareToTheNextBarrier(map_[y_ / tileSize_ - 1][x_ / tileSize_], isMovePossible);
                else
                    isMovePossible = true;
            }
            else
                isMovePossible = true;
            if(isMovePossible)
            {
                y_ -= speed_;
                if(y_ <= -tileSize_)
                    y_ = map_.size() * tileSize_;
            }
            break;
        case Direction::DOWN :
            if(y_ < int((map_.size() - 1) * tileSize_))
            {
                if(y_ % tileSize_ == 0)
                    prepareToTheNextBarrier(map_[y_ / tileSize_ + 1][x_ / tileSize_], isMovePossible);
                else
                    isMovePossible = true;
            }
            else
                isMovePossible = true;
            if(isMovePossible)
            {
                y_ += speed_;
                if(y_ >= bottomLimit)
                    y_ = -tileSize_;
            }
            break;
        case Direction::LEFT :
            if(x_ >= tileSize_)
            {
                if(x_ % tileSize_ == 0)
                    prepareToTheNextBarrier(map_[y_ / tileSize_][x_ / tileSize_ - 1], isMovePossible);
                else
                    isMovePossible = true;
            }
            else
                isMovePossible = true;
            if(isMovePossible)
            {
                x_ -= speed_;
                if(x_ <= -tileSize_)
                    x_ = map_[y_ / tileSize_].size() * tileSize_;
            }
            break;
        case Direction::RIGHT :
            if(x_ < (map_[y_ / tileSize_].size() - 1) * tileSize_ )
            {
                if(x_ % tileSize_ == 0)
                    prepareToTheNextBarrier(map_[y_ / tileSize_][x_ / tileSize_ + 1], isMovePossible);
                else
                    isMovePossible = true;
            }
            else
                isMovePossible = true;
            if(isMovePossible)
            {
                x_ += speed_;
                rightLimit = map_[y_ / tileSize_].size() * tileSize_;
                if(x_ >= rightLimit)
                    x_ = -tileSize_;
            }
            break;
    }
    ++sourceX_;
    if(sourceX_ >= 4)
        sourceX_ = 0;
    synchronizeImgAndDir();
}

void Tank::brake()
{
    speed_ = 0;
    isBraked_ = true;
}

bool Tank::detectTrophy(int trophy_X, int trophy_Y)
{
    return x_ == trophy_X && y_ == trophy_Y;
}

bool Tank::isPossibleDir(const Direction &dir)
{
    bool result;
    charMatrix map_ = game_->getMap();
    if(isPointOnTheField(x_, y_))
    {
        int x_r = x_ / tileSize_;
        int y_r = y_ / tileSize_;
        switch(dir)
        {
            case Direction::LEFT :
                result = (x_r > 0)
                        ? (map_[y_r][x_r - 1] != 'X'
                          && map_[y_r][x_r - 1] != 'B'
                          && map_[y_r][x_r - 1] != 'C')
                        : true;
                break;
            case Direction::RIGHT :
                result = (x_r < map_[y_r].size() - 1)
                         ? (map_[y_r][x_r + 1] != 'X'
                           && map_[y_r][x_r + 1] != 'B'
                           && map_[y_r][x_r + 1] != 'C')
                         : true;
                break;
            case Direction::UP:
                result = (y_r > 0)
                        ? (map_[y_r - 1][x_r] != 'X'
                           && map_[y_r - 1][x_r] != 'B'
                           && map_[y_r - 1][x_r] != 'C')
                        : true;
                break;
            case Direction::DOWN :
                result = (y_r < map_.size() - 1)
                         ? (map_[y_r + 1][x_r] != 'X'
                            && map_[y_r + 1][x_r] != 'B'
                            && map_[y_r + 1][x_r] != 'C')
                         : true;
                break;
        }
    }
    else
        result = false;
    return result;
}

void Tank::synchronizeImgAndDir()
{
    switch(dir_)
    {
        case Direction::UP :
            imageAngle_ = 0.0f;
            break;
        case Direction::DOWN:
            imageAngle_ = 180.0f;
            break;
        case Direction::LEFT :
            imageAngle_ = -90.0f;
            break;
        case Direction::RIGHT :
            imageAngle_ = 90.0f;
            break;
    }
}

void Tank::generateRandDir()
{
    if(isPointOnTheField(x_, y_))
    {
        Direction result;
        do
        {
            result = static_cast<Direction>(rand() % 4);
        }while(!isPossibleDir(result));
        dir_ = result;
        synchronizeImgAndDir();
    }
}

float Tank::calcDistanceToTheTarget(Direction dir, const Tank * const target)
{
    int targetX = target->getX();
    int targetY = target->getY();
    switch(dir)
    {
        case Direction::LEFT:
            return (x_ - speed_ -  targetX) * (x_ - speed_ - targetX) +
                    (y_ - targetY) * (y_ - targetY);
        case Direction::RIGHT:
            return (x_ + speed_ -  targetX) * (x_ + speed_ - targetX) +
                    (y_ - targetY) * (y_ - targetY);
        case Direction::UP:
            return (x_ - targetX) * (x_ - targetX) +
                    (y_ - speed_ - targetY) * (y_ - speed_ - targetY);
        case Direction::DOWN:
            return (x_ - targetX) * (x_ - targetX) +
                    (y_ + speed_ - targetY) * (y_ + speed_ - targetY);
    }
}

void Tank::chooseShortestWay(const Tank * const pacman)
{
    std::multimap<float, Direction> distDirMap;
    Direction allDirs[] = { Direction::LEFT, Direction::RIGHT, Direction::UP,
                            Direction::DOWN };
    for(auto dir: allDirs)
    {
        if(isPossibleDir(dir))
        {
            float dist = calcDistanceToTheTarget(dir, pacman);
            distDirMap.insert(std::make_pair(dist, dir));
        }
    }
    dir_ = distDirMap.begin()->second;
    synchronizeImgAndDir();
}

bool Tank::isOnCrossRoads()
{
    bool result;
    charMatrix map_ = game_->getMap();
    if(isPointOnTheField(x_, y_))
    {
        bool isXDivisible = x_ % tileSize_ == 0;
        bool isYDivisible = y_ % tileSize_ == 0;
        result = isXDivisible && isYDivisible;
    }
    else
        result = false;
    return result;
}

bool Tank::isCollision(const Tank * const other)
{
    bool result;
    int dx = abs(x_ - other->getX());
    int dy = abs(y_ - other->getY());
    if(this->isAlive() && other->isAlive() && other != this)
        result = dx < tileSize_ && dy < tileSize_;
    else
        result = false;
    return result;
}

void Tank::accelerate()
{
    isBraked_ = false;
    speed_ = topGearSpeed_;
}

void Tank::decelerate()
{
    speed_ = lowGearSpeed_;
}

void Tank::resetPosition()
{
    x_ = initX_r * tileSize_;
    y_ = initY_r * tileSize_;
    sourceX_ = 0;
    sourceY_ = static_cast<int>(type_);
}

void Tank::fire()
{
    std::vector<std::unique_ptr<Projectile>> &projectiles = game_->getProjectiles();
    if(isAlive_)
    {
        int px, py;
        switch (dir_) {
            case Direction::LEFT:
                px = x_ - 10; // Убрать магические десятки задающие положение снаряда отностильно танка
                py = y_ + 10;
                break;
            case Direction::RIGHT:
                px = x_ + tileSize_ + 10;
                py = y_ + 10;
                break;
            case Direction::UP:
                px = x_ + 10 ;
                py = y_ + 10;
                break;
            case Direction::DOWN:
                px = x_ + 10;
                py = y_ + tileSize_ + 10;
                break;
        }
        auto projectile = std::make_unique<Projectile>(px, py, projectileSpeed_,
                               dir_, game_->getBitmapProjectile());
        projectiles.push_back(std::move(projectile));
    }
    playShotSound();
}

void Tank::gatherTrophy(Trophy * trophy)
{
    trophy->gather();
    score_ += static_cast<int>(trophy->getType());
    // Green trophies increase pacman health
    if(trophy->getType() == Trophy::Type::GREEN)
        health_ = 100;
    else if(trophy->getType() == Trophy::Type::BLUE && health_ < 50)
    {
        health_ = 50;
    }
    ++numGatheredThrophies_;
}

void Tank::resetTrophiesAmount()
{
   numGatheredThrophies_ = 0;
}

void Tank::explode()
{
    switch (type_)
    {
        case Type::ENEMY:
        case Type::PACMAN:
            health_ -= 25; // 4 shots to kill
            //--speed_;
            break;
        case Type::HUNTER:
            health_ -= 20; // 5 shots to kill
            //--speed_;
            break;
        case Type::FANTOM:
            health_ -= 10; // 10 shots to kill
            //--speed_;
            break;
    }
    if(health_ <= 0)
        isAlive_ = false;
    playExplosionSound();
}

void Tank::repair()
{
    isAlive_ = true;
    health_ = 100;
}

void Tank::draw()
{
    if(isAlive_)
    {
        al_draw_tinted_scaled_rotated_bitmap_region(bitmap_, sourceX_ * tileSize_,
        sourceY_ * tileSize_, tileSize_, tileSize_, al_map_rgb(255,255,254), tileSize_ / 2,
        tileSize_ / 2, x_ + tileSize_ / 2, y_ + tileSize_ / 2, 1.0f, 1.0f,
        imageAngle_ * M_PI / 180, 0);
    }
    else
    {
        drawFired();
    }
}

void Tank::drawFired()
{
    if(counter_ < 20)
    {
        sourceY_ = 4;
        al_draw_tinted_scaled_rotated_bitmap_region(bitmap_, sourceX_ * tileSize_,
        sourceY_ * tileSize_, tileSize_, tileSize_, al_map_rgb(255,255,254), tileSize_ / 2,
        tileSize_ / 2, x_ + tileSize_ / 2, y_ + tileSize_ / 2, 1.0f, 1.0f,
        imageAngle_ * M_PI / 180, 0);
        ++sourceX_;
        if(sourceX_ >= 4)
            sourceX_ = 0;
        ++counter_;
    }
}

void Tank::printDir()
{
    switch (dir_) {
        case Direction::LEFT:
            std::cout << "Dir = LEFT" << std::endl;
            break;
        case Direction::RIGHT:
            std::cout << "Dir = RIGHT" << std::endl;
            break;
        case Direction::UP:
            std::cout << "Dir = UP" << std::endl;
            break;
        case Direction::DOWN:
            std::cout << "Dir = DOWN" << std::endl;
            break;
    }
}

bool Tank:: isPointOnTheField(int x_a, int y_a)
{
    charMatrix map_ = game_->getMap();
    int x_r = x_a / tileSize_; //25
    int y_r = y_a / tileSize_; //17
    bool result = true;
    if(y_r < 0 || y_r > map_.size() - 1)
        result = false;
    else if(x_r < 0 || x_r > map_[y_r].size() - 1)
             result = false;
    return result;
}

void Tank::prepareToTheNextBarrier(char nextPositionOnTheMap, bool &isPossibleToMove)
{
    switch(nextPositionOnTheMap)
    {
        case 'X' :
        case 'B' :
        case 'C' :
            isPossibleToMove = false;
            break;
        case 'W' :
            decelerate();
            isPossibleToMove = true;
            break;
        case '+' :
        case ' ':
        default:
            accelerate();
            isPossibleToMove = true;
            break;
    }
}
