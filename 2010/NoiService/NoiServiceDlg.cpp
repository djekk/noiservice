
// NoiServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NoiService.h"
#include "NoiServiceDlg.h"
#include "afxdialogex.h"
#include "WebPageDownloader.h"
#include "UiFields.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNoiServiceDlg dialog


#define GET_INQUIRY           132
#define SET_RESULT            131
#define WORKING_ON_RESULT     130
#define VALIDATION_SERVICE    134

#define DELAY_FOR_TIMER		5000

const CString _ID = _T("id");
const CString _ERROR = _T("error");
const CString _RESULT = _T("result");
const CString _OWNER = _T("owner");

const static CString PUT = _T("PUT");
const static CString POST = _T("POST");

CNoiServiceDlg::CNoiServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNoiServiceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CNoiServiceDlg::~CNoiServiceDlg()
{
	
}

void CNoiServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNoiServiceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNoiServiceDlg::OnBnClickedGetInquiry)
	ON_BN_CLICKED(IDOK2, &CNoiServiceDlg::OnBnClickedSetResult)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK3, &CNoiServiceDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDOK4, &CNoiServiceDlg::OnBnClickedStop)
END_MESSAGE_MAP()


// CNoiServiceDlg message handlers

BOOL CNoiServiceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	CUiFields::InitUiFiels(this);
		
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CUiFields::ValidateUiFields(this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CNoiServiceDlg::OnDestroy()
{
	CUiFields::SaveProfileProperty(this);
}


void CNoiServiceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CNoiServiceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CNoiServiceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CNoiServiceDlg::resolveJSON(CString response)
{
	response.Replace(_T("\r"), _T(""));
	response.Replace(_T("\n"), _T(""));
	response.Replace(_T("{"), _T(""));
	response.Replace(_T("}"), _T(""));
	response.TrimLeft();
	response.TrimRight();

	int index = 0;
	CString field;
	while (AfxExtractSubString(field, response, index, _T(',')))
	{
		CString subField;
		CString key;
		CString value;
		int subIndex = 0;
		while (AfxExtractSubString(subField, field, subIndex, _T(':')))
		{
			subField.Replace(_T("\""), _T(""));
			subField.TrimLeft();
			subField.TrimRight();
			if (subIndex == 0)
				key = subField;
			if (subIndex == 1)
				value = subField;
			++subIndex;
		}
		m_StringMap.SetAt(key, value);
		++index;
	}
}


void CNoiServiceDlg::OnBnClickedGetInquiry()
{
	CString sFullUrl = CUiFields::GetServer(this) + "/getInquiry";

	CString sError;
	CString sResponse;
	m_StringMap.RemoveAll();

	CString sOwner;
	GetDlgItem(IDC_EDIT_OWNER)->GetWindowText(sOwner);

	CString strFormData;
	strFormData.Format(_T("{ \"%s\": \"%s\" }"), 
		_OWNER, sOwner);

	if (CWebPageDownloader::RestApi(POST, sFullUrl, strFormData, sResponse, sError))
	{
		resolveJSON(sResponse);
		GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(sResponse);
	}
	else
	{
		m_StringMap.RemoveAll();
		m_StringMap.SetAt(_ERROR, sError);
		GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(sError);
	}
}


void CNoiServiceDlg::OnBnClickedSetResult()
{
	CString sFullUrl = CUiFields::GetServer(this) + "/setResult";

	CString sId;
	GetResponseItem(_ID, sId);
	if (sId.IsEmpty())
	{
		GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(_T("No data"));
		return;
	}

	CString sResult;
	GetResponseItem(_RESULT, sResult);
	if (sResult.IsEmpty())
	{
		GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(_T("No result value"));
		return;
	}

	CString strRequest;
	strRequest.Format(_T("{ \"%s\": %s,\"%s\" : \"%s\" }"),
		_ID, sId, _RESULT, sResult);

	CString sResponse;
	CString sError;

	if (CWebPageDownloader::RestApi(PUT, sFullUrl, strRequest, sResponse, sError))
	{
		resolveJSON(sResponse);
		GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(sResponse);
	}
	else
	{
		m_StringMap.SetAt(_ERROR, sError);
		GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(sError);
	}
}

void CNoiServiceDlg::GetResponseItems(CString& sId, CString& sError)
{
	m_StringMap.Lookup(_ID, sId);
	m_StringMap.Lookup(_ERROR, sError);
}

void CNoiServiceDlg::GetResponseItem(const CString sName, CString& sValue)
{
	m_StringMap.Lookup(sName, sValue);
}

void CNoiServiceDlg::StartTimer(int nID)
{
	int res = SetTimer(nID, 20, NULL);
	ASSERT(res == nID);
}


//vichislenie
void CNoiServiceDlg::Processing()
{
	m_StringMap.SetAt(_RESULT, "fdgsdfgsdfgsdfgsdfgsdf");
}

void CNoiServiceDlg::OnTimer(UINT nIDEvent)
{
	if (!CUiFields::ValidateUiFields(this))
	{
		OnBnClickedStop();
		return;
	}

	switch (nIDEvent)
	{		
		case GET_INQUIRY:
		{
			OnBnClickedGetInquiry();

			CString sError, sId;
			GetResponseItems(sId, sError);

			if (!sId.IsEmpty())
			{
				KillTimer(GET_INQUIRY);
				StartTimer(WORKING_ON_RESULT);
				SetTimer(WORKING_ON_RESULT, DELAY_FOR_TIMER, NULL);
			}
			else if (!sError.IsEmpty())
			{
				KillTimer(GET_INQUIRY);				
				StartTimer(GET_INQUIRY);
				SetTimer(GET_INQUIRY, DELAY_FOR_TIMER, NULL);
			}
		}
		break;

		case WORKING_ON_RESULT:
		{		
			
			Processing();

			KillTimer(WORKING_ON_RESULT);			
			StartTimer(SET_RESULT);
			SetTimer(SET_RESULT, DELAY_FOR_TIMER, NULL);
		}
		break;

		case SET_RESULT:
		{				
			OnBnClickedSetResult();

			KillTimer(SET_RESULT);			
			StartTimer(GET_INQUIRY);
			SetTimer(GET_INQUIRY, DELAY_FOR_TIMER, NULL);
		}
		break;
	}

	CDialogEx::OnTimer(nIDEvent);

}

void CNoiServiceDlg::OnBnClickedStart()
{
	if (!CUiFields::ValidateUiFields(this))
		return;

	GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(_T("Start"));
	StartTimer(GET_INQUIRY);
}

void CNoiServiceDlg::OnBnClickedStop()
{
	GetDlgItem(IDC_STATIC_RESSPONSE)->SetWindowText(_T("Stop"));
	KillTimer(GET_INQUIRY);
	KillTimer(WORKING_ON_RESULT);
	KillTimer(SET_RESULT);
}
