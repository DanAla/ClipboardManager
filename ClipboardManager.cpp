#include "ClipboardManager.h"
#include <wx/msgdlg.h>
#include <wx/textfile.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/log.h>

// Initialize static members
const wxString ClipboardFrame::LOG_FILE = wxT("clipboard_history.txt");
ClipboardFrame* ClipboardFrame::s_instance = nullptr;
bool ClipboardFrame::s_ctrlCPressed = false;
wxDateTime ClipboardFrame::s_lastCtrlCTime;

// Event tables
wxBEGIN_EVENT_TABLE(ClipboardTaskBarIcon, wxTaskBarIcon)
    EVT_MENU(ID_SHOW, ClipboardTaskBarIcon::OnMenuShow)
    EVT_MENU(ID_EXIT, ClipboardTaskBarIcon::OnMenuExit)
    EVT_TASKBAR_LEFT_UP(ClipboardTaskBarIcon::OnLeftButtonClick)
    EVT_TASKBAR_LEFT_DCLICK(ClipboardTaskBarIcon::OnLeftButtonDClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(NotificationPopup, wxFrame)
    EVT_TIMER(ID_NOTIFICATION_TIMER, NotificationPopup::OnTimer)
    EVT_CLOSE(NotificationPopup::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ClipboardFrame, wxFrame)
    EVT_CLOSE(ClipboardFrame::OnClose)
    EVT_ICONIZE(ClipboardFrame::OnIconize)
    EVT_TIMER(ID_TIMER, ClipboardFrame::OnTimer)
    EVT_BUTTON(ID_CLEAR_ALL, ClipboardFrame::OnClearAll)
    EVT_BUTTON(ID_COPY_SELECTED, ClipboardFrame::OnCopySelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, ClipboardFrame::OnItemActivated)
wxEND_EVENT_TABLE()

// ClipboardTaskBarIcon implementation
ClipboardTaskBarIcon::ClipboardTaskBarIcon(ClipboardFrame* parent) 
    : m_parent(parent) {
    // Set icon for system tray
    wxIcon icon(wxICON(wxICON_INFORMATION));
    SetIcon(icon, wxT("Clipboard Manager"));
}

ClipboardTaskBarIcon::~ClipboardTaskBarIcon() {
}

void ClipboardTaskBarIcon::OnMenuShow(wxCommandEvent& event) {
    m_parent->ShowFrame();
}

void ClipboardTaskBarIcon::OnMenuExit(wxCommandEvent& event) {
    m_parent->Close(true);
}

void ClipboardTaskBarIcon::OnLeftButtonClick(wxTaskBarIconEvent& event) {
    // Toggle window visibility - show if hidden, hide if visible
    if (m_parent->IsShown()) {
        m_parent->HideFrame();
    } else {
        m_parent->ShowFrame();
    }
}

void ClipboardTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent& event) {
    m_parent->ShowFrame();
}

wxMenu* ClipboardTaskBarIcon::CreatePopupMenu() {
    wxMenu* menu = new wxMenu;
    menu->Append(ID_SHOW, wxT("&Show Clipboard Manager"));
    menu->AppendSeparator();
    menu->Append(ID_EXIT, wxT("E&xit"));
    return menu;
}

