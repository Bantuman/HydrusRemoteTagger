#include "cupMediaCtrl.h"

BEGIN_EVENT_TABLE(cuppMediaCtrl, Inherited)
EVT_MEDIA_LOADED(IDM_MEDIACONTROL, OnMediaLoaded)
EVT_MEDIA_STOP(IDM_MEDIACONTROL, OnMediaFinished)
END_EVENT_TABLE()


cuppMediaCtrl::cuppMediaCtrl(wxWindow* parent, wxString filename = _T(""), bool looping = false) :
	wxMediaCtrl(parent, IDM_MEDIACONTROL, wxEmptyString, wxDefaultPosition, wxSize(148, 148), wxBORDER_SUNKEN)
{
	if (filename != _T(""))
		bool result = Load(filename.data());
	isLooping = looping;
	//tagBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString, this->GetPosition() + wxPoint(15, 50) , wxDefaultSize, wxTE_LEFT, wxDefaultValidator, "Test");
	//sizer->Add(tagBox, wxALIGN_TOP);
	//this->GetContainingSizer()->Add(tagBox);
	return;
}

bool cuppMediaCtrl::LoadFile(wxString filename)
{
	bool result = this->Load(filename.data());
	return result;
}

void cuppMediaCtrl::OnMediaLoaded(wxMediaEvent& event)
{
	Play();
}

void cuppMediaCtrl::OnMediaFinished(wxMediaEvent& event)
{ 
	if (isLooping)
		Play();
}

void cuppMediaCtrl::OnEnter(wxMouseEvent& event)
{
	wxMessageBox("Test", "test", wxOK | wxICON_INFORMATION);
}