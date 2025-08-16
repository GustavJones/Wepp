#pragma once
#include <filesystem>
#include <future>
#include <wx/wx.h>
#include "Builder/MainFrame.hpp"

#define WEPP_BUILDER_APP_NAME "Wepp Builder"
#define WEPP_BUILDER_APP_VERSION_MAJOR 0
#define WEPP_BUILDER_APP_VERSION_MINOR 1
#define WEPP_BUILDER_FILES_DIR "data"
#define WEPP_BUILDER_WEBSERVER_ADDRESS "127.0.0.1"
#define WEPP_BUILDER_WEBSERVER_PORT 55555

namespace Wepp {
class WeppBuilderApp : public wxApp {
private:
  std::atomic<bool> m_closeWebserver;
  std::future<void> m_webserverThread;
  MainFrame *m_mainFrame;

  void Setup(const std::filesystem::path &_filesDir);
  void RunWebServer();
public:
  WeppBuilderApp();
  ~WeppBuilderApp();

  bool OnInit() override;
  int OnExit() override;
};
} // namespace Wepp
