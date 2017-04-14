#include "game.h"
#include "tank.h"
#include "trophy.h"
#include "projectile.h"

Game::Game()
{
    // Инициализируем библиотеку аллегро
    init();
    pacmanHealthBitmap_ = al_load_bitmap("Resources/Images/health.png");
    // Создаем три шрифта разных размеров
    smallFont_ = al_load_font("Resources/Fonts/DroidSansMono.ttf", 36, 0);
    middleFont_ = al_load_font("Resources/Fonts/DroidSansMono.ttf", 52, 0);
    largeFont_ = al_load_font("Resources/Fonts/DroidSansMono.ttf", 120, 0);
    //Загружаем звуковые эффекты
    shotSound_ = al_load_sample("Resources/Sounds/shot.wav");
    explosion_ = al_load_sample("Resources/Sounds/explosion.wav");
    al_reserve_samples(3);
    backgroundSound_ = al_load_sample("Resources/Sounds/TankMov.wav");
    backgroundInstance_ = al_create_sample_instance(backgroundSound_);
    al_set_sample_instance_playmode(backgroundInstance_ , ALLEGRO_PLAYMODE_LOOP);
    al_attach_sample_instance_to_mixer(backgroundInstance_ , al_get_default_mixer());
    //Создаем 4 таймера
    const float pacmanMovingSpeed = 10.0f;
    pacTimer_ = al_create_timer(1 / pacmanMovingSpeed);
    const float enemiesMovingSpeed = 5.0f;
    enemyTimer_ = al_create_timer(1 / enemiesMovingSpeed);
    const float newEnemyFrequency = 30.0f;
    newEnemyTimer_ = al_create_timer(newEnemyFrequency);
    const float newTrophyFrequency = 20.0f;
    newTrophyTimer_ = al_create_timer(newTrophyFrequency);
    const float enemiesShootingSpeed = 2.0f;
    shotTimer_ = al_create_timer(1 / enemiesShootingSpeed);
    const float projectilesMovingSpeed = 20.0f;
    projectileTimer_ = al_create_timer(1 / projectilesMovingSpeed);
    //Создаем пакмена
    pacman_ = new Tank(PACMAN, initPacmanX_, initPacmanY_, 6, 100, shotSound_, explosion_, this);
    //Создаем трофеи
    createTrophies();
    //Создаем врагов
    createEnemies();
}

Game::~Game()
{
    al_destroy_display(display_);
    al_destroy_bitmap(wall_);
    al_destroy_bitmap(pacmanHealthBitmap_);
    al_destroy_sample(shotSound_);
    al_destroy_sample(explosion_);
    al_destroy_sample(backgroundSound_);
    al_destroy_sample_instance(backgroundInstance_);
    al_destroy_font(smallFont_);
    al_destroy_font(middleFont_);
    al_destroy_font(largeFont_);
    al_destroy_timer(pacTimer_);
    al_destroy_timer(enemyTimer_);
    al_destroy_timer(newEnemyTimer_);
    al_destroy_timer(newTrophyTimer_);
    al_destroy_timer(projectileTimer_);
    al_destroy_timer(shotTimer_);
    delete pacman_;
    for(auto &trophy : trophies_)
        if(trophy != nullptr)
            delete trophy;
    for(auto &enemy : enemies_)
        if(enemy != nullptr)
            delete enemy;
    for(auto &projectile : projectiles_)
        if(projectile != nullptr)
            delete projectile;
}

