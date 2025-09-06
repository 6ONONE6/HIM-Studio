#include "DragDropPanel.hpp"
#include "Widgets/Label.hpp"
#include <slic3r/GUI/wxExtensions.hpp>

namespace Slic3r { namespace GUI {

struct CustomData
{
    int filament_id;
    unsigned char r, g, b;
};


wxColor Hex2Color(const std::string& str)
{
    if (str.empty() || (str.length() != 9 && str.length() != 7) || str[0] != '#')
        throw std::invalid_argument("Invalid hex color format");

    auto hexToByte = [](const std::string& hex)->unsigned char
        {
            unsigned int byte;
            std::istringstream(hex) >> std::hex >> byte;
            return static_cast<unsigned char>(byte);
        };
    auto r = hexToByte(str.substr(1, 2));
    auto g = hexToByte(str.substr(3, 2));
    auto b = hexToByte(str.substr(5, 2));
    unsigned char a = 255;
    if (str.size() == 9)
        a = hexToByte(str.substr(7, 2));
    return wxColor(r, g, b, a);
}

// Custom data object used to store information that needs to be backed up during drag and drop
class ColorDataObject : public wxCustomDataObject
{
public:
    ColorDataObject(const wxColour &color = *wxBLACK, int filament_id = 0)
        : wxCustomDataObject(wxDataFormat("application/customize_format"))
    {
        std::memset(&m_data, 0, sizeof(m_data));
        set_custom_data_filament_id(filament_id);
        set_custom_data_color(color);
    }

    wxColour GetColor() const { return wxColor(m_data.r, m_data.g, m_data.b); }
    void     SetColor(const wxColour &color) { set_custom_data_color(color); }

    int      GetFilament() const { return m_data.filament_id; }
    void     SetFilament(int label) { set_custom_data_filament_id(label); }

    void set_custom_data_filament_id(int filament_id) {
        m_data.filament_id = filament_id;
    }

    void set_custom_data_color(const wxColor& color) {
        m_data.r           = color.Red();
        m_data.g           = color.Green();
        m_data.b           = color.Blue();
    }

    virtual size_t GetDataSize() const override { return sizeof(m_data); }
    virtual bool   GetDataHere(void *buf) const override
    {
        char *ptr = static_cast<char *>(buf);
        std::memcpy(buf, &m_data, sizeof(m_data));
        return true;
    }
    virtual bool SetData(size_t len, const void *buf) override
    {
        if (len == GetDataSize()) {
            std::memcpy(&m_data, buf, sizeof(m_data));
            return true;
        }
        return false;
    }
private:
    CustomData m_data;
};

///////////////   ColorPanel  start ////////////////////////
// The UI panel of drag item
ColorPanel::ColorPanel(DragDropPanel *parent, const wxColour &color, int filament_id)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(32, 40), wxBORDER_NONE), m_parent(parent), m_color(color), m_filament_id(filament_id)
{
    Bind(wxEVT_LEFT_DOWN, &ColorPanel::OnLeftDown, this);
    Bind(wxEVT_LEFT_UP, &ColorPanel::OnLeftUp, this);
    Bind(wxEVT_PAINT, &ColorPanel::OnPaint, this);
}

void ColorPanel::OnLeftDown(wxMouseEvent &event)
{
    m_parent->set_is_draging(true);
    m_parent->DoDragDrop(this, GetColor(), GetFilamentId());
}

void ColorPanel::OnLeftUp(wxMouseEvent &event) { m_parent->set_is_draging(false); }

void ColorPanel::OnPaint(wxPaintEvent &event)
{
    wxPaintDC dc(this);
    wxSize   size  = GetSize();
    std::string replace_color = m_color.GetAsString(wxC2S_HTML_SYNTAX).ToStdString();
    wxBitmap bmp = ScalableBitmap(this, "filament_green", 40, false, false, false, { replace_color }).bmp();
    dc.DrawBitmap(bmp, wxPoint(0,0));
    wxString label = wxString::Format(wxT("%d"), m_filament_id);
    dc.SetTextForeground(m_color.GetLuminance() < 0.51 ? *wxWHITE : *wxBLACK);  // set text color
    dc.DrawLabel(label, wxRect(0, 0, size.GetWidth(), size.GetHeight()), wxALIGN_CENTER);
}
///////////////   ColorPanel  end ////////////////////////


// Save the source object information to m_data when dragging
class ColorDropSource : public wxDropSource
{
public:
    ColorDropSource(wxPanel *parent, wxPanel *color_block, const wxColour &color, int filament_id) : wxDropSource(parent)
    {
        m_data.SetColor(color);
        m_data.SetFilament(filament_id);
        SetData(m_data);  // Set drag source data
    }

private:
    ColorDataObject m_data;
};

///////////////   ColorDropTarget  start ////////////////////////
// Get the data from the drag source when drop it
class ColorDropTarget : public wxDropTarget
{
public:
    ColorDropTarget(DragDropPanel *panel) : wxDropTarget(/*new wxDataObjectComposite*/), m_panel(panel)
    {
        m_data = new ColorDataObject();
        SetDataObject(m_data);
    }

    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) override;
    virtual bool         OnDrop(wxCoord x, wxCoord y) override {
        return true;
    }

private:
    DragDropPanel * m_panel;
    ColorDataObject* m_data;
};