// NotificationPopup implementation
NotificationPopup::NotificationPopup(wxWindow* parent, const wxString& title, const wxString& content, bool isImage)
    : wxFrame(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 
              wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP | wxBORDER_SIMPLE),
      m_timer(nullptr) {
    
    // Set background color
    SetBackgroundColour(wxColour(245, 245, 245));
    
    // Create main panel
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    panel->SetBackgroundColour(wxColour(245, 245, 245));
    
    // Create title label
    wxStaticText* titleLabel = new wxStaticText(panel, wxID_ANY, title);
    wxFont titleFont = titleLabel->GetFont();
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    titleLabel->SetFont(titleFont);
    titleLabel->SetForegroundColour(wxColour(50, 50, 50));
    
    // Create content label - show full text, preserve newlines
    wxString displayContent = content;
    // Keep original newlines for proper display
    
    // Calculate optimal window size first
    // Start with maximum allowed width to give plenty of horizontal space
    int idealWidth = 470; // Start close to maximum (480) to maximize horizontal space
    int idealHeight = 100; // Base height for title + padding
    
    // Calculate text size for sizing purposes
    wxClientDC dc(panel);
    wxSize textSize = dc.GetTextExtent(displayContent);
    
    // Only reduce width if content is actually narrower
    if (textSize.GetWidth() < 400) {
        idealWidth = wxMax(textSize.GetWidth() + 70, 350); // Generous padding, but not less than 350px
    }
    
    // Calculate height based on content
    int lineHeight = dc.GetCharHeight();
    
    // Count actual newlines in the content
    int newlineCount = displayContent.Freq('\n');
    
    // Estimate wrapped lines based on available width
    int availableTextWidth = idealWidth - 40; // Account for padding and margins
    int estimatedWrappedLines = 1;
    
    if (textSize.GetWidth() > availableTextWidth) {
        estimatedWrappedLines = (textSize.GetWidth() / availableTextWidth) + 1;
    }
    
    // Take the maximum of newlines and wrapped lines
    int totalLines = wxMax(newlineCount + 1, estimatedWrappedLines);
    idealHeight += totalLines * lineHeight + 30; // Extra padding for better appearance
    
    // Enforce maximum size constraints
    idealWidth = wxMin(idealWidth, 480);
    idealHeight = wxMin(idealHeight, 480);
    
    // Set reasonable minimum size
    idealWidth = wxMax(idealWidth, 300); // Increased minimum width
    idealHeight = wxMax(idealHeight, 100);
    
    // Calculate text control size (window size minus title and padding)
    int textWidth = idealWidth - 20; // Account for window padding
    int textHeight = idealHeight - 50; // Account for title and padding
    
    // Create a text control sized to fill most of the window
    wxTextCtrl* contentText = new wxTextCtrl(panel, wxID_ANY, displayContent, 
                                           wxDefaultPosition, wxSize(textWidth, textHeight),
                                           wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP | wxBORDER_NONE);
    contentText->SetBackgroundColour(wxColour(245, 245, 245));
    contentText->SetForegroundColour(wxColour(100, 100, 100));
    
    // Set the calculated size
    SetSize(idealWidth, idealHeight);
    
    // Layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(titleLabel, 0, wxALL | wxEXPAND, 5);
    sizer->Add(contentText, 1, wxALL | wxEXPAND, 5);
    
    panel->SetSizer(sizer);
    
    // Position window in bottom-right corner
    PositionWindow();
    
    // Create timer to auto-close after 2 seconds
    m_timer = new wxTimer(this, ID_NOTIFICATION_TIMER);
    m_timer->Start(2000, wxTIMER_ONE_SHOT); // 2 seconds, one-shot
    
    // Show the window
    Show();
}

NotificationPopup::~NotificationPopup() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
    }
}

void NotificationPopup::OnTimer(wxTimerEvent& event) {
    // Close the popup after timeout
    Close();
}

void NotificationPopup::OnClose(wxCloseEvent& event) {
    Destroy();
}

void NotificationPopup::PositionWindow() {
    // Get screen dimensions
    wxSize screenSize = wxGetDisplaySize();
    wxSize windowSize = GetSize();
    
    // Position in bottom-right corner with some margin
    int x = screenSize.GetWidth() - windowSize.GetWidth() - 20;
    int y = screenSize.GetHeight() - windowSize.GetHeight() - 50; // Account for taskbar
    
    SetPosition(wxPoint(x, y));
}