void Game::go()
{

    ALLEGRO_KEYBOARD_STATE keyState;
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_timer_event_source(pacTimer_));
    al_register_event_source(event_queue, al_get_timer_event_source(enemyTimer_));
    al_register_event_source(event_queue, al_get_timer_event_source(newEnemyTimer_));
    al_register_event_source(event_queue, al_get_timer_event_source(projectileTimer_));
    al_register_event_source(event_queue, al_get_timer_event_source(shotTimer_));
    al_register_event_source(event_queue, al_get_display_event_source(display_));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    startAllTimers();

    al_play_sample_instance(backgroundInstance_);

    while (!done_)
    {
        ALLEGRO_EVENT events;
        al_wait_for_event(event_queue, &events);
        al_get_keyboard_state(&keyState);
        bool draw = true;
        if(events.type == ALLEGRO_EVENT_KEY_UP)
        {
            if(events.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                done_ = true;
            }
            else if(events.keyboard.keycode == ALLEGRO_KEY_SPACE)
            {
                if(!isWin_ && !isGameOver_)
                {
                    isGamePaused_ = !isGamePaused_;
                    if(isGamePaused_)
                    {
                        stopAllTimers();
                        al_stop_sample_instance(backgroundInstance_);
                    }
                    else
                    {
                        startAllTimers();
                        al_play_sample_instance(backgroundInstance_);
                    }
                    draw = true;
                }
            }
            else if(events.keyboard.keycode == ALLEGRO_KEY_N)
            {
                prepareNewGame();
                startAllTimers();
                al_play_sample_instance(backgroundInstance_);
            }
            else if(events.keyboard.keycode == ALLEGRO_KEY_F)
            {
                if(!isGamePaused_ && !isWin_ && !isGameOver_)
                {
                    pacman_->fire(projectiles_);
                    draw = true;
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
            done_ = true;
        }
        else if(events.type == ALLEGRO_EVENT_TIMER)
        {
            if(events.timer.source == projectileTimer_)
            {
                projectilesTimerEvent();
                draw = true;
            }
            else if(events.timer.source == pacTimer_)
            {
                if(al_key_down(&keyState, ALLEGRO_KEY_D))
                    pacman_->move();
                if(al_key_down(&keyState, ALLEGRO_KEY_LEFT))
                    pacman_->turn(LEFT);
                else if(al_key_down(&keyState, ALLEGRO_KEY_RIGHT))
                    pacman_->turn(RIGHT);
                else if(al_key_down(&keyState, ALLEGRO_KEY_UP))
                    pacman_->turn(UP);
                else if(al_key_down(&keyState, ALLEGRO_KEY_DOWN))
                    pacman_->turn(DOWN);
                for(auto t : trophies_)
                {
                    if(!t->isGathered() && pacman_->detectTrophy(t->getX(), t->getY()))
                    {
                        pacman_->gatherTrophy(t);
                        break;
                    }
                }
                for(auto &e : enemies_)
                {
                    //На время отключаем столкновение с пакменом
                    if(e->isAlive() && pacman_->isCollision(e))
                    {
                        isGameOver_ = true;
                        break;
                        draw = true;
                    }
                }
                if(pacman_->getNumGatheredThrophies() == int(trophies_.size()))
                {
                    isWin_ = true;
                    draw = true;
                }
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
            //Попробуем удалить неактивные снаряды и погибшие танки из векторов

            auto trophy_iterator = std::remove_if(trophies_.begin(), trophies_.end(),
                                   [](Trophy *trophy)
                                    {
                                        bool isGathered = trophy->isGathered();
                                        if(isGathered) delete trophy;
                                        return isGathered;
                                    });
            trophies_.erase(trophy_iterator, trophies_.end());

            /*auto enemy_iterator = std::remove_if(enemies_.begin(), enemies_.end(),
                                   [](Tank *enemy)
                                    {
                                        bool isDead = !enemy->isAlive();
                                        if(isDead) delete enemy;
                                        return isDead;
                                    });
            enemies_.erase(enemy_iterator, enemies_.end());*/

            auto projectile_iterator = std::remove_if(projectiles_.begin(), projectiles_.end(),
                                   [](Projectile *projectile)
                                    {
                                        bool isDead = !projectile->isActive();
                                        if(isDead) delete projectile;
                                        return isDead;
                                    });
            projectiles_.erase(projectile_iterator, projectiles_.end());

            if(isGameOver_ || isWin_)
            {
                stopAllTimers();
                al_stop_sample_instance(backgroundInstance_);
            }
            draw = true;
        }
        if(draw)
        {
            drawMap();
            drawProjectiles();
            for(auto &t : trophies_)
                t->draw();
            pacman_->draw();
            for(auto &e : enemies_)
                e->draw();
            drawPacmanScore();
            drawPacmanHealth();
            if(isGamePaused_ && !isWin_ && !isGameOver_)
                drawText(SCREEN_WIDTH / 2 - 400, SCREEN_HEIGHT / 2 - 100, GAME_PAUSED_TEXT, al_map_rgb(255, 255, 255), largeFont_);
            else if(isWin_)
                drawText(SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 100, WIN_MESSAGE_TEXT, al_map_rgb(255, 255, 0), largeFont_);
            else if(isGameOver_)
                drawText(SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 100, LOST_MESSAGE_TEXT, al_map_rgb(255, 255, 255), largeFont_);
            al_flip_display();
            al_clear_to_color(al_map_rgb(0, 0, 0));
        }
    }
    al_destroy_event_queue(event_queue);
}

charMatrix Game::getMap()
{
    return map_;
}

void Game::init()
{
    if(!al_init())
    {
        al_show_native_message_box(nullptr, nullptr, nullptr, "Could not initialize Allegro 5", nullptr, 0);
    }

    display_ = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);

    if(!display_)
    {
         al_show_native_message_box(nullptr, nullptr, nullptr, "Could not create Allegro Window", nullptr, 0);
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

    if(!loadSettings("settings.txt"))
    {
        al_show_native_message_box(nullptr, nullptr, nullptr, "Could not load settings", nullptr, 0);
        done_ = true;
    }
}

bool Game::loadSettings(const std::string &fileName)
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
                    wall_ = al_load_bitmap(line.c_str());
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
        return !map_.empty() && wall_ != nullptr;
    }
    else
    {
        al_show_native_message_box(nullptr, nullptr, nullptr, "Could not open the settings file", nullptr, 0);
        return false;
    }
}

