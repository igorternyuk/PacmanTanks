#pragma once

#include "utils.hpp"
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <memory>

class Tank;
class Trophy;
class Projectile;

class Game
{
public:
    explicit Game();
    ~Game();
    void run();
    charMatrix getMap();
    std::vector<std::unique_ptr<Projectile> > &getProjectiles();
    ALLEGRO_BITMAP *getBitmapProjectile() const;

private:
    enum
    {
        SCREEN_WIDTH  = 870,
        SCREEN_HEIGHT  = 680,
        WINDOW_LEFT = 200,
        WINDOW_TOP = 50
    };

    enum GameState
    {
        PLAYING,
        PAUSED,
        VICTORY,
        DEFEAT
    };

    const std::string WINDOW_TITLE = "Tanks";
    const std::string GAME_PAUSED_TEXT = "GAME PAUSED";
    const std::string WIN_MESSAGE_TEXT = "YOU WON!";
    const std::string LOST_MESSAGE_TEXT = "YOU LOST";
    const std::string PATH_TO_SETTINGS_FILE = "settings.txt";

    ALLEGRO_DISPLAY *display_;
    ALLEGRO_BITMAP *bitmapWall_;
    ALLEGRO_BITMAP *bitmapTank_;
    ALLEGRO_BITMAP *bitmapProjectile_;
    ALLEGRO_BITMAP *bitmapPacmanHealth_;

    ALLEGRO_FONT *smallFont_;
    ALLEGRO_FONT *middleFont_;
    ALLEGRO_FONT *largeFont_;
    ALLEGRO_SAMPLE *shotSound_;
    ALLEGRO_SAMPLE *explosion_;
    ALLEGRO_SAMPLE *backgroundSound_;
    ALLEGRO_SAMPLE_INSTANCE *backgroundInstance_;
    ALLEGRO_KEYBOARD_STATE keyState_;
    ALLEGRO_EVENT_QUEUE *eventQueue_;
    ALLEGRO_TIMER *pacTimer_;
    ALLEGRO_TIMER *enemyTimer_;
    ALLEGRO_TIMER *shotTimer_;
    ALLEGRO_TIMER *projectileTimer_;
    ALLEGRO_TIMER *newEnemyTimer_;
    ALLEGRO_TIMER *newTrophyTimer_;

    int initPacmanX_, initPacmanY_;
    int enemiesNumber_;
    int trophiesAmount_;
    charMatrix initialMap_;
    charMatrix map_;

    const float pacmanMovingSpeed = 10.0f;
    const float enemiesMovingSpeed = 5.0f;
    const float newEnemyFrequency = 30.0f;
    const float newTrophyFrequency = 20.0f;
    const float enemiesShootingSpeed = 2.0f;
    const float projectilesMovingSpeed = 20.0f;

    std::unique_ptr<Tank> pacman_;
    std::vector<std::unique_ptr<Trophy>> trophies_;
    std::vector<std::unique_ptr<Tank>> enemies_;
    std::vector<std::unique_ptr<Projectile>> projectiles_;

    bool render_ = true;
    bool isRunning_ = true;
    GameState gameState_ = GameState::PLAYING;

    void update();
    void enemiesTimerEvent();
    void projectilesTimerEvent();
    void shotTimerEvent();
    void handlePacmanCollisions();
    void checkWin();
    void render();

    void initAllegro5();
    void loadSettings(const std::string &fileName);
    void loadBitmaps();
    void loadSounds();
    void loadFonts();
    void createTimers();
    void destroyBitmaps();
    void destroyFonts();
    void destroySounds();
    void destroyTimers();
    void prepareNewGame();
    void createTrophies();
    void createTrophyOnRandPos();
    void createEnemies();
    void togglePause();
    void addEnemies();
    int  countAliveEnemies();
    void moveProjectiles();
    void resolveCollisions(Tank *tank);
    void startAllTimers();
    void stopAllTimers();
    void clearDestroyedEntitites();

    void renderProjectiles();
    void renderMap();
    void renderText(const int &textLeft, const int &textTop,
                    const std::string &text, ALLEGRO_COLOR color,
                    ALLEGRO_FONT *font);
    void renderPacmanScore() const;
    void renderPacmanHealth() const;
    void printEnemiesCoordinates() const;
    void printMap() const;
};
