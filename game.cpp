#include "game.hpp"
#include "tank.hpp"
#include "trophy.hpp"
#include "projectile.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

Game::Game()
{
    initAllegro5();
    loadSettings(PATH_TO_SETTINGS_FILE);
    loadBitmaps();
    loadFonts();
    loadSounds();
    createTimers();

    eventQueue_ = al_create_event_queue();

    al_register_event_source(eventQueue_, al_get_timer_event_source(pacTimer_));
    al_register_event_source(eventQueue_, al_get_timer_event_source(enemyTimer_));
    al_register_event_source(eventQueue_, al_get_timer_event_source(newEnemyTimer_));
    al_register_event_source(eventQueue_, al_get_timer_event_source(projectileTimer_));
    al_register_event_source(eventQueue_, al_get_timer_event_source(shotTimer_));
    al_register_event_source(eventQueue_, al_get_display_event_source(display_));
    al_register_event_source(eventQueue_, al_get_keyboard_event_source());

    //Create pacman
    pacman_ = std::make_unique<Tank>(Tank::Type::PACMAN, initPacmanX_, initPacmanY_, 6,
                       100, bitmapTank_, shotSound_, explosion_, this);
    //Create trophies
    createTrophies();

    //Create enemies
    createEnemies();
}

Game::~Game()
{
    al_destroy_event_queue(eventQueue_);
    destroyTimers();
    destroySounds();
    destroyFonts();
    destroyBitmaps();
    al_destroy_display(display_);
}

void Game::run()
{
    startAllTimers();
    al_play_sample_instance(backgroundInstance_);

    //Game loop
    while (isRunning_)
    {
        update();
        render();
    }
}

