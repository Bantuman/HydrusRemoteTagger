#include <cpr/cpr.h>
#include "HydrusRemoteTagger.h"
#include "cupMediaCtrl.h"
#include "wxImagePanel.h"
#include "APIFrame.h"

#include <wx/splitter.h>
#include <thread>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#define hydrusPATHFILE std::pair<std::wstring, std::string>

class MyApp : public wxApp
{
private:
    void OnCommit(wxCommandEvent& event);
    void OnSkip(wxCommandEvent& event);
public:
    std::string curlReadBuffer;
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    virtual bool OnInit();
};

enum
{
    ID_SetAPI = wxID_HIGHEST + 1,
    ID_CancelConf = wxID_HIGHEST + 2,
    ID_ConfirmConf = wxID_HIGHEST + 3,
    ID_MouseEnterMedia = wxID_HIGHEST + 4,
    ID_CommitTag = wxID_HIGHEST + 5,
    ID_SkipTag = wxID_HIGHEST + 6,
};

struct MemoryStruct {
    char* memory;
    size_t size;
};/*
wxBoxSizer* sizerTotal;*/


std::vector<hydrusPATHFILE> currentFiles;

wxBoxSizer* sizerTop;
wxBoxSizer* sizerBot;
wxPanel* holderBottom;
wxPanel* holderTop;
wxStaticText* commitsText;
wxTextCtrl* tagInput;
MyFrame* globalFrame;
wxClassInfo* mediaClassInfo;
std::string apiKey = "d9047f897547ea8964244324ff908efdbded17927cfa90b5f4a322e2fe5ab0fe";
std::string apiUrl = "http://127.0.0.1:45869/";
bool accessKeyValid = false;

wxIMPLEMENT_APP(MyApp);

size_t MyApp::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void RefreshAccess()
{
    cpr::Response response = cpr::Get(cpr::Url{ apiUrl + "verify_access_key" },
        cpr::Header{ { "Hydrus-Client-API-Access-Key", apiKey } });
    accessKeyValid = response.status_code == 200;
    globalFrame->SetStatusText(accessKeyValid ? "(Connected) Hydrus Remote Tagger" : "(No connection) Hydrus Remote Tagger");
}

std::wstring ExePath() {
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    return std::wstring(buffer).substr(0, pos);
}

std::wstring StringToWString(const std::string& s)
{
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}

void _wremoveImages()
{
    std::vector<hydrusPATHFILE> temp(currentFiles);
    for (hydrusPATHFILE file : temp)
    {
        _wremove(file.first.c_str());
    }
}

void ClearImages()
{
    std::thread removeImagesIO(_wremoveImages);
    holderTop->DestroyChildren();
    currentFiles.clear();
    removeImagesIO.join();
    commitsText->SetForegroundColour(wxColor(0, 0, 0));
    commitsText->SetLabelText("...");
}

void RefreshImages()
{
    cpr::Response filesFetch = cpr::Get(cpr::Url{ apiUrl + "get_files/search_files" },
        cpr::Header{ { "Hydrus-Client-API-Access-Key", apiKey} },
        cpr::Parameters{ { "tags", "[\"system:no tags\",\"system:limit=10\"]" }, {"file_sort_type", "4"} }
    );
    int numFiles = 1;
    rapidjson::Document doc;
    doc.Parse(filesFetch.text.c_str());
    const rapidjson::Value& ids = doc["file_ids"];
    int processedFiles = 0;
    for (rapidjson::SizeType i = 0; i < ids.Size(); ++i)
    {
        int file_id = ids[i].GetInt();
        cpr::Response fileFetch = cpr::Get(cpr::Url{ apiUrl + "get_files/file" },
            cpr::Header{ { "Hydrus-Client-API-Access-Key", apiKey} },
            cpr::Parameters{ { "file_id", std::to_string(file_id) } }
        );
        std::ofstream outfile;
        std::wstring name = L"thumb" + std::to_wstring(file_id);
        std::string mime = fileFetch.header["Content-Type"];
        if (mime == "image/png")
        {
            name += L".png";
        }
        else if (mime == "image/jpeg")
        {
            name += L".jpg";
        }
        else if (mime == "image/gif")
        {
            name += L".gif";
        }
        else if (mime == "image/webp")
        {
            name += L".webp";
        }
        else if (mime == "video/mp4")
        {
            name += L".mp4";
        }
        else if (mime == "video/webm")
        {
            name += L".webm";
        }
        else
        {
            continue;
        }
        std::wstring fullpath = ExePath() + L"\\" + name;
        outfile.open(fullpath, std::ios_base::binary | std::ios_base::out);
        outfile.write(fileFetch.text.c_str(), fileFetch.downloaded_bytes);
        outfile.flush();
        outfile.close();
        cuppMediaCtrl* mediaC = new cuppMediaCtrl(holderTop, fullpath, true); //fullpath
        currentFiles.push_back(hydrusPATHFILE{fullpath, std::to_string(file_id)});
        processedFiles++;
        sizerTop->Add(mediaC, 1, wxEXPAND | wxALL, 0);
        if (processedFiles >= numFiles)
        {
            break;
        }
    }
    sizerTop->RepositionChildren(wxSize(148, 148));
}

