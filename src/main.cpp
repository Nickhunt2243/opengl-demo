#include <iostream>

#include "setup/app.hpp"

int main()
{
    Engine::Application app{};
    if (!app.initialize()) return -1;
    app.run();
    std::cout << "Closing..." << std::endl;
    return 0;
}
