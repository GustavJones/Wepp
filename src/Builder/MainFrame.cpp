#include "Builder/MainFrame.hpp"
#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

namespace Wepp {
template <typename TYPE> TYPE *MainFrame::GetChild(const std::string &_id) {
  return wxDynamicCast(XRCCTRL(*this, _id.c_str(), TYPE), TYPE);
}

void MainFrame::OnMenuNew(wxCommandEvent &ev) {
  std::cout << "Test" << std::endl;
}

void MainFrame::LoadChildren() {
  m_menu = GetChild<wxMenuBar>("mainFrameMenu");

  // GetChild<wxMenuItem>("fileNewProject");

  // m_menu->Bind(wxEVT_MENU, &MainFrame::OnMenuNew, this);
}
} // namespace Wepp
