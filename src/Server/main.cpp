#include "Server/Server.hpp"
#include "GLog/Log.hpp"

int main (int argc, char *argv[]) {
  GLog::SetLogLevel(GLog::LOG_TRACE);
  Wepp::Server serverApp;
  serverApp.Run("0.0.0.0", 8081);
  return 0;
}