// ClipboardFrame implementation
ClipboardFrame::ClipboardFrame()
    : wxFrame(NULL, wxID_ANY, wxT("Clipboard Manager"), 
              wxDefaultPosition, wxSize(800, 600)),
      m_taskBarIcon(nullptr),
      m_listCtrl(nullptr),
      m_timer(nullptr),
      m_clearButton(nullptr),
      m_copyButton(nullptr),
      m_nextId(1),
      m_keyboardHook(NULL) {
    
    try {
        // Enable logging to file for debugging
        wxLog::SetActiveTarget(new wxLogStderr());
        wxLogMessage(wxT("Starting ClipboardFrame constructor"));

        // Create system tray icon
        m_taskBarIcon = new ClipboardTaskBarIcon(this);
        
        // Create main panel
        wxPanel* panel = new wxPanel(this, wxID_ANY);
        if (!panel) {
            wxLogError(wxT("Failed to create main panel"));
            return;
        }
        
        // Create list control for clipboard entries
        m_listCtrl = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, 
                                    wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
        if (!m_listCtrl) {
            wxLogError(wxT("Failed to create list control"));
            return;
        }
        
        // Add columns
        m_listCtrl->AppendColumn(wxT("Time"), wxLIST_FORMAT_LEFT, 150);
        m_listCtrl->AppendColumn(wxT("Type"), wxLIST_FORMAT_LEFT, 80);
        m_listCtrl->AppendColumn(wxT("Content"), wxLIST_FORMAT_LEFT, 500);
        
        // Create buttons
        m_clearButton = new wxButton(panel, ID_CLEAR_ALL, wxT("Clear All"));
        m_copyButton = new wxButton(panel, ID_COPY_SELECTED, wxT("Copy Selected"));
        
        if (!m_clearButton || !m_copyButton) {
            wxLogError(wxT("Failed to create buttons"));
            return;
        }
        
        // Layout
        wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
        buttonSizer->Add(m_clearButton, 0, wxALL, 5);
        buttonSizer->Add(m_copyButton, 0, wxALL, 5);
        
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(m_listCtrl, 1, wxEXPAND | wxALL, 5);
        mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
        
        panel->SetSizer(mainSizer);
        
        // Create timer for clipboard monitoring
        m_timer = new wxTimer(this, ID_TIMER);
        if (!m_timer) {
            wxLogError(wxT("Failed to create timer"));
            return;
        }
        
        if (!m_timer->Start(500)) { // Check every 500ms (0.5 seconds) for more responsive detection
            wxLogError(wxT("Failed to start timer"));
            return;
        }
        
        wxLogMessage(wxT("Timer started successfully"));
        
        // Load existing history
        LoadFromFile();
        
        // Get initial clipboard content
        m_lastClipboardContent = GetClipboardText();
        
        wxLogMessage(wxT("Constructor completed successfully"));
        
        // Start minimized to system tray
        Hide();
        
        wxLogMessage(wxT("Application started and minimized to system tray"));
    }
    catch (const std::exception& e) {
        wxString errorMsg = wxString::Format(wxT("Exception in constructor: %s"), e.what());
        wxLogError(errorMsg);
        wxMessageBox(errorMsg, wxT("Error"), wxOK | wxICON_ERROR);
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in constructor"));
        wxMessageBox(wxT("Unknown error occurred during initialization"), wxT("Error"), wxOK | wxICON_ERROR);
    }
}

ClipboardFrame::~ClipboardFrame() {
    if (m_timer) {
        m_timer->Stop();
        delete m_timer;
    }
    
    if (m_taskBarIcon) {
        delete m_taskBarIcon;
    }
    
    SaveToFile();
}

void ClipboardFrame::OnClose(wxCloseEvent& event) {
    if (event.CanVeto()) {
        // Hide to system tray instead of closing
        Hide();
        event.Veto();
    } else {
        // Force close
        SaveToFile();
        Destroy();
    }
}

void ClipboardFrame::OnIconize(wxIconizeEvent& event) {
    if (event.IsIconized()) {
        // Hide to system tray when minimized
        Hide();
        wxLogMessage(wxT("Application minimized to system tray"));
    }
}

void ClipboardFrame::OnTimer(wxTimerEvent& event) {
    CheckClipboard();
}

