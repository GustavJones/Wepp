#pragma once
#include <wx/wx.h>

#define WEPP_BUILDER_APP_NAME "Wepp Builder"
#define WEPP_BUILDER_APP_VERSION_MAJOR 0
#define WEPP_BUILDER_APP_VERSION_MINOR 1

namespace Wepp {
class WeppBuilderApp : public wxApp {
private:
  wxFrame *m_mainFrame;
public:
  WeppBuilderApp();
  ~WeppBuilderApp();

  bool OnInit() override;
  int OnExit() override;
};
} // namespace Wepp
