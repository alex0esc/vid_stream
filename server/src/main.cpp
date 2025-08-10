#include "server.hpp"
#include "logger.hpp"

int main() {
  try {  
    vsa::Server server(50000);
    server.init();
    server.run();
    server.shutdown();
  } catch (const std::exception& error) {
    LOG_ERROR(error.what());
  }
  std::cout << "Press enter to exit..." << std::endl;
  std::cin.get();
}
