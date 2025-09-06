
#ifndef FILAMENT_MAP_PANEL_HPP
#define FILAMENT_MAP_PANEL_HPP

#include "GUI.hpp"
#include "DragDropPanel.hpp"
#include "Widgets/SwitchButton.hpp"

namespace Slic3r { namespace GUI {

class FilamentMapManualPanel : public wxPanel
{
public:
    FilamentMapManualPanel(wxWindow *parent, const std::vector<std::string> &color, const std::vector<int> &filament_list, const std::vector<int> &filament_map);

    std::vector<int> GetFilamentMaps() const { return m_filament_map; }
    std::vector<int> GetLeftFilaments() const { return m_left_panel->GetAllFilaments(); }
    std::vector<int> GetRightFilaments() const { return m_right_panel->GetAllFilaments(); }

    void Hide();
    void Show();

private:
    void           OnSwitchFilament(wxCommandEvent &);
    DragDropPanel *m_left_panel;
    DragDropPanel *m_right_panel;

    Label *m_description;
    Label *m_tips;

    ScalableButton *m_switch_btn;

    std::vector<int>         m_filament_map;
    std::vector<int>         m_filament_list;
    std::vector<std::string> m_filament_color;
};

class FilamentMapBtnPanel : public wxPanel
{
public:
    FilamentMapBtnPanel(wxWindow *parent, const wxString &label, const wxString &detail, const std::string &icon_path);
    void Hide();
    void Show();
    void Select(bool selected);
protected:
    void OnPaint(wxPaintEvent &event);
private:
    void OnEnterWindow(wxMouseEvent &event);
    void OnLeaveWindow(wxMouseEvent &evnet);

    void UpdateStatus();

    wxBitmapButton *m_btn;
    wxStaticText   *m_label;
    Label          *m_detail;
    bool            m_hover{false};
    bool            m_selected{false};
};

class FilamentMapAutoPanel : public wxPanel
{
public:
    FilamentMapAutoPanel(wxWindow *parent, FilamentMapMode mode);
    void            Hide();
    void            Show();
    FilamentMapMode GetMode() const { return m_mode; }

private:
    void OnModeSwitch(FilamentMapMode mode);
    void UpdateStatus();

    FilamentMapBtnPanel *m_flush_panel;
    FilamentMapBtnPanel *m_match_panel;
    FilamentMapMode      m_mode;
};

class FilamentMapDefaultPanel : public wxPanel
{
public:
    FilamentMapDefaultPanel(wxWindow *parent);
    void Hide();
    void Show();

private:
    Label *m_label;
};
}} // namespace Slic3r::GUI

#endif