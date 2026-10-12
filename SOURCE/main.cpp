#include "Game.h"
#include <windows.h>
#include <stdexcept>


int main()
{
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Critical Error", MB_ICONERROR | MB_OK);
        return 1;
    }
    return 0;
}
