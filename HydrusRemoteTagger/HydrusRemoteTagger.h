//#include <string>
#include <wx/wxprec.h>
#include <wx/mediactrl.h>
#include <wx/stattext.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class MyFrame : public wxFrame
{
public:
    MyFrame();
    ~MyFrame();

private:

    void OnSetAPI(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};
