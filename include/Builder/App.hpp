#pragma once
#include <wx/wx.h>

namespace Wepp {
class WeppBuilderApp : public wxApp {
private:
  wxFrame m_mainFrame;
public:
  WeppBuilderApp();
  // WeppBuilderApp(WeppBuilderApp &&) = default;
  // WeppBuilderApp(const WeppBuilderApp &) = default;
  // WeppBuilderApp &operator=(WeppBuilderApp &&) = default;
  // WeppBuilderApp &operator=(const WeppBuilderApp &) = default;
  ~WeppBuilderApp();

  bool OnInit() override;
  int OnExit() override;

private:
};
} // namespace Wepp
