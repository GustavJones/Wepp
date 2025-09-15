#pragma once
#include <cstdint>
#include <filesystem>
#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/splitter.h>
#include <wx/propgrid/propgrid.h>
#include <wx/webview.h>
#include <wx/dataview.h>
#include "GParsing/HTML/GParsing-HTML.hpp"

namespace Wepp {
class MainFrame : public wxFrame {
private:
  GParsing::HTMLDocument<char> m_doc;
  std::filesystem::path m_filesDir;
  std::string m_webserverAddress;
  uint16_t m_webserverPort;

  enum {
    ID_MENU_NEW = 1,
    ID_MENU_OPEN,
    ID_MENU_EXIT,
    ID_MENU_TOGGLE_SIDEBAR,
    ID_MENU_TOGGLE_TOOLKIT,
    ID_MENU_ABOUT,

    ID_TOOLKIT_DATATREE,
    ID_WEB_VIEW
  };

  wxMenuBar *m_menuBar;
  wxMenu *m_menuFile, *m_menuView, *m_menuHelp;
  wxMenuItem *m_fileNew, *m_fileOpen, *m_fileExit;
  wxMenuItem *m_viewToggleSidebar, *m_viewToggleToolkit;
  wxMenuItem *m_helpAbout;

  wxAuiManager m_auiManager;

  wxPanel *m_auiSidebar;
  wxWebView *m_auiViewport;
  wxPanel *m_auiToolkit;

  wxPropertyGrid *m_sidebarProperties;
  wxDataViewTreeCtrl* m_toolkitDataTree;

  wxDataViewItem m_rootContainer;
  wxDataViewItem m_doctypeContainer;

public:
  MainFrame(const std::string &_programName, uint16_t _programVersionMajor, uint16_t _programVersionMinor, const std::filesystem::path &_filesDir, const std::string &_webserverAddress, const uint16_t _webserverPort);
  ~MainFrame();

private:
  void MenuExit(wxCommandEvent &_ev);
  void MenuToggleSidebar(wxCommandEvent &_ev);
  void MenuToggleToolkit(wxCommandEvent &_ev);
};
} // namespace Wepp
