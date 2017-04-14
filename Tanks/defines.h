#ifndef DEFINES_H
#define DEFINES_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>


enum Direction {LEFT, RIGHT, UP, DOWN};
enum TankType {PACMAN, ENEMY, HUNTER, FANTOM};
enum TrophyType {YELLOW = 100, GREEN = 300, BLUE = 500, RED = 1000};
typedef std::vector<std::string> charMatrix;

#endif // DEFINES_H
