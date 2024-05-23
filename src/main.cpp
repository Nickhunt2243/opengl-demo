#include <iostream>

#include "setup/app.hpp"
int main()
{
    Engine::Application app{};
    app.initialize();
    app.run();
    std::cout << "Closing..." << std::endl;
    return 0;
}
