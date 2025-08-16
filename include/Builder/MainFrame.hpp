#pragma once
#include <wx/wx.h>

namespace Wepp {
  class MainFrame : public wxFrame {
  private:
    wxMenuBar *m_menu;

    template<typename TYPE>
    TYPE *GetChild(const std::string &_id);

    void OnMenuNew(wxCommandEvent &ev);    
  public:
    MainFrame();
    ~MainFrame();

    void LoadChildren();
  };
}
