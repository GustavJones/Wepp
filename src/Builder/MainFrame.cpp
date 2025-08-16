#include "Builder/MainFrame.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>

namespace Wepp {
MainFrame::MainFrame(const std::string &_programName,
                     uint16_t _programVersionMajor,
                     uint16_t _programVersionMinor, const std::filesystem::path &_filesDir, const std::string &_webserverAddress, const uint16_t _webserverPort)
    : wxFrame(nullptr, wxID_ANY, _programName + " - Main Frame",
              wxDefaultPosition, wxSize(1280, 720)), m_filesDir(_filesDir), m_webserverAddress(_webserverAddress), m_webserverPort(_webserverPort) {
  m_menuBar = new wxMenuBar;
  SetMenuBar(m_menuBar);
  m_menuFile = new wxMenu;
  m_menuView = new wxMenu;
  m_menuHelp = new wxMenu;
  m_menuBar->Append(m_menuFile, "File");
  m_menuBar->Append(m_menuView, "View");
  m_menuBar->Append(m_menuHelp, "Help");
  m_fileNew = m_menuFile->Append(ID_MENU_NEW, "New", "Create a new project");
  m_fileOpen = m_menuFile->Append(ID_MENU_OPEN, "Open", "Open a project");
  m_fileExit = m_menuFile->Append(ID_MENU_EXIT, "Exit", "Exit editor");
  m_fileExit->SetAccel(new wxAcceleratorEntry(wxACCEL_CTRL, (int)'Q'));
  m_menuFile->Bind(wxEVT_MENU, &MainFrame::MenuExit, this, ID_MENU_EXIT);
  m_viewToggleSidebar = m_menuView->Append(ID_MENU_TOGGLE_SIDEBAR, "Toggle Sidebar", "Toggle the sidebar", wxITEM_CHECK);
  m_viewToggleSidebar->SetAccel(new wxAcceleratorEntry(wxACCEL_CTRL, (int)'B'));
  m_viewToggleSidebar->Check(true);
  m_viewToggleToolkit = m_menuView->Append(ID_MENU_TOGGLE_TOOLKIT, "Toggle Toolkit", "Toggle the toolkit panel", wxITEM_CHECK);
  m_viewToggleToolkit->SetAccel(new wxAcceleratorEntry(wxACCEL_CTRL, (int)'T'));
  m_viewToggleToolkit->Check(true);
  m_menuView->Bind(wxEVT_MENU, &MainFrame::MenuToggleSidebar, this, ID_MENU_TOGGLE_SIDEBAR);
  m_menuView->Bind(wxEVT_MENU, &MainFrame::MenuToggleToolkit, this, ID_MENU_TOGGLE_TOOLKIT);
  m_helpAbout = m_menuHelp->Append(ID_MENU_ABOUT, "About", "About this program");

  m_auiManager.SetManagedWindow(this);
  m_auiSidebar = new wxPanel(this);

#ifdef __unix__
  setenv("WEBKIT_DISABLE_COMPOSITING_MODE", "1", 1);
#endif // __unix__

  m_auiViewport = wxWebView::New(this, ID_WEB_VIEW, "http://" + m_webserverAddress + ':' + std::to_string(m_webserverPort));
  m_auiToolkit = new wxPanel(this);

  m_auiManager.AddPane(m_auiSidebar, wxLEFT, "Sidebar");
  m_auiManager.AddPane(m_auiViewport, wxCENTER, "Viewport");
  m_auiManager.AddPane(m_auiToolkit, wxRIGHT, "Toolkit");

  wxAuiPaneInfo &sidebarPaneInfo = m_auiManager.GetPane(m_auiSidebar);
  sidebarPaneInfo.MinSize(200, -1);
  sidebarPaneInfo.PinButton(false);
  sidebarPaneInfo.CloseButton(false);

  wxAuiPaneInfo &viewportPaneInfo = m_auiManager.GetPane(m_auiViewport);
  viewportPaneInfo.CaptionVisible();

  wxAuiPaneInfo &toolkitPaneInfo = m_auiManager.GetPane(m_auiToolkit);
  toolkitPaneInfo.MinSize(200, -1);
  toolkitPaneInfo.PinButton(false);
  toolkitPaneInfo.CloseButton(false);
  m_auiManager.Update();

  wxBoxSizer *sidebarSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *toolkitSizer = new wxBoxSizer(wxVERTICAL);
  m_sidebarProperties = new wxPropertyGrid(m_auiSidebar);
  m_toolkitDataTree = new wxDataViewTreeCtrl(m_auiToolkit, ID_TOOLKIT_DATATREE);

  sidebarSizer->Add(m_sidebarProperties, 1, wxEXPAND);
  toolkitSizer->Add(m_toolkitDataTree, 1, wxEXPAND);
  m_auiSidebar->SetSizer(sidebarSizer);
  m_auiToolkit->SetSizer(toolkitSizer);
  m_auiSidebar->Layout();
  m_auiToolkit->Layout();

  m_sidebarProperties->Append(new wxStringProperty("Test", "Name", "Value"));
  m_toolkitDataTree->AppendContainer(wxDataViewItem(), "Root");
}

MainFrame::~MainFrame() { m_auiManager.UnInit(); }

void MainFrame::MenuExit(wxCommandEvent &_ev) { Close(); }

void MainFrame::MenuToggleSidebar(wxCommandEvent &_ev) {
  wxAuiPaneInfo &paneInfo = m_auiManager.GetPane(m_auiSidebar);
  paneInfo.Show(_ev.IsChecked());
  m_auiManager.Update();
}

void MainFrame::MenuToggleToolkit(wxCommandEvent &_ev) {
  wxAuiPaneInfo &paneInfo = m_auiManager.GetPane(m_auiToolkit);
  paneInfo.Show(_ev.IsChecked());
  m_auiManager.Update();
}
} // namespace Wepp
