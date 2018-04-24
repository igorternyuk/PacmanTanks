#include "game.hpp"

int main()
{
    /*
    1.Разбиение на методы
*/
    try
    {
        Game game;
        game.run();
    }
    catch(std::exception &ex)
    {
        al_show_native_message_box(nullptr, nullptr, nullptr,
                                   ex.what(), nullptr, 0);
    }

    return 0;
}