void ClipboardFrame::OnClearAll(wxCommandEvent& event) {
    if (wxMessageBox(wxT("Clear all clipboard history?"), 
                     wxT("Confirm"), wxYES_NO | wxICON_QUESTION) == wxYES) {
        m_entries.clear();
        m_listCtrl->DeleteAllItems();
        SaveToFile();
    }
}

void ClipboardFrame::OnCopySelected(wxCommandEvent& event) {
    long selectedItem = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem != -1 && selectedItem < (long)m_entries.size()) {
        const ClipboardEntry& entry = m_entries[selectedItem];
        
        if (entry.type == wxT("Image")) {
            // Copy image to clipboard
            CopyImageToClipboard(entry.imagePath);
        } else {
            // Copy text to clipboard
            if (wxTheClipboard->Open()) {
                wxTheClipboard->SetData(new wxTextDataObject(entry.content));
                wxTheClipboard->Close();
                m_lastClipboardContent = entry.content; // Prevent re-adding
            }
        }
    }
}

void ClipboardFrame::OnItemActivated(wxListEvent& event) {
    // Double-click to copy
    wxCommandEvent cmdEvent;
    OnCopySelected(cmdEvent);
}

void ClipboardFrame::CheckClipboard() {
    try {
        wxString dataType = DetermineDataType();
        
        if (dataType == wxT("Image")) {
            // Handle image clipboard content
            wxBitmap bitmap = GetClipboardBitmap();
            if (bitmap.IsOk()) {
                // Calculate hash to detect duplicate images
                wxString currentImageHash = CalculateImageHash(bitmap);
                
                // Only add if it's different from the last image
                if (currentImageHash != m_lastImageHash) {
                    ClipboardEntry entry;
                    entry.type = wxT("Image");
                    entry.timestamp = wxDateTime::Now();
                    entry.id = m_nextId++;
                    entry.imageSize = wxSize(bitmap.GetWidth(), bitmap.GetHeight());
                    
                    // Save image to file
                    entry.imagePath = SaveImageToFile(bitmap, entry.id);
                    entry.content = wxString::Format(wxT("Image (%dx%d)"), 
                                                    bitmap.GetWidth(), bitmap.GetHeight());
                    
                    AddClipboardEntry(entry);
                    SaveToFile();
                    
                    // Update last image hash
                    m_lastImageHash = currentImageHash;
                    
                    // Show notification popup
                    new NotificationPopup(this, wxT("Image Copied"), entry.content, true);
                    
                    wxLogMessage(wxT("Added image entry: %s"), entry.content);
                } else {
                    wxLogMessage(wxT("Skipping duplicate image"));
                }
            }
        } else {
            // Handle text clipboard content
            wxString currentContent = GetClipboardText();
            
            // Simple approach: Only save to history, no automatic notifications for text
            // This eliminates the selection vs copy problem entirely
            if (!currentContent.IsEmpty() && 
                currentContent != m_lastClipboardContent && 
                currentContent.Length() > 3) { // Minimum 4 characters
                
                // Skip if it's just whitespace
                wxString trimmedContent = currentContent;
                trimmedContent.Trim().Trim(false);
                if (trimmedContent.IsEmpty() || trimmedContent.Length() < 3) {
                    return;
                }
                ClipboardEntry entry;
                entry.content = currentContent;
                entry.type = dataType;
                entry.timestamp = wxDateTime::Now();
                entry.id = m_nextId++;
                
                AddClipboardEntry(entry);
                m_lastClipboardContent = currentContent;
                
                // Clear image hash when text is copied (different clipboard content type)
                m_lastImageHash.Clear();
                
                SaveToFile();
                
                // Show notification popup for text content
                new NotificationPopup(this, wxT("Text Copied"), entry.content, false);
                
                wxLogMessage(wxT("Added clipboard entry: %s"), entry.content.Left(50));
            }
        }
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in CheckClipboard: %s"), e.what());
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in CheckClipboard"));
    }
}

