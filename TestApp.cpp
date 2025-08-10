#include <wx/wx.h>
#include <iostream>

class TestApp : public wxApp {
public:
    virtual bool OnInit() override {
        std::cout << "TestApp::OnInit() called" << std::endl;
        
        if (!wxApp::OnInit()) {
            std::cout << "wxApp::OnInit() failed" << std::endl;
            return false;
        }
        
        std::cout << "Creating simple frame..." << std::endl;
        wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "Test Frame");
        if (!frame) {
            std::cout << "Failed to create frame" << std::endl;
            return false;
        }
        
        frame->Show();
        std::cout << "Frame shown successfully" << std::endl;
        
        return true;
    }
    
    virtual int OnExit() override {
        std::cout << "TestApp::OnExit() called" << std::endl;
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(TestApp);
