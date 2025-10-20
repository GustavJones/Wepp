#include "GLog/Log.hpp"
#include "Wepp/Server/Server.hpp"
#include "Wepp/Server/HandlerFunctions.hpp"
#include <string>
#include <cstdint>

static const std::string PREFIX = "[Wepp]";
static const std::string ADDRESS = "0.0.0.0";
static uint16_t PORT = 8080;

int main(int argc, char *argv[]) {
#ifdef NDEBUG
  GLog::SetLogLevel(GLog::LOG_WARNING);
#else
  GLog::SetLogLevel(GLog::LOG_TRACE);
#endif // NDEBUG

  GLog::SetLogPrefix(PREFIX);

  if (argc == 2) {
    try {
      PORT = std::stoi(argv[1]);
    }
    catch (const std::exception&) {
      GLog::Log(GLog::LOG_WARNING, "Could not set port from command line arguments. Using default.");
    }
  }

  Wepp::SetupHandling();
  Wepp::Server server(Wepp::HandleWeb, Wepp::HandleWebPost, true);

  std::atomic<bool> close = false;
  GLog::Log(GLog::LOG_PRINT, "Starting Wepp server on " + ADDRESS + ':' + std::to_string(PORT));

  try {
    server.Run(ADDRESS, PORT, close);
  }
  catch (const std::exception &e) {
    GLog::Log(GLog::LOG_ERROR, e.what());
    return 1;
  }

  return 0;
}