wxString ClipboardFrame::GetClipboardText() {
    wxString text;
    try {
        if (wxTheClipboard && wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
                wxTextDataObject data;
                if (wxTheClipboard->GetData(data)) {
                    text = data.GetText();
                }
            }
            wxTheClipboard->Close();
        } else {
            wxLogError(wxT("Failed to open clipboard for reading"));
        }
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in GetClipboardText: %s"), e.what());
        if (wxTheClipboard && wxTheClipboard->IsOpened()) {
            wxTheClipboard->Close();
        }
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in GetClipboardText"));
        if (wxTheClipboard && wxTheClipboard->IsOpened()) {
            wxTheClipboard->Close();
        }
    }
    return text;
}

wxBitmap ClipboardFrame::GetClipboardBitmap() {
    wxBitmap bitmap;
    try {
        if (wxTheClipboard && wxTheClipboard->Open()) {
            if (wxTheClipboard->IsSupported(wxDF_BITMAP)) {
                wxBitmapDataObject data;
                if (wxTheClipboard->GetData(data)) {
                    bitmap = data.GetBitmap();
                }
            }
            wxTheClipboard->Close();
        } else {
            wxLogError(wxT("Failed to open clipboard for bitmap reading"));
        }
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in GetClipboardBitmap: %s"), e.what());
        if (wxTheClipboard && wxTheClipboard->IsOpened()) {
            wxTheClipboard->Close();
        }
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in GetClipboardBitmap"));
        if (wxTheClipboard && wxTheClipboard->IsOpened()) {
            wxTheClipboard->Close();
        }
    }
    return bitmap;
}

wxString ClipboardFrame::SaveImageToFile(const wxBitmap& bitmap, size_t id) {
    try {
        // Create images directory if it doesn't exist
        wxString imageDir = wxT("clipboard_images");
        if (!wxDirExists(imageDir)) {
            wxMkdir(imageDir);
        }
        
        // Generate filename with timestamp and id
        wxString filename = wxString::Format(wxT("%s/image_%lu_%s.png"), 
                                           imageDir,
                                           (unsigned long)id,
                                           wxDateTime::Now().Format(wxT("%Y%m%d_%H%M%S")));
        
        // Convert bitmap to image and save as PNG
        wxImage image = bitmap.ConvertToImage();
        if (image.IsOk() && image.SaveFile(filename, wxBITMAP_TYPE_PNG)) {
            wxLogMessage(wxT("Saved image to: %s"), filename);
            return filename;
        } else {
            wxLogError(wxT("Failed to save image to: %s"), filename);
        }
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in SaveImageToFile: %s"), e.what());
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in SaveImageToFile"));
    }
    return wxEmptyString;
}

wxString ClipboardFrame::DetermineDataType() {
    wxString type = wxT("Text");
    try {
        if (!wxTheClipboard || !wxTheClipboard->Open()) {
            wxLogError(wxT("Failed to open clipboard for type determination"));
            return wxT("Unknown");
        }
        
        if (wxTheClipboard->IsSupported(wxDF_BITMAP)) {
            type = wxT("Image");
        } else if (wxTheClipboard->IsSupported(wxDF_FILENAME)) {
            type = wxT("File");
        } else if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
            type = wxT("Text");
        }
        
        wxTheClipboard->Close();
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in DetermineDataType: %s"), e.what());
        if (wxTheClipboard && wxTheClipboard->IsOpened()) {
            wxTheClipboard->Close();
        }
        type = wxT("Unknown");
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in DetermineDataType"));
        if (wxTheClipboard && wxTheClipboard->IsOpened()) {
            wxTheClipboard->Close();
        }
        type = wxT("Unknown");
    }
    return type;
}

