#pragma once
#include "wx/wx.h"
#include "wx/mediactrl.h"
#include <vector>
class cuppMediaCtrl : public wxMediaCtrl
{
	typedef wxMediaCtrl Inherited;
public:
	wxTextCtrl* tagBox;
	std::vector<std::string> Tags;
	cuppMediaCtrl(wxWindow* parent, wxString filename, bool looping);
	bool LoadFile(wxString filename);
	void SetLoop(bool loop);
	bool GetLoop();
	void OnMediaLoaded(wxMediaEvent& event);
	void OnMediaFinished(wxMediaEvent& event);
	void OnEnter(wxMouseEvent& event);

protected:
	enum MenuIds {
		IDM_MEDIACONTROL = wxID_HIGHEST + 100
	};

private:
	bool isLooping;

	DECLARE_EVENT_TABLE()
};
