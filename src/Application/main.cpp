#include "GLog/Log.hpp"
#include "Server/Server.hpp"
#include "Application/HandlerFunctions.hpp"

int main(int argc, char *argv[]) {
  GLog::SetLogLevel(GLog::LOG_DEBUG);
  Wepp::Server serverApp(&Wepp::HandleRequestResponse, &Wepp::HandlerContinue);
  serverApp.Run("0.0.0.0", 8081);
  return 0;
}