wxString ClipboardFrame::CalculateImageHash(const wxBitmap& bitmap) {
    try {
        if (!bitmap.IsOk()) {
            return wxEmptyString;
        }
        
        // Convert bitmap to image for pixel access
        wxImage image = bitmap.ConvertToImage();
        if (!image.IsOk()) {
            return wxEmptyString;
        }
        
        // Simple hash calculation using image dimensions and a sampling of pixels
        unsigned long hash = 0;
        int width = image.GetWidth();
        int height = image.GetHeight();
        
        // Include dimensions in hash
        hash ^= (width << 16) | height;
        
        // Sample pixels at regular intervals (to avoid processing every pixel for performance)
        int stepX = width > 32 ? width / 32 : 1;
        int stepY = height > 32 ? height / 32 : 1;
        
        unsigned char* data = image.GetData();
        if (!data) {
            return wxEmptyString;
        }
        
        for (int y = 0; y < height; y += stepY) {
            for (int x = 0; x < width; x += stepX) {
                int pos = (y * width + x) * 3; // RGB format
                if (pos + 2 < width * height * 3) {
                    unsigned char r = data[pos];
                    unsigned char g = data[pos + 1];
                    unsigned char b = data[pos + 2];
                    
                    // Simple hash combination
                    hash ^= (r << 16) | (g << 8) | b;
                    hash = (hash << 1) | (hash >> 31); // Rotate left by 1
                }
            }
        }
        
        // Convert hash to hex string
        return wxString::Format(wxT("%08lx"), hash);
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in CalculateImageHash: %s"), e.what());
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in CalculateImageHash"));
    }
    return wxEmptyString;
}

void ClipboardFrame::CopyImageToClipboard(const wxString& imagePath) {
    try {
        if (!wxFileExists(imagePath)) {
            wxLogError(wxT("Image file not found: %s"), imagePath);
            return;
        }
        
        // Load image from file
        wxImage image(imagePath, wxBITMAP_TYPE_PNG);
        if (!image.IsOk()) {
            wxLogError(wxT("Failed to load image: %s"), imagePath);
            return;
        }
        
        // Convert to bitmap and copy to clipboard
        wxBitmap bitmap(image);
        if (bitmap.IsOk() && wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxBitmapDataObject(bitmap));
            wxTheClipboard->Close();
            wxLogMessage(wxT("Copied image to clipboard: %s"), imagePath);
        } else {
            wxLogError(wxT("Failed to copy image to clipboard: %s"), imagePath);
        }
    }
    catch (const std::exception& e) {
        wxLogError(wxT("Exception in CopyImageToClipboard: %s"), e.what());
    }
    catch (...) {
        wxLogError(wxT("Unknown exception in CopyImageToClipboard"));
    }
}

void ClipboardFrame::AddClipboardEntry(const ClipboardEntry& entry) {
    // Add to internal storage
    m_entries.insert(m_entries.begin(), entry); // Add at beginning (most recent first)
    
    // Limit to 1000 entries
    if (m_entries.size() > 1000) {
        m_entries.resize(1000);
    }
    
    // Add to list control
    long index = m_listCtrl->InsertItem(0, entry.timestamp.Format(wxT("%Y-%m-%d %H:%M:%S")));
    m_listCtrl->SetItem(index, 1, entry.type);
    
    // Truncate long content for display
    wxString displayContent = entry.content;
    if (displayContent.Length() > 100) {
        displayContent = displayContent.Left(100) + wxT("...");
    }
    // Replace newlines with spaces for display
    displayContent.Replace(wxT("\n"), wxT(" "));
    displayContent.Replace(wxT("\r"), wxT(" "));
    
    m_listCtrl->SetItem(index, 2, displayContent);
}

void ClipboardFrame::ShowFrame() {
    // Restore window if it's iconized (minimized)
    if (IsIconized()) {
        Iconize(false);
    }
    
    // Show and raise the window
    Show();
    Raise();
    RequestUserAttention();
    
    wxLogMessage(wxT("Window restored from system tray"));
}

void ClipboardFrame::HideFrame() {
    Hide();
}

void ClipboardFrame::SaveToFile() {
    wxTextFile file;
    
    if (wxFileName::FileExists(LOG_FILE)) {
        file.Open(LOG_FILE);
        file.Clear();
    } else {
        file.Create(LOG_FILE);
    }
    
    for (const auto& entry : m_entries) {
        wxString line = wxString::Format(wxT("%s|%s|%s"), 
                                       entry.timestamp.Format(wxT("%Y-%m-%d %H:%M:%S")),
                                       entry.type,
                                       entry.content);
        // Escape newlines for storage
        line.Replace(wxT("\n"), wxT("\\n"));
        line.Replace(wxT("\r"), wxT("\\r"));
        file.AddLine(line);
    }
    
    file.Write();
}

