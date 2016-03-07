#include <SDL.h>
#include <iostream>
#include "raycasterEngine.hpp"

int main(int argc, char *argv[])
{
    Raycaster::RaycasterEngine engine;
    engine.Init();
    engine.Run();
    engine.Cleanup();
    
    return 0;
}
