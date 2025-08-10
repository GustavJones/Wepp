#include "Builder/App.hpp"
#include "wx/object.h"
#include <stdexcept>
#include <wx/xrc/xmlres.h>
#include <wx/wx.h>

namespace Wepp {
WeppBuilderApp::WeppBuilderApp() : wxApp() {}

WeppBuilderApp::~WeppBuilderApp() {}

bool WeppBuilderApp::OnInit() {
  wxXmlResource::Get()->InitAllHandlers();

  if (!wxXmlResource::Get()->Load("MainFrame.xrc")) {
    throw std::runtime_error("Cannot find MainFrame.xrc");
  }

  m_mainFrame = wxDynamicCast(wxXmlResource::Get()->LoadObject(nullptr, "MainFrame", "wxFrame"), wxFrame);
  m_mainFrame->Show();
  return true;
}

int WeppBuilderApp::OnExit() { return 0; }

} // namespace Wepp