bool MyApp::OnInit()
{
    wxInitAllImageHandlers();
    mediaClassInfo = wxClassInfo::FindClass("cuppMediaCtrl");
    globalFrame = new MyFrame();
    wxSplitterWindow* mySplitter = new wxSplitterWindow(globalFrame, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);
    
    sizerTop = new wxBoxSizer(wxHORIZONTAL);
    sizerBot = new wxBoxSizer(wxHORIZONTAL);

    holderBottom = new wxPanel(mySplitter);
    holderBottom->SetBackgroundColour(wxColor(100, 100, 100));

    holderBottom->SetSizer(sizerBot);
    holderTop = new wxPanel(mySplitter);
    holderTop->SetBackgroundColour(wxColor(0, 0, 0));

    mySplitter->SetMinimumPaneSize(200);
    mySplitter->SplitHorizontally(holderTop, holderBottom);
    mySplitter->SetSashGravity(0);

    tagInput = new wxTextCtrl(holderBottom, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE, wxDefaultValidator, "Test");
    wxButton* commitButton = new wxButton(holderBottom, ID_CommitTag, "Commit", wxPoint(0, 0), wxSize(100, 24));
    commitButton->SetForegroundColour(wxColor(30, 180, 30));
    wxButton* skipButton = new wxButton(holderBottom, ID_SkipTag, "Skip", wxPoint(0, 24), wxSize(100, 24));
    skipButton->SetForegroundColour(wxColor(200, 50, 50));
    commitsText = new wxStaticText(holderBottom, wxID_ANY, "...", wxPoint(0, 48), wxSize(100, 18), wxALIGN_CENTER_HORIZONTAL);
    Connect(ID_CommitTag, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MyApp::OnCommit);
    Connect(ID_SkipTag, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&MyApp::OnSkip);
    RefreshAccess();
    if (accessKeyValid)
    {
        RefreshImages();
    }
    sizerBot->Add(tagInput, 1, wxEXPAND | wxLEFT, 100);
    holderTop->SetSizer(sizerTop);
    globalFrame->Show(true);
    return true;
}

void MyApp::OnSkip(wxCommandEvent& event)
{
    tagInput->SetValue("");
    ClearImages();
    RefreshImages();
}


