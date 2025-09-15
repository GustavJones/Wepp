#include "Builder/App.hpp"
#include "Server/Server.hpp"
#include "Server/HandlerFunctions.hpp"
#include <filesystem>
#include <fstream>
#include <wx/wx.h>

namespace Wepp {

void WeppBuilderApp::Setup(const std::filesystem::path &_filesDir) {
  if (!std::filesystem::exists(_filesDir)) {
    std::filesystem::create_directory(_filesDir);
  }
  else if (!std::filesystem::is_directory(_filesDir)) {
    std::filesystem::remove(_filesDir);
    std::filesystem::create_directory(_filesDir);
  }

  std::fstream f;
  f.open(_filesDir / "index.html", std::ios::app);
  f.close();
}

void WeppBuilderApp::RunWebServer() {
  Wepp::Server s(Wepp::HandleWeb, Wepp::HandleWebPost, true);
  GLog::SetLogLevel(GLog::LOG_TRACE);
  try
  {
    s.Run(WEPP_BUILDER_WEBSERVER_ADDRESS, WEPP_BUILDER_WEBSERVER_PORT, m_closeWebserver);
  }
  catch (const std::exception &e)
  {
    wxMessageBox((std::string)"Webserver crashed. Please restart: " + e.what());
    wxExit();
  }
}

WeppBuilderApp::WeppBuilderApp() : wxApp() {
  m_closeWebserver = false;
}

WeppBuilderApp::~WeppBuilderApp() {}

bool WeppBuilderApp::OnInit() {
  Setup(WEPP_BUILDER_FILES_DIR);

  m_webserverThread = std::async(std::launch::async, &WeppBuilderApp::RunWebServer, this);

  m_mainFrame = new MainFrame(WEPP_BUILDER_APP_NAME, WEPP_BUILDER_APP_VERSION_MAJOR, WEPP_BUILDER_APP_VERSION_MINOR, std::filesystem::absolute(WEPP_BUILDER_FILES_DIR), WEPP_BUILDER_WEBSERVER_ADDRESS, WEPP_BUILDER_WEBSERVER_PORT);
  SetTopWindow(m_mainFrame);
  m_mainFrame->Show();
  return true;
}

int WeppBuilderApp::OnExit() {
  m_closeWebserver = true;
  m_webserverThread.wait();
  return 0;
}

} // namespace Wepp