wxDragResult ColorDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if (!GetData())
        return wxDragNone;

    ColorDataObject *dataObject = dynamic_cast<ColorDataObject *>(GetDataObject());
    m_panel->AddColorBlock(m_data->GetColor(), m_data->GetFilament());

    return wxDragCopy;
}
///////////////   ColorDropTarget  end ////////////////////////


DragDropPanel::DragDropPanel(wxWindow *parent, const wxString &label, bool is_auto)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
    , m_is_auto(is_auto)
{
    SetBackgroundColour(0xF8F8F8);

    m_sizer    = new wxBoxSizer(wxVERTICAL);

    auto title_panel = new wxPanel(this);
    title_panel->SetBackgroundColour(0xEEEEEE);
    auto title_sizer = new wxBoxSizer(wxHORIZONTAL);
    title_panel->SetSizer(title_sizer);

    Label* static_text = new Label(this, label);
    static_text->SetFont(Label::Head_13);
    static_text->SetBackgroundColour(0xEEEEEE);

    title_sizer->Add(static_text, 0, wxALIGN_CENTER | wxALL, FromDIP(5));

    m_sizer->Add(title_panel, 0, wxEXPAND);
    m_sizer->AddSpacer(20);

    m_grid_item_sizer = new wxGridSizer(0, 6, FromDIP(5),FromDIP(5));   // row = 0, col = 3,  10 10 is space
    m_sizer->Add(m_grid_item_sizer, 1, wxEXPAND);

    // set droptarget
    auto drop_target = new ColorDropTarget(this);
    SetDropTarget(drop_target);

    SetSizer(m_sizer);
    Layout();
    Fit();
}

void DragDropPanel::AddColorBlock(const wxColour &color, int filament_id, bool update_ui)
{
    ColorPanel *panel = new ColorPanel(this, color, filament_id);
    panel->SetMinSize(wxSize(FromDIP(32), FromDIP(40)));
    m_grid_item_sizer->Add(panel, 0, wxALIGN_CENTER);
    m_filament_blocks.push_back(panel);
    if (update_ui) {
        m_filament_blocks.front()->Refresh();  // FIX BUG: STUDIO-8467
        Layout();
        Fit();
        GetParent()->Layout();
        GetParent()->Fit();
    }
}

void DragDropPanel::RemoveColorBlock(ColorPanel *panel, bool update_ui)
{
    m_sizer->Detach(panel);
    panel->Destroy();
    m_filament_blocks.erase(std::remove(m_filament_blocks.begin(), m_filament_blocks.end(), panel), m_filament_blocks.end());
    if (update_ui) {
        Layout();
        Fit();
        GetParent()->Layout();
        GetParent()->Fit();
    }
}

void DragDropPanel::DoDragDrop(ColorPanel *panel, const wxColour &color, int filament_id)
{
    if (m_is_auto)
        return;

    ColorDropSource source(this, panel, color, filament_id);
    if (source.DoDragDrop(wxDrag_CopyOnly) == wxDragResult::wxDragCopy) {
        this->RemoveColorBlock(panel);
    }
}

std::vector<int> DragDropPanel::GetAllFilaments() const
{
    std::vector<int>          filaments;
    for (size_t i = 0; i < m_grid_item_sizer->GetItemCount(); ++i) {
        wxSizerItem *item = m_grid_item_sizer->GetItem(i);
        if (item != nullptr) {
            wxWindow *  window     = item->GetWindow();
            ColorPanel *colorPanel = dynamic_cast<ColorPanel *>(window);
            if (colorPanel != nullptr) {
                filaments.push_back(colorPanel->GetFilamentId());
            }
        }
    }

    return filaments;
}

}} // namespace Slic3r::GUI