void Game::prepareNewGame()
{
    map_ = initialMap_;
    pacman_->repair();
    pacman_->resetPosition();
    pacman_->resetScore();
    pacman_->resetTrophiesAmount();
    pacman_->repair();
    if(!trophies_.empty())
    {
        for(auto &trophy : trophies_)
            if(trophy != nullptr)
                delete trophy;
    }
    trophies_.clear();
    createTrophies();
    if(!enemies_.empty())
    {
        for(auto &enemy : enemies_)
            if(enemy != nullptr)
                delete enemy;
    }
    enemies_.clear();
    createEnemies();
    if(!projectiles_.empty())
    {
        for(auto &projectile : projectiles_)
            if(projectile != nullptr)
                delete projectile;
    }
    projectiles_.clear();
    isGamePaused_ = false;
    isGameOver_ = false;
    isWin_ = false;
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
        map_[randY][randX] != 'C' && map_[randY][randX] != 'M' && !(pacman_->getX() / pacman_->getTileSize() == randX &&
        pacman_->getY() / pacman_->getTileSize() == randY);
        ++counter;
    }while((counter <= interrationLimit) || !isPosOK);
    if(isPosOK)
    {
        Trophy *t = new Trophy(randX, randY);
        trophies_.push_back(t);
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
            bool isPacmanPos = randY * 30 == pacman_->getY() && randX * 30 == pacman_->getX();
            isGoodPosition = !isBricks && !isConcrete && !isTree && !isBase && !isPacmanPos && !isTooClosePos;
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
        TankType currType;
        const int healthPerecentage = 100;
        int speed;
        if(i == 0)
        {
            currType = HUNTER;
            speed = HUNTER_SPEED;
        }
        else if(i == 1)
        {
            currType = FANTOM;
            speed = FANTOM_SPEED;
        }
        else
        {
            currType = ENEMY;
            speed = ENEMY_SPEED;
        }
        Tank *enemy = new Tank(currType, randX, randY, speed, healthPerecentage, shotSound_, explosion_, this);
        enemies_.push_back(enemy);
    }
    //printEnemiesCoordinates();
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
    for(auto e : enemies_)
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
            Tank *enemy = new Tank(ENEMY, 0, 0, 3, 100, shotSound_, explosion_, this);
            enemies_.push_back(enemy);
        }
        if(isTopRightPosOK)
        {
            Tank *enemy = new Tank(ENEMY, map_[0].size() - 1, 0, 3, 100, shotSound_, explosion_, this);
            enemies_.push_back(enemy);
        }
    }
    else
    {
        if(isBottomLeftPosOK)
        {
            Tank *enemy = new Tank(ENEMY, 0, map_.size() - 1, 3, 100, shotSound_, explosion_, this);
            enemies_.push_back(enemy);
        }
        if(isBottomRightPosOK)
        {
            Tank *enemy = new Tank(ENEMY, map_[0].size() - 1, map_.size() - 1, 3, 100, shotSound_, explosion_, this);
            enemies_.push_back(enemy);
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
            int tileSize = al_get_bitmap_height(wall_);
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
                        isGameOver_ = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

void Game::resolveCollisions(Tank *&tank)
{
    bool isCollision = false;
    if(tank->isAlive())
    {
        if(tank->isCollision(pacman_))
        {
            isGameOver_ = true;
            return;
        }
        for(auto &other : enemies_)
        {
            if(other->isAlive() && tank->isCollision(other))
            {
                tank->reverse();
                tank->move();
                isCollision = true;
                break;
            }
        }
        if(!isCollision && tank->isOnCrossRoads())
        {
            if(tank->getType() == HUNTER || tank->getType() == FANTOM)
            {
                tank->chooseShortestWay(pacman_);
            }
            else if(tank->getType() == ENEMY)
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
        resolveCollisions(e);
        if(isGameOver_)
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
                if(target->isAlive() && p->isCollision(target))
                    target->explode();
            }
            //Проверяем коллизию текущего снаряда с пакменом
            if(pacman_->isAlive() && p->isCollision(pacman_))
            {
                pacman_->explode();
                if(!pacman_->isAlive())
                    isGameOver_ = true;
            }
        }
    }
}

void Game::shotTimerEvent()
{
    for(auto &e : enemies_)
    {
        bool isCeaseFire = false;
        if(e->isAlive() /*&& (e->getType() == HUNTER || e->getType() == FANTOM)*/)
        {
            for(auto &other : enemies_)
            {
                if(other->isAlive() && other != e)
                {
                    switch(e->getDirection())
                    {
                        case LEFT :
                        case RIGHT :
                            isCeaseFire = abs(e->getY() - other->getY()) < 60;
                            break;
                        case UP :
                        case DOWN :
                            isCeaseFire = abs(e->getX() - other->getX()) < 60;
                            break;
                    }
                }
                if(isCeaseFire)
                    break;
            }
            if(!isGamePaused_ && !isWin_ && !isGameOver_ && !isCeaseFire)
                e->fire(projectiles_);
        }
    }
}

void Game::drawProjectiles()
{
    for(auto &projectile : projectiles_)
            projectile->draw();
}

void Game::drawMap()
{
    int tileSize = al_get_bitmap_width(wall_) / 5;
    for(int row = 0; row < int(map_.size()); ++row)
        for(int col = 0; col < int(map_[row].size()); ++col)
            if(map_[row][col] == 'X')
            {
                al_draw_bitmap_region(wall_, 0, 0, tileSize, tileSize, col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'B')
            {
                al_draw_bitmap_region(wall_, tileSize, 0, tileSize, tileSize, col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'W')
            {
               al_draw_bitmap_region(wall_, 2 * tileSize, 0, tileSize, tileSize, col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'C')
            {
               al_draw_bitmap_region(wall_, 3 * tileSize, 0, tileSize, tileSize, col * tileSize, row *tileSize, 0);
            }
            else if(map_[row][col] == 'M')
            {
               if(!isGameOver_)
                    al_draw_bitmap_region(wall_, 4 * tileSize, 0, tileSize, tileSize, col * tileSize, row *tileSize, 0);
            }
}

void Game::drawText(const int &textLeft, const int &textTop, const std::string &text,
              ALLEGRO_COLOR color, ALLEGRO_FONT *font)
{
    al_draw_text(font, color, textLeft, textTop, 0, text.c_str());
}

void Game::drawPacmanScore() const
{
    std::stringstream ss;
    ss << "Score: " << pacman_->getScore();
    al_draw_text(smallFont_, al_map_rgb(255,50,0), 20 , SCREEN_HEIGHT - 55, ALLEGRO_ALIGN_LEFT, ss.str().c_str());
}

void Game::drawPacmanHealth() const
{
    al_draw_text(smallFont_, al_map_rgb(70, 80, 255),
    SCREEN_WIDTH + 5 - (pacman_->getHealth() / 20) * (al_get_bitmap_width(pacmanHealthBitmap_) + 5),
    SCREEN_HEIGHT - 55, ALLEGRO_ALIGN_RIGHT, "Pacman health: ");
    for(int i = 0; i < pacman_->getHealth() / 20; ++i)
        al_draw_bitmap(pacmanHealthBitmap_, SCREEN_WIDTH - 40 - i * (al_get_bitmap_width(pacmanHealthBitmap_) + 5), SCREEN_HEIGHT - 45, 0);
}

void Game::printEnemiesCoordinates() const
{
    for(int i = 0; i < int(enemies_.size()); ++i)
    {
        std::cout << "---------------------"<< std::endl;
        std::cout << "Танк номер " << i + 1 << std::endl;
        std::cout << "X = " << enemies_[i]->getX() / 30 << " Y = " << enemies_[i]->getY() / 30 << std::endl;
    }
}

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

