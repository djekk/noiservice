#pragma once
class CUiFields
{
public:
	CUiFields();
	virtual ~CUiFields();
	
	static void InitUiFiels(CWnd * pUi);

	static bool ValidateUiFields(CWnd * pUi);
	static void SaveProfileProperty(CWnd * pUi);


	static CString GetServer(CWnd * pUi);
};

