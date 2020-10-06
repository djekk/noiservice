#include "stdafx.h"
#include "UiFields.h"
#include "resource.h"


CUiFields::CUiFields()
{
}


CUiFields::~CUiFields()
{
}

const static CString FILE_PROPERTY = _T("noiservice.ini");
const static CString DB_SETTINGS = _T("DB_SETTINGS");

const static CString SERVER_PROPERTY = _T("SERVER");
const static CString OWNER_PROPERTY = _T("OWNER");

void CUiFields::InitUiFiels(CWnd * pUi)
{
	TCHAR server[100];
	int a = GetPrivateProfileString(DB_SETTINGS, _T("SERVER"), _T(""), server, 100, FILE_PROPERTY);
	pUi->GetDlgItem(IDC_EDIT_SERVER)->SetWindowText(server);

	TCHAR owner[100];
	int b = GetPrivateProfileString(DB_SETTINGS, _T("OWNER"), _T(""), owner, 100, FILE_PROPERTY);
	pUi->GetDlgItem(IDC_EDIT_OWNER)->SetWindowText(owner);
}


bool CUiFields::ValidateUiFields(CWnd * pUi)
{
	CString s;
	pUi->GetDlgItem(IDC_EDIT_SERVER)->GetWindowText(s);

	if (s.IsEmpty())
	{
		AfxMessageBox(_T("Please enter host"));
		return FALSE;
	}

	pUi->GetDlgItem(IDC_EDIT_OWNER)->GetWindowText(s);

	if (s.IsEmpty())
	{
		AfxMessageBox(_T("Please enter owner name (calculator)"));
		return FALSE;
	}

	return TRUE;
}


void CUiFields::SaveProfileProperty(CWnd * pUi)
{
	CString sServer;
	pUi->GetDlgItem(IDC_EDIT_SERVER)->GetWindowText(sServer);

	if (!sServer.IsEmpty())
	{
		WritePrivateProfileString(DB_SETTINGS, SERVER_PROPERTY, sServer, FILE_PROPERTY);
	}

	CString sOwner;
	pUi->GetDlgItem(IDC_EDIT_OWNER)->GetWindowText(sOwner);
	if (!sOwner.IsEmpty())
	{
		WritePrivateProfileString(DB_SETTINGS, OWNER_PROPERTY, sOwner, FILE_PROPERTY);
	}
}


CString CUiFields::GetServer(CWnd * pUi)
{
	CString sServer;
	pUi->GetDlgItem(IDC_EDIT_SERVER)->GetWindowText(sServer);

	if (sServer.IsEmpty())
	{
		AfxMessageBox(_T("Please enter host"));
		return "";
	}

	return sServer;
}