void MyApp::OnCommit(wxCommandEvent& event)
{
    std::vector<std::pair<std::string, std::string>> finalVector;
    std::string pushstr = "[";
    for (hydrusPATHFILE file : currentFiles)
    {
        std::string hash;
        pushstr += (file.second + ", ");
    }
    pushstr = pushstr.substr(0, pushstr.length() - 2);
    cpr::Response getMetadata = cpr::Get(cpr::Url{ apiUrl + "get_files/file_metadata" },
        cpr::Header{ { "Hydrus-Client-API-Access-Key", apiKey} },
        cpr::Parameters{ { "file_ids", pushstr + "]"}, {"only_return_identifiers", "true"} }
    );
    if (getMetadata.status_code == 200)
    {
        rapidjson::Document doc;
        doc.Parse(getMetadata.text.c_str());
        const rapidjson::Value& fileData = doc["metadata"];
        for (rapidjson::SizeType i = 0; i < fileData.Size(); ++i)
        {
            std::string tags = "";
            for (int line = 0; line < tagInput->GetNumberOfLines(); ++line)
            {
                tags += "\"" + tagInput->GetLineText(line) + "\", ";
            }
            tags = tags.substr(0, tags.length() - 2);

            std::string hash = fileData[i]["hash"].GetString();
            std::string body = "{\"hash\" : \"" + hash + "\",\"service_names_to_tags\":{\"my tags\":[" + tags + "]}}";
            cpr::Response postTag = cpr::Post(cpr::Url{ apiUrl + "add_tags/add_tags" },
                cpr::Header{ {"Content-Type", "application/json"}, {"Hydrus-Client-API-Access-Key", apiKey}},
                cpr::Body{body}
            );

            if (postTag.status_code == 200)
            {
                ClearImages();
                RefreshImages();
                tagInput->SetValue("");
                commitsText->SetForegroundColour(wxColor(30, 180, 30));
                commitsText->SetLabelText("Added tags!");
            }
            else
            {
                commitsText->SetForegroundColour(wxColor(200, 50, 50));
                commitsText->SetLabelText(postTag.status_code + " Failed!");
            }
        }
    }
}

MyFrame::MyFrame()
    : wxFrame(NULL, wxID_ANY, "Hydrus Remote Tagger", wxDefaultPosition, wxSize(800, 600))
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_SetAPI, "&Settings\tCtrl-H",
        "Set API URL & Access Key");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);
    CreateStatusBar();
    SetStatusText("(No connection) Hydrus Remote Tagger");
    Bind(wxEVT_MENU, &MyFrame::OnSetAPI, this, ID_SetAPI);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

MyFrame::~MyFrame() {
    ClearImages();
}

void MyFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("Written by Bantu",
        "About Hydrus Remote Tagger", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnSetAPI(wxCommandEvent& event)
{
    APIFrame* inputFrame = new APIFrame();
    globalFrame->AddChild(inputFrame);
    inputFrame->Show();
}

void APIFrame::OnCancel(wxCommandEvent& event)
{
    Close(true);
}

void APIFrame::OnConfirm(wxCommandEvent& event)
{
    apiUrl = this->apiUrlBox->GetValue();
    apiKey = this->apiKeyBox->GetValue();
    RefreshAccess();
    Close(true);
    ClearImages();
    if (accessKeyValid)
    {
        RefreshImages();
    }
}

APIFrame::APIFrame()
    : wxFrame(NULL, wxID_ANY, "Configure Client API", globalFrame->GetPosition(), wxSize(600, 196), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
    wxStaticText* apiUrlBoxLabel = new wxStaticText(this, wxID_ANY, "API URL", wxPoint(0, 4), wxSize(600, 18), wxALIGN_CENTER_HORIZONTAL);
    this->apiUrlBox = new wxTextCtrl(this, wxID_ANY, apiUrl, wxPoint(20, 24), wxSize(550, 24), wxTE_AUTO_URL | wxTE_LEFT | wxTE_DONTWRAP & ~wxHSCROLL, wxDefaultValidator, "Test");
    wxStaticText* apiKeyBoxLabel = new wxStaticText(this, wxID_ANY, "Access Key", wxPoint(0, 52), wxSize(600, 18), wxALIGN_CENTER_HORIZONTAL);
    this->apiKeyBox = new wxTextCtrl(this, wxID_ANY, apiKey, wxPoint(20, 72), wxSize(550, 24), wxTE_PASSWORD | wxTE_LEFT | wxTE_DONTWRAP & ~wxHSCROLL, wxDefaultValidator, "Test");
    wxButton* confirmButton = new wxButton(this, ID_ConfirmConf, "Confirm", wxPoint(175, 156 - 36), wxSize(100, 24));
    wxButton* cancelButton = new wxButton(this, ID_CancelConf, "Cancel", wxPoint(325, 156 - 36), wxSize(100, 24));
    Connect(ID_ConfirmConf, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&APIFrame::OnConfirm);
    Connect(ID_CancelConf, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&APIFrame::OnCancel);
}