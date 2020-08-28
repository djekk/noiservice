
// NoiServiceDlg.h : header file
//

#pragma once


// CNoiServiceDlg dialog
class CNoiServiceDlg : public CDialogEx
{
// Construction
public:
	CNoiServiceDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_NOISERVICE_DIALOG };
	CMapStringToString m_StringMap;
	void resolveJSON(CString response);
	CString GetServer();

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	void StartTimer(int nID);
	void OnTimer(UINT nIDEvent);

	void GetResponseItem(const CString sName, CString& sValue);
	void GetResponseItems(CString& sId, CString& sError);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedGetInquiry();
	afx_msg void OnBnClickedSetResult();

	void Processing();
	
};
