#include "trophy.h"

Trophy::Trophy(int x, int y) :
tileSize_(30), x_(x * tileSize_), y_(y * tileSize_), isGathered_(false)
{
    //Генерируем случайным образом тип трофея
    int randNum = rand() % 100;
    if(randNum < 25)
        type_ = YELLOW;
    else if(randNum < 50)
        type_ = GREEN;
    else if(randNum < 75)
        type_ = BLUE;
    else
        type_ = RED;
}

int Trophy::getX() const
{
    return x_;
}

int Trophy::getY() const
{
    return y_;
}

TrophyType Trophy::getType() const
{
    return type_;
}

void Trophy::gather()
{
    isGathered_ = true;
}

bool Trophy::isGathered()
{
    return isGathered_;
}

void Trophy::draw()
{
    const int radius = 9;
    ALLEGRO_COLOR color;
    switch(type_)
    {
        case YELLOW :
            color = al_map_rgb(255, 255, 0);
            break;
        case GREEN:
            color = al_map_rgb(0, 200, 50);
            break;
        case  BLUE :
            color = al_map_rgb(70, 80, 255);
            break;
        case RED :
            color = al_map_rgb(255,50,0);
            break;
    }

    if(!isGathered_)
        al_draw_filled_ellipse(x_ + tileSize_ / 2, y_ + tileSize_ / 2, radius, radius, color);
}
