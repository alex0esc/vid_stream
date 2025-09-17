#include "client.hpp"
#include <iostream>


int main() {        
    try {
        vsa::Client client;
        client.init();
        client.run();
        client.destroy();
    } catch (std::exception& error) {
        std::cout<< error.what() << std::endl;
    }
}

#ifdef _WIN32
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    return main();
}
#endif
