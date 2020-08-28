
// NoiServiceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NoiService.h"
#include "NoiServiceDlg.h"
#include "afxdialogex.h"
#include "WebPageDownloader.h"

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

const CString _ID = _T("id");
const CString _ERROR = _T("error");
const CString _RESULT = _T("result");

CNoiServiceDlg::CNoiServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNoiServiceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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

	GetDlgItem(IDC_EDIT_SERVER)->SetWindowText(_T("http://localhost:8080/"));
		
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	StartTimer(GET_INQUIRY);

	return TRUE;  // return TRUE  unless you set the focus to a control
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

CString CNoiServiceDlg::GetServer()
{
	CString sServer;
	GetDlgItem(IDC_EDIT_SERVER)->GetWindowText(sServer);

	if (sServer.IsEmpty())
	{
		AfxMessageBox(_T("Please enter host"));
		return "";
	}

	return sServer;
}

void CNoiServiceDlg::OnBnClickedGetInquiry()
{
	CString sFullUrl = GetServer() + "/getInquiry";

	CString sError;
	CString sResponse;
	m_StringMap.RemoveAll();

	if (CWebPageDownloader::RestApi(_T("POST"), sFullUrl, "", sResponse, sError))
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

void CNoiServiceDlg::GetResponseItems(CString& sId, CString& sError)
{
	m_StringMap.Lookup(_ID, sId);
	m_StringMap.Lookup(_ERROR, sError);
}

void CNoiServiceDlg::GetResponseItem(const CString sName, CString& sValue)
{
	m_StringMap.Lookup(sName, sValue);
}



void CNoiServiceDlg::OnBnClickedSetResult()
{
	CString sFullUrl = GetServer() + "/setResult";

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
	strRequest.Format(_T("{ \"id\": %s,\"result\" : \"%s\" }"), sId, sResult);

	CString sResponse;
	CString sError;

	if (CWebPageDownloader::RestApi(_T("PUT"), sFullUrl, strRequest, sResponse, sError))
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
			}
			else if (!sError.IsEmpty())
			{
				KillTimer(GET_INQUIRY);
				StartTimer(GET_INQUIRY);
			}
		}
		break;

		case WORKING_ON_RESULT:
		{		
			
			Processing();

			KillTimer(WORKING_ON_RESULT);
			StartTimer(SET_RESULT);
		}
		break;

		case SET_RESULT:
		{				
			OnBnClickedSetResult();

			KillTimer(SET_RESULT);
			StartTimer(GET_INQUIRY);
		}
		break;
	}

	CDialogEx::OnTimer(nIDEvent);

}