void Game::update()
{
    ALLEGRO_EVENT events;
    al_wait_for_event(eventQueue_, &events);
    al_get_keyboard_state(&keyState_);
    render_ = true;
    if(events.type == ALLEGRO_EVENT_KEY_UP)
    {
        if(events.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
        {
            isRunning_ = false;
        }
        else if(events.keyboard.keycode == ALLEGRO_KEY_SPACE)
        {
            togglePause();
        }
        else if(events.keyboard.keycode == ALLEGRO_KEY_N)
        {
            prepareNewGame();
            startAllTimers();
            al_play_sample_instance(backgroundInstance_);
        }
        else if(events.keyboard.keycode == ALLEGRO_KEY_F)
        {
            if(gameState_ == GameState::PLAYING)
            {
                pacman_->fire();
                render_ = true;
            }
        }
        else if(events.keyboard.keycode == ALLEGRO_KEY_P)
        {
            printEnemiesCoordinates();
        }
        else if(events.keyboard.keycode == ALLEGRO_KEY_K)
        {
            printMap();
        }
    }
    else if(events.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
    {
        isRunning_ = false;
    }
    else if(events.type == ALLEGRO_EVENT_TIMER)
    {
        if(events.timer.source == projectileTimer_)
        {
            projectilesTimerEvent();
            render_ = true;
        }
        else if(events.timer.source == pacTimer_)
        {
            pacman_->handleUserInput(&keyState_);
            handlePacmanCollisions();
            checkWin();
        }
        else if(events.timer.source == enemyTimer_)
        {
            enemiesTimerEvent();
        }
        else if(events.timer.source == shotTimer_)
        {
            shotTimerEvent();
        }
        else if(events.timer.source == newEnemyTimer_)
        {
            //addEnemies();
        }
        else if(events.timer.source == newTrophyTimer_)
        {
            //createTrophyOnRandPos();
        }
        auto trophy_iterator = std::remove_if(trophies_.begin(), trophies_.end(),
                               [](auto &trophy)
                                {
                                     return trophy->isGathered();
                                });
        trophies_.erase(trophy_iterator, trophies_.end());

       /* auto enemy_iterator = std::remove_if(enemies_.begin(), enemies_.end(),
                               [](auto &enemy)
                                {
                                    return !enemy->isAlive();
                                });
        enemies_.erase(enemy_iterator, enemies_.end());
        if we erase enemy immediately it will not be burning
        */

        auto projectile_iterator = std::remove_if(projectiles_.begin(), projectiles_.end(),
                               [](auto &projectile)
                                {
                                       return !projectile->isActive();
                                });
        projectiles_.erase(projectile_iterator, projectiles_.end());

        if(gameState_ == GameState::VICTORY ||
           gameState_ == GameState::DEFEAT)
        {
            stopAllTimers();
            al_stop_sample_instance(backgroundInstance_);
        }
        render_ = true;
    }
}

void Game::render()
{
    if(render_)
    {
        renderMap();
        renderProjectiles();
        for(auto &t : trophies_)
            t->draw();
        pacman_->draw();
        for(auto &e : enemies_)
            e->draw();
        renderPacmanScore();
        renderPacmanHealth();
        if(gameState_ == GameState::PAUSED)
            renderText(SCREEN_WIDTH / 2 - 400, SCREEN_HEIGHT / 2 - 100,
                     GAME_PAUSED_TEXT, al_map_rgb(255, 255, 255), largeFont_);
        else if(gameState_ == GameState::VICTORY)
            renderText(SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 100,
                     WIN_MESSAGE_TEXT, al_map_rgb(255, 255, 0), largeFont_);
        else if(gameState_ == GameState::DEFEAT)
            renderText(SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 100,
                     LOST_MESSAGE_TEXT, al_map_rgb(255, 255, 255), largeFont_);
        al_flip_display();
        al_clear_to_color(al_map_rgb(0, 0, 0));
    }
}

charMatrix Game::getMap()
{
    return map_;
}

std::vector<std::unique_ptr<Projectile>> &Game::getProjectiles()
{
    return projectiles_;
}

ALLEGRO_BITMAP *Game::getBitmapProjectile() const
{
    return bitmapProjectile_;
}

void Game::initAllegro5()
{
    if(!al_init())
    {
        throw std::runtime_error("Could not initialize Allegro 5");
    }

    display_ = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);

    if(!display_)
    {
        throw std::runtime_error("Could not create Allegro Window");
    }

    al_set_window_position(display_, WINDOW_LEFT, WINDOW_TOP);
    al_set_window_title(display_, WINDOW_TITLE.c_str());
    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_init_primitives_addon();
    al_install_audio();
    al_init_acodec_addon();
    al_install_keyboard();
    al_install_mouse();
}

void Game::loadSettings(const std::string &fileName)
{
    enum LoadState {INIT_PACMAN_X, INIT_PACMAN_Y, ENEMIES_NUMBER, TROPHIES_AMOUNT,
                   TILE_SET, MAP};
    int state = 0;
    std::ifstream fi(fileName);
    if(!map_.empty())
        map_.clear();
    if(fi.is_open())
    {
        while(!fi.eof())
        {
            std::string line;
            std::getline(fi, line);
            if(line.length() == 0)
                continue;
            if(line == "[InitialPacmanX]")
            {
                state = INIT_PACMAN_X;
                continue;
            }
            else if(line == "[InitialPacmanY]")
            {
                state = INIT_PACMAN_Y;
                continue;
            }
            else if(line == "[EnemiesNumber]")
            {
                state = ENEMIES_NUMBER;
                continue;
            }
            else if(line == "[TrophiesAmount]")
            {
                state = TROPHIES_AMOUNT;
                continue;
            }
            else if(line == "[TileSet]")
            {
                state = TILE_SET;
                continue;
            }
            else if(line == "[Map]")
            {
                state = MAP;
                continue;
            }
            std::stringstream stream;
            switch(state)
            {
                case INIT_PACMAN_X :
                    stream << line;
                    stream >> initPacmanX_;
                    break;
                case INIT_PACMAN_Y :
                    stream << line;
                    stream >> initPacmanY_;
                    break;
                case ENEMIES_NUMBER :
                    stream << line;
                    stream >> enemiesNumber_;
                    break;
                case TROPHIES_AMOUNT :
                    stream << line;
                    stream >> trophiesAmount_;
                    break;
                case TILE_SET :
                    bitmapWall_ = al_load_bitmap(line.c_str());
                    break;
                case MAP :
                    map_.push_back(line);
                    break;
                default:
                    break;
            }

        }
        fi.close();
        initialMap_ = map_;
        if(map_.empty() || bitmapWall_ == nullptr)
            throw std::runtime_error("Could not load game settings");
    }
    else
    {
        throw std::runtime_error("Could not load game settings");
    }
}

void Game::loadBitmaps()
{
    bitmapTank_ = al_load_bitmap("Resources/Images/tank.png");
    bitmapProjectile_ = al_load_bitmap("Resources/Images/projectile.png");
    bitmapPacmanHealth_ = al_load_bitmap("Resources/Images/health.png");
    if(!bitmapTank_ || !bitmapProjectile_ || !bitmapPacmanHealth_)
    {
        throw std::runtime_error("Could not load images");
    }
}

void Game::loadSounds()
{
    shotSound_ = al_load_sample("Resources/Sounds/shot.wav");
    explosion_ = al_load_sample("Resources/Sounds/explosion.wav");
    al_reserve_samples(3);
    backgroundSound_ = al_load_sample("Resources/Sounds/TankMov.wav");
    if(!shotSound_ || !explosion_ || !backgroundSound_)
    {
        throw std::runtime_error("Could not load sounds");
    }
    backgroundInstance_ = al_create_sample_instance(backgroundSound_);
    al_set_sample_instance_playmode(backgroundInstance_ , ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(backgroundInstance_ , al_get_default_mixer());

}

void Game::loadFonts()
{
    smallFont_ = al_load_font("Resources/Fonts/DroidSansMono.ttf", 36, 0);
    if(!smallFont_)
    {
        throw std::runtime_error("Could not load font");
    }
    middleFont_ = al_load_font("Resources/Fonts/DroidSansMono.ttf", 52, 0);
    largeFont_ = al_load_font("Resources/Fonts/DroidSansMono.ttf", 120, 0);
}

void Game::createTimers()
{
    pacTimer_ = al_create_timer(1 / pacmanMovingSpeed);
    enemyTimer_ = al_create_timer(1 / enemiesMovingSpeed);
    newEnemyTimer_ = al_create_timer(newEnemyFrequency);
    newTrophyTimer_ = al_create_timer(newTrophyFrequency);
    shotTimer_ = al_create_timer(1 / enemiesShootingSpeed);
    projectileTimer_ = al_create_timer(1 / projectilesMovingSpeed);
}

void Game::destroyBitmaps()
{
    al_destroy_bitmap(bitmapWall_);
    al_destroy_bitmap(bitmapPacmanHealth_);
    al_destroy_bitmap(bitmapTank_);
    al_destroy_bitmap(bitmapProjectile_);
}

void Game::destroyFonts()
{
    al_destroy_font(smallFont_);
    al_destroy_font(middleFont_);
    al_destroy_font(largeFont_);
}

void Game::destroySounds()
{
    al_destroy_sample(shotSound_);
    al_destroy_sample(explosion_);
    al_destroy_sample(backgroundSound_);
    al_destroy_sample_instance(backgroundInstance_);
}

void Game::destroyTimers()
{
    al_destroy_timer(pacTimer_);
    al_destroy_timer(enemyTimer_);
    al_destroy_timer(newEnemyTimer_);
    al_destroy_timer(newTrophyTimer_);
    al_destroy_timer(projectileTimer_);
    al_destroy_timer(shotTimer_);
}

void Game::prepareNewGame()
{
    map_ = initialMap_;
    pacman_->repair();
    pacman_->resetPosition();
    pacman_->resetScore();
    pacman_->resetTrophiesAmount();
    pacman_->repair();
    trophies_.clear();
    createTrophies();
    enemies_.clear();
    createEnemies();
    projectiles_.clear();
    gameState_ = GameState::PLAYING;
}

void Game::createTrophies()
{
    for(int i = 0; i < trophiesAmount_; ++i)
        createTrophyOnRandPos();
}

void Game::createTrophyOnRandPos()
{
    int randX, randY;
    bool isPosOK = false;
    static const int interrationLimit = 20;
    int counter = 0;
    do
    {
        randY = rand() % map_.size();
        randX = rand() % map_[randY].size();
        isPosOK = map_[randY][randX] != 'X' && map_[randY][randX] != 'B' &&
        map_[randY][randX] != 'C' && map_[randY][randX] != 'M' &&
                !(pacman_->getX() / pacman_->getTileSize() == randX &&
        pacman_->getY() / pacman_->getTileSize() == randY);
        ++counter;
    }while((counter <= interrationLimit) || !isPosOK);
    if(isPosOK)
    {
        auto t = std::make_unique<Trophy>(randX, randY);
        trophies_.push_back(std::move(t));
    }
}

void Game::createEnemies()
{
    static const int ENEMY_SPEED = 3;
    static const int HUNTER_SPEED = 3;
    static const int FANTOM_SPEED = 3;
    for(int i = 0; i < enemiesNumber_ + 2; ++i)
    {
        int randX, randY;
        bool isGoodPosition = false;
        do
        {
            randY = rand() % map_.size();
            randX = rand() % map_[randY].size();
            bool isBricks = map_[randY][randX] == 'X';
            bool isConcrete = map_[randY][randX] == 'C';
            bool isTree = map_[randY][randX] == 'B';
            bool isBase = map_[randY][randX] == 'M';
            bool isTooClosePos = map_[randY][randX] == 'o';
            bool isPacmanPos = randY * 30 == pacman_->getY() &&
                    randX * 30 == pacman_->getX();
            isGoodPosition = !isBricks && !isConcrete && !isTree &&
                    !isBase && !isPacmanPos && !isTooClosePos;
            for(auto &e : enemies_)
            {
                if(e->getX() == 30 * randX && e->getY() == 30 * randY)
                {
                    isGoodPosition = false;
                    break;
                }
            }
        }
        while(!isGoodPosition);
        Tank::Type currType;
        const int healthPerecentage = 100;
        int speed;
        if(i == 0)
        {
            currType = Tank::Type::HUNTER;
            speed = HUNTER_SPEED;
        }
        else if(i == 1)
        {
            currType = Tank::Type::FANTOM;
            speed = FANTOM_SPEED;
        }
        else
        {
            currType = Tank::Type::ENEMY;
            speed = ENEMY_SPEED;
        }
        auto enemy = std::make_unique<Tank>(currType, randX, randY, speed, healthPerecentage,
                               bitmapTank_, shotSound_, explosion_, this);
        enemies_.push_back(std::move(enemy));
    }
}

void Game::togglePause()
{
    if(gameState_ == GameState::PLAYING)
    {
        gameState_ = GameState::PAUSED;
        stopAllTimers();
        al_stop_sample_instance(backgroundInstance_);
    }
    else if(gameState_ == GameState::PAUSED)
    {
        gameState_ = GameState::PLAYING;
        startAllTimers();
        al_play_sample_instance(backgroundInstance_);
    }
    render_ = true;
}

void Game::addEnemies()
{
    //Tank *enemy1 = new Tank(ENEMY, 0, 0, 3, 100, shotSound_, explosion_, this);
    //enemies_.push_back(enemy1);
    static bool tanksFromTop = false;
    tanksFromTop = !tanksFromTop;
    bool isTopLeftPosOK = true;
    bool isTopRightPosOK = true;
    bool isBottomLeftPosOK = true;
    bool isBottomRightPosOK = true;
    for(auto &e : enemies_)
    {
        if(e->getX() / 30 == 0 && e->getY() / 30 == 0)
           isTopLeftPosOK = false;
        if(e->getX() / 30 == map_[0].size() - 1 && e->getY() / 30 == 0)

           isTopRightPosOK = false;
        if(e->getX() / 30 == 0 && e->getY() / 30 == map_.size() - 1)
           isBottomLeftPosOK = false;
        if(e->getX() / 30 == map_[0].size() - 1 && e->getY() / 30 == map_.size() - 1)
           isBottomRightPosOK = false;
    }
    if(tanksFromTop)
    {
        if(isTopLeftPosOK)
        {
            auto enemy = std::make_unique<Tank>(Tank::Type::ENEMY, 0, 0, 3, 100,
                                   bitmapTank_, shotSound_,
                                   explosion_, this);
            enemies_.push_back(std::move(enemy));
        }
        if(isTopRightPosOK)
        {
            auto enemy = std::make_unique<Tank>(Tank::Type::ENEMY, map_[0].size() - 1,
                                   0, 3, 100, bitmapTank_, shotSound_,
                                   explosion_, this);
            enemies_.push_back(std::move(enemy));
        }
    }
    else
    {
        if(isBottomLeftPosOK)
        {
            auto enemy = std::make_unique<Tank>(Tank::Type::ENEMY, 0, map_.size() - 1,
                                   3, 100, bitmapTank_, shotSound_,
                                   explosion_, this);
            enemies_.push_back(std::move(enemy));
        }
        if(isBottomRightPosOK)
        {
            auto enemy = std::make_unique<Tank>(Tank::Type::ENEMY, map_[0].size() - 1,
                                    map_.size() - 1, 3, 100, bitmapTank_,
                                    shotSound_, explosion_, this);
            enemies_.push_back(std::move(enemy));
        }
    }
}

int Game::countAliveEnemies()
{
    int counter = 0;
    for(auto &e : enemies_)
        if(e->isAlive())
            ++counter;
    return counter;
}

void Game::moveProjectiles()
{
    for(auto &projectile : projectiles_)
    {
        if(projectile->isActive())
        {
            projectile->move();
            int tileSize = al_get_bitmap_height(bitmapWall_);
            int px = projectile->getX() / tileSize;
            int py = projectile->getY() / tileSize;
            if(!(py >= 0 && py < map_.size() && px >= 0 && px < map_.at(py).size()))
                projectile->explode();
            else
            {
                switch(map_[py][px])
                {
                    case 'X' :
                        projectile->explode();
                        map_[py][px] = ' ';
                        break;
                    case 'C' :
                        projectile->explode();
                        break;
                    case 'M' :
                        projectile->explode();
                        gameState_ = GameState::DEFEAT;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void Game::resolveCollisions(Tank *tank)
{
    bool isCollision = false;
    if(tank->isAlive())
    {
        if(tank->isCollision(pacman_.get()))
        {
            gameState_ = GameState::DEFEAT;
            return;
        }
        for(auto &other : enemies_)
        {
            if(other->isAlive() && tank->isCollision(other.get()))
            {
                tank->reverse();
                tank->move();
                isCollision = true;
                break;
            }
        }
        if(!isCollision && tank->isOnCrossRoads())
        {
            if(tank->getType() == Tank::Type::HUNTER ||
                    tank->getType() == Tank::Type::FANTOM)
            {
                tank->chooseShortestWay(pacman_.get());
            }
            else if(tank->getType() == Tank::Type::ENEMY)
            {
                tank->generateRandDir();
            }
        }
    }
}

void Game::startAllTimers()
{
    al_start_timer(pacTimer_);
    al_start_timer(enemyTimer_);
    al_start_timer(newEnemyTimer_);
    al_start_timer(newTrophyTimer_);
    al_start_timer(projectileTimer_);
    al_start_timer(shotTimer_);
}

void Game::stopAllTimers()
{
    al_stop_timer(pacTimer_);
    al_stop_timer(enemyTimer_);
    al_stop_timer(newEnemyTimer_);
    al_stop_timer(newTrophyTimer_);
    al_stop_timer(projectileTimer_);
    al_stop_timer(shotTimer_);
}

void Game::enemiesTimerEvent()
{
    for(auto &e : enemies_)
    {
        e->move();
        resolveCollisions(e.get());
        if(gameState_ == GameState::DEFEAT)
            break;
     }
}

void Game::projectilesTimerEvent()
{
    moveProjectiles();
    for(auto &p :projectiles_)
    {
        if(p->isActive())
        {
            for(auto &target: enemies_)
            {
                if(target->isAlive() && p->isCollision(target.get()))
                    target->explode();
            }
            //Check if current projectile collides pacman
            if(pacman_->isAlive() && p->isCollision(pacman_.get()))
            {
                pacman_->explode();
                if(!pacman_->isAlive())
                    gameState_ = GameState::DEFEAT;
            }
        }
    }
}

void Game::shotTimerEvent()
{
    for(auto &e : enemies_)
    {
        bool isCeaseFire = false;
        if(e->isAlive())
        {
            for(auto &other : enemies_)
            {
                if(other->isAlive() && other != e)
                {
                    switch(e->getDirection())
                    {
                        case Direction::LEFT :
                        case Direction::RIGHT :
                            isCeaseFire = abs(e->getY() - other->getY()) < 60;
                            break;
                        case Direction::UP :
                        case Direction::DOWN :
                            isCeaseFire = abs(e->getX() - other->getX()) < 60;
                            break;
                    }
                }
                if(isCeaseFire)
                    break;
            }
            if(gameState_ == GameState::PLAYING && !isCeaseFire)
                e->fire();
        }
    }
}

void Game::handlePacmanCollisions()
{
    for(auto &t : trophies_)
    {
        if(!t->isGathered() && pacman_->detectTrophy(t->getX(), t->getY()))
        {
            pacman_->gatherTrophy(t.get());
            break;
        }
    }
    for(auto &e : enemies_)
    {
        if(e->isAlive() && pacman_->isCollision(e.get()))
        {
            gameState_ = GameState::DEFEAT;
            break;
            render_ = true;
        }
    }
}

void Game::checkWin()
{
    if(pacman_->getNumGatheredThrophies() == trophiesAmount_)
    {
        gameState_ = GameState::VICTORY;
        render_ = true;
    }
}

void Game::renderProjectiles()
{
    for(auto &projectile : projectiles_)
            projectile->draw();
}

void Game::renderMap()
{
    int tileSize = al_get_bitmap_width(bitmapWall_) / 5;
    for(int row = 0; row < int(map_.size()); ++row)
        for(int col = 0; col < int(map_[row].size()); ++col)
            if(map_[row][col] == 'X')
            {
                al_draw_bitmap_region(bitmapWall_, 0, 0, tileSize, tileSize,
                                      col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'B')
            {
                al_draw_bitmap_region(bitmapWall_, tileSize, 0, tileSize, tileSize,
                                      col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'W')
            {
               al_draw_bitmap_region(bitmapWall_, 2 * tileSize, 0, tileSize, tileSize,
                                     col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'C')
            {
               al_draw_bitmap_region(bitmapWall_, 3 * tileSize, 0, tileSize, tileSize,
                                     col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'M')
            {
               if(gameState_ != GameState::DEFEAT)
                    al_draw_bitmap_region(bitmapWall_, 4 * tileSize, 0, tileSize,
                                          tileSize, col * tileSize, row *tileSize, 0);
            }
}

void Game::renderText(const int &textLeft, const int &textTop, const std::string &text,
              ALLEGRO_COLOR color, ALLEGRO_FONT *font)
{
    al_draw_text(font, color, textLeft, textTop, 0, text.c_str());
}

void Game::renderPacmanScore() const
{
    std::stringstream ss;
    ss << "Score: " << pacman_->getScore() << " Trophies: " << pacman_->getNumGatheredThrophies();
    al_draw_text(smallFont_, al_map_rgb(255,50,0), 20 ,
                 SCREEN_HEIGHT - 55, ALLEGRO_ALIGN_LEFT, ss.str().c_str());
}

void Game::renderPacmanHealth() const
{
    al_draw_text(smallFont_, al_map_rgb(70, 80, 255),
    SCREEN_WIDTH + 5 - (pacman_->getHealth() / 20) *
                 (al_get_bitmap_width(bitmapPacmanHealth_) + 5),
    SCREEN_HEIGHT - 55, ALLEGRO_ALIGN_RIGHT, "Lives: ");
    for(int i = 0; i < pacman_->getHealth() / 20; ++i)
        al_draw_bitmap(bitmapPacmanHealth_, SCREEN_WIDTH - 40 - i *
                       (al_get_bitmap_width(bitmapPacmanHealth_) + 5),
                       SCREEN_HEIGHT - 45, 0);
}

void Game::printEnemiesCoordinates() const
{
    for(int i = 0; i < int(enemies_.size()); ++i)
    {
        std::cout << "---------------------"<< std::endl;
        std::cout << "Tank number " << i + 1 << std::endl;
        std::cout << "X = " << enemies_[i]->getX() / 30 << " Y = " <<
                     enemies_[i]->getY() / 30 << std::endl;
    }
}

//For debugging only
void Game::printMap() const
{
    std::cout << "-------Печатаем карту----------" << std::endl;
    for(int i = 0; i < map_.size(); ++i)
    {
        for(int j = 0; j < map_.at(i).size(); ++j)
        {
            std::cout << map_.at(i).at(j);
        }
        std::cout << std::endl;
    }
}

