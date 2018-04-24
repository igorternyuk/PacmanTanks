#include "projectile.hpp"
#include "tank.hpp"
#include <cmath>

Projectile::Projectile(int x, int y, int speed, Direction dir, ALLEGRO_BITMAP *bitmap) :
    x_(x), y_(y), speed_(speed), dir_(dir), bitmap_(bitmap)
{
    const int numFrames = 2;
    tileSize_ = al_get_bitmap_width(bitmap_) / numFrames;
    switch(dir_)
    {
        case Direction::LEFT :
            imageAngle_ = -90.0f;
            break;
        case Direction::RIGHT :
            imageAngle_ = +90.0f;
            break;
        case Direction::UP :
            imageAngle_ = +0.0f;
            break;
        case Direction::DOWN :
            imageAngle_ = +180.0f;
            break;
    }
}

int Projectile::getX() const
{
    return x_;
}

int Projectile::getY() const
{
    return y_;
}

Direction Projectile::getDirection() const
{
    return dir_;
}

int Projectile::getTileSize() const
{
    return tileSize_;
}

void Projectile::move()
{
    switch(dir_)
    {
        case Direction::LEFT :
            x_ -= speed_;
            break;
        case Direction::RIGHT :
            x_ += speed_;
            break;
        case Direction::UP :
            y_ -= speed_;
            break;
        case Direction::DOWN :
            y_ += speed_;
            break;
    }

}

void Projectile::explode()
{
    //std::cout << "Взрываем снаряд!" << std::cout;
    isActive_ = false;
}

bool Projectile::isCollision(Tank *target)
{
    bool isInsideOfTheTank = x_ > target->getX() &&
                             x_ < target->getX() + target->getTileSize() &&
                             y_ > target->getY() &&
                             y_ < target->getY() + target->getTileSize();
    if(isInsideOfTheTank)
        explode();
    return isInsideOfTheTank;
}

bool Projectile::isActive() const
{
    return isActive_;
}

void Projectile::render() const
{
    if(isActive_)
    {
        al_draw_tinted_scaled_rotated_bitmap_region(bitmap_, 0, 0, tileSize_, tileSize_,
            al_map_rgb(255,255,254), tileSize_ / 2, tileSize_ / 2, x_ + tileSize_ / 2,
            y_ + tileSize_ / 2, 1.0f, 1.0f, imageAngle_ * M_PI / 180.0, 0);
    }
    else
        drawExplosion();
}

void Projectile::drawExplosion() const
{
    if(counter_ < 10)
    {
        al_draw_bitmap_region(bitmap_, tileSize_, 0, tileSize_, tileSize_, x_, y_, 0);
        ++counter_;
    }
}
