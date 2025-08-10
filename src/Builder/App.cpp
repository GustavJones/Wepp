#include "Builder/App.hpp"
#include "wx/wx.h"

namespace Wepp {
WeppBuilderApp::WeppBuilderApp()
    : wxApp(), m_mainFrame(nullptr, wxID_ANY, "Hello World") {}

WeppBuilderApp::~WeppBuilderApp() {}

bool WeppBuilderApp::OnInit() {
  m_mainFrame.Show();
  return true;
}

int WeppBuilderApp::OnExit() {
  m_mainFrame.Close();
  return 0;
}

} // namespace Wepp
