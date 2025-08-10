#pragma once

#include <wx/wx.h>
#include <wx/taskbar.h>
#include <wx/clipbrd.h>
#include <wx/listctrl.h>
#include <wx/datetime.h>
#include <wx/timer.h>
#include <wx/tokenzr.h>
#include <wx/textfile.h>
#include <wx/log.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/dataobj.h>
#include <wx/imaglist.h>
#include <vector>
#include <fstream>
#include <windows.h>

// Forward declaration
class ClipboardFrame;

// Notification popup window
class NotificationPopup : public wxFrame {
public:
    NotificationPopup(wxWindow* parent, const wxString& title, const wxString& content, bool isImage = false);
    virtual ~NotificationPopup();

private:
    void OnTimer(wxTimerEvent& event);
    void OnClose(wxCloseEvent& event);
    void PositionWindow();
    
    wxTimer* m_timer;
    
    enum {
        ID_NOTIFICATION_TIMER = 30001
    };
    
    DECLARE_EVENT_TABLE()
};

struct ClipboardEntry {
    wxString content;        // Text content or image file path
    wxString type;           // "Text", "Image", "File"
    wxDateTime timestamp;
    size_t id;
    wxString imagePath;      // Path to saved image file (for images)
    wxSize imageSize;        // Original image dimensions
};

class ClipboardTaskBarIcon : public wxTaskBarIcon {
public:
    ClipboardTaskBarIcon(class ClipboardFrame* parent);
    virtual ~ClipboardTaskBarIcon();

    void OnMenuShow(wxCommandEvent& event);
    void OnMenuExit(wxCommandEvent& event);
    void OnLeftButtonClick(wxTaskBarIconEvent& event);
    void OnLeftButtonDClick(wxTaskBarIconEvent& event);

    virtual wxMenu* CreatePopupMenu() override;

private:
    class ClipboardFrame* m_parent;

    enum {
        ID_SHOW = 10001,
        ID_EXIT = 10002
    };

    DECLARE_EVENT_TABLE()
};

class ClipboardFrame : public wxFrame {
public:
    ClipboardFrame();
    virtual ~ClipboardFrame();

    void AddClipboardEntry(const ClipboardEntry& entry);
    void ShowFrame();
    void HideFrame();

private:
    void OnClose(wxCloseEvent& event);
    void OnIconize(wxIconizeEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnClearAll(wxCommandEvent& event);
    void OnCopySelected(wxCommandEvent& event);
    void OnItemActivated(wxListEvent& event);

    void CheckClipboard();
    void SaveToFile();
    void LoadFromFile();
    wxString GetClipboardText();
    wxBitmap GetClipboardBitmap();
    wxString SaveImageToFile(const wxBitmap& bitmap, size_t id);
    wxString DetermineDataType();
    wxString CalculateImageHash(const wxBitmap& bitmap);
    void CopyImageToClipboard(const wxString& imagePath);
    
    // Keyboard monitoring
    bool InstallKeyboardHook();
    void UninstallKeyboardHook();
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    void OnCtrlCPressed();

    ClipboardTaskBarIcon* m_taskBarIcon;
    wxListCtrl* m_listCtrl;
    wxTimer* m_timer;
    wxButton* m_clearButton;
    wxButton* m_copyButton;

    std::vector<ClipboardEntry> m_entries;
    wxString m_lastClipboardContent;
    wxString m_lastImageHash;  // Hash of last processed image
    size_t m_nextId;
    
    // Debounce mechanism variables (unused but kept for future)
    wxString m_pendingClipboardContent;
    wxDateTime m_pendingContentTimestamp;
    
    // Keyboard hook variables
    HHOOK m_keyboardHook;
    static ClipboardFrame* s_instance;
    static bool s_ctrlCPressed;
    static wxDateTime s_lastCtrlCTime;

    static const wxString LOG_FILE;

    enum {
        ID_TIMER = 20001,
        ID_CLEAR_ALL = 20002,
        ID_COPY_SELECTED = 20003
    };

    DECLARE_EVENT_TABLE()
};

class ClipboardApp : public wxApp {
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;

private:
    ClipboardFrame* m_frame;
};

DECLARE_APP(ClipboardApp)
