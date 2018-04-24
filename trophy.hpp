#pragma once
#include "utils.hpp"

class Trophy
{
public:
    enum class Type {YELLOW = 100, GREEN = 300, BLUE = 500, RED = 1000};
    Trophy(int x, int y);
    int getX() const;
    int getY() const;
    Type getType() const;
    void gather();
    bool isGathered();
    void render();
private:
    int tileSize_;
    int x_, y_; // Координаты не в пикселях, а в тайлах!
    Type type_;
    bool isGathered_;
};

