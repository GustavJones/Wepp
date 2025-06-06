#include "Application/HandlerFunctions.hpp"
#include "GLog/Log.hpp"
#include "Server/Server.hpp"
#include <string>
#include <cstdint>

static const std::string PREFIX = "[Wepp]";
static const std::string ADDRESS = "0.0.0.0";
static const uint16_t PORT = 8081;

int main(int argc, char *argv[]) {
  GLog::SetLogLevel(GLog::LOG_DEBUG);
  GLog::SetLogPrefix(PREFIX);

  Wepp::Server server(Wepp::HandleWeb, Wepp::HandleWebPost);

  GLog::Log(GLog::LOG_PRINT, "Starting Wepp server on " + ADDRESS + ':' + std::to_string(PORT));
  server.Run(ADDRESS, PORT);
  return 0;
}
