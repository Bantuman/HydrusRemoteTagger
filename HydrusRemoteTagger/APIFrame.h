#ifndef __APIF_H
#define __APIF_H
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif



class APIFrame : public wxFrame
{
public:
    APIFrame();
    wxTextCtrl* apiUrlBox;
    wxTextCtrl* apiKeyBox;
    
private:
    //void OnSetAPI(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnConfirm(wxCommandEvent& event);
};
#endif