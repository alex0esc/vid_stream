#include "server.hpp"
#include "logger.hpp"

int main() {
  try {  
    vsa::Server server;
    server.init();
    server.run();
    server.shutdown();
  } catch (const std::exception& error) {
    LOG_ERROR(error.what());
  }
}
