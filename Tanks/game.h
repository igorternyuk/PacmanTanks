#ifndef GAME_H
#define GAME_H

#include "defines.h"

class Tank;
class Trophy;
class Projectile;
class Game
{
public:
    explicit Game();
    ~Game();
    void go();
    charMatrix getMap();
private:
    const int SCREEN_WIDTH  = 870;
    const int SCREEN_HEIGHT  = 680;
    const int WINDOW_LEFT = 200;
    const int WINDOW_TOP = 50;
    const std::string WINDOW_TITLE = "Tanks";
    const std::string GAME_PAUSED_TEXT = "GAME PAUSED";
    const std::string WIN_MESSAGE_TEXT = "YOU WON!";
    const std::string LOST_MESSAGE_TEXT = "YOU LOST";
    ALLEGRO_DISPLAY *display_;
    int initPacmanX_, initPacmanY_;
    int enemiesNumber_;
    int trophiesAmount_;
    ALLEGRO_BITMAP *wall_;
    ALLEGRO_BITMAP *pacmanHealthBitmap_;
    charMatrix initialMap_;
    charMatrix map_;
    ALLEGRO_FONT *smallFont_;
    ALLEGRO_FONT *middleFont_;
    ALLEGRO_FONT *largeFont_;
    ALLEGRO_SAMPLE *shotSound_;
    ALLEGRO_SAMPLE *explosion_;
    ALLEGRO_SAMPLE *backgroundSound_;
    ALLEGRO_SAMPLE_INSTANCE *backgroundInstance_;
    ALLEGRO_TIMER *pacTimer_;
    ALLEGRO_TIMER *enemyTimer_;
    ALLEGRO_TIMER *shotTimer_;
    ALLEGRO_TIMER *projectileTimer_;
    ALLEGRO_TIMER *newEnemyTimer_;
    ALLEGRO_TIMER *newTrophyTimer_;
    Tank *pacman_;
    std::vector<Trophy*> trophies_;
    std::vector<Tank*> enemies_;
    std::vector<Projectile*> projectiles_;
    bool done_ = false;
    bool isWin_ = false;
    bool isGameOver_ = false;
    bool isGamePaused_ = false;
    void init();
    bool loadSettings(const std::string &fileName);
    void prepareNewGame();
    void createTrophies();
    void createTrophyOnRandPos();
    void createEnemies();
    void addEnemies();
    int  countAliveEnemies();
    void moveProjectiles();
    void resolveCollisions(Tank *&tank);
    void startAllTimers();
    void stopAllTimers();
    void enemiesTimerEvent();
    void projectilesTimerEvent();
    void shotTimerEvent();
    void drawProjectiles();
    void drawMap();
    void drawText(const int &textLeft, const int &textTop, const std::string &text,
                  ALLEGRO_COLOR color, ALLEGRO_FONT *font);
    void drawPacmanScore() const;
    void drawPacmanHealth() const;
    void printEnemiesCoordinates() const;
    void printMap() const;
};

#endif // GAME_H
