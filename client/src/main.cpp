#include "client.hpp"
#include "logger.hpp"


int main() {        
    try {
        vsa::Client client;
        client.init();
        client.run();
        client.destroy();
    } catch (std::exception& error) {
        LOG_ERROR(error.what());
    }
    LOG_INFO("Press any button to close...");
    std::cin.get();
}

#ifdef _WIN32
extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    return main();
}
#endif
