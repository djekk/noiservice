
#pragma once
#include <afxinet.h>
#include <iostream>

class CWebPageDownloader
{
private:
	CHttpConnection* m_pConnection;
	CHttpFile* m_pFile;
	CInternetSession m_session;

	bool Download_impl(const CString& strUrl, CString& strContent, CString& strError, int nTimeOut = 0);
	static void parseHtmlContext(CString webContext, std::string &status, std::string &dataString);
	static void composePOSTRequest(CString strServerName, CString strObject, const CString& sFormData, std::string &requestString);
	static void composePUTRequest(CString strServerName, CString strObject, const CString& sFormData, std::string &requestString);
	static void composeGETRequest(CString strServerName, CString strObject, std::string &requestString);
	static bool ConnectToWeb(const CString& strUrl, const CString& sFormData, CString& strContent, CString& strError, CString webMethod, int timeout = 1000);
	static int simple_net_connect(const char* server_name, const char* server_port, const char* inputContext, CString& strContent, CString& strError, int timeout);
	static int secure_net_connect(const char* server_name, const char* server_port, const char* inputContext, CString& strContent, CString& strError, int timeout);

public:
	CWebPageDownloader();
	~CWebPageDownloader();

	static bool RestApi(const CString& strMethod, const CString& strUrl, const CString& sFormData, CString& strContent, CString& strError);

};


/////////////////////////////////////////////////////////////////////////////
