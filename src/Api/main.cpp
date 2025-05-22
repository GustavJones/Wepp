#include "Api/HandlerFunctions.hpp"
#include "GLog/Log.hpp"
#include "Server/Server.hpp"
#include <cstdint>
#include <string>

static const std::string PREFIX = "[Wepp-Api]";
static const std::string ADDRESS = "0.0.0.0";
static const uint16_t PORT = 8082;

int main (int argc, char *argv[]) {
  GLog::SetLogLevel(GLog::LOG_DEBUG);
  GLog::SetLogPrefix(PREFIX);

  Wepp::Server server(Wepp::HandleApi, Wepp::HandleApiPost);

  GLog::Log(GLog::LOG_PRINT, "Starting Api server on " + ADDRESS + ':' + std::to_string(PORT));
  server.Run(ADDRESS, PORT);
  return 0;
}