void ClipboardFrame::LoadFromFile() {
    if (!wxFileName::FileExists(LOG_FILE)) {
        return;
    }
    
    wxTextFile file(LOG_FILE);
    if (!file.Open()) {
        return;
    }
    
    m_entries.clear();
    m_listCtrl->DeleteAllItems();
    
    for (size_t i = 0; i < file.GetLineCount(); ++i) {
        wxString line = file.GetLine(i);
        
        // Parse: timestamp|type|content
        wxStringTokenizer tokenizer(line, wxT("|"));
        if (tokenizer.CountTokens() >= 3) {
            ClipboardEntry entry;
            
            wxString timeStr = tokenizer.GetNextToken();
            entry.timestamp.ParseFormat(timeStr, wxT("%Y-%m-%d %H:%M:%S"));
            
            entry.type = tokenizer.GetNextToken();
            
            entry.content = tokenizer.GetString(); // Get remaining content
            // Unescape newlines
            entry.content.Replace(wxT("\\n"), wxT("\n"));
            entry.content.Replace(wxT("\\r"), wxT("\r"));
            
            entry.id = m_nextId++;
            
            m_entries.push_back(entry);
        }
    }
    
    // Update list control
    for (const auto& entry : m_entries) {
        long index = m_listCtrl->InsertItem(m_listCtrl->GetItemCount(), 
                                          entry.timestamp.Format(wxT("%Y-%m-%d %H:%M:%S")));
        m_listCtrl->SetItem(index, 1, entry.type);
        
        wxString displayContent = entry.content;
        if (displayContent.Length() > 100) {
            displayContent = displayContent.Left(100) + wxT("...");
        }
        displayContent.Replace(wxT("\n"), wxT(" "));
        displayContent.Replace(wxT("\r"), wxT(" "));
        
        m_listCtrl->SetItem(index, 2, displayContent);
    }
}

// Keyboard hook implementation
LRESULT CALLBACK ClipboardFrame::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        
        if (wParam == WM_KEYDOWN) {
            // Check for Ctrl+C (Ctrl key must be down and C key pressed)
            if (pKeyboard->vkCode == 'C' && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
                s_ctrlCPressed = true;
                s_lastCtrlCTime = wxDateTime::Now();
                wxLogMessage(wxT("Ctrl+C detected!"));
                
                if (s_instance) {
                    s_instance->OnCtrlCPressed();
                }
            }
        }
    }
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

bool ClipboardFrame::InstallKeyboardHook() {
    s_instance = this;
    m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (m_keyboardHook == NULL) {
        wxLogError(wxT("Failed to install keyboard hook"));
        return false;
    }
    wxLogMessage(wxT("Keyboard hook installed successfully"));
    return true;
}

void ClipboardFrame::UninstallKeyboardHook() {
    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = NULL;
        wxLogMessage(wxT("Keyboard hook uninstalled"));
    }
    s_instance = nullptr;
}

void ClipboardFrame::OnCtrlCPressed() {
    // This method is called when Ctrl+C is detected
    // We can use this to mark the next clipboard change as intentional
    wxLogMessage(wxT("Intentional copy detected - next clipboard change should show notification"));
}

// ClipboardApp implementation
bool ClipboardApp::OnInit() {
    if (!wxApp::OnInit()) {
        return false;
    }
    
    // Temporarily disable system tray check for testing
    // Check if system tray is available
    // if (!wxTaskBarIcon::IsAvailable()) {
    //     wxMessageBox(wxT("System tray is not available on this system."),
    //                  wxT("Error"), wxOK | wxICON_ERROR);
    //     return false;
    // }
    
    m_frame = new ClipboardFrame();
    
    return true;
}

int ClipboardApp::OnExit() {
    return wxApp::OnExit();
}

wxIMPLEMENT_APP(ClipboardApp);
