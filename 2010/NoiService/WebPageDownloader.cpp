
#include "stdafx.h"
#include "WebPageDownloader.h"
#include "Winsock2.h"
#include <ws2tcpip.h>
#include <iostream>     // std::cout, std::ios
#include <sstream>      // std::ostringstream
#include <locale>
#include <algorithm>
//#include "utf.h"
#include "mbedtls/config.h"
#include "mbedtls/platform.h"

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

static int wsa_init_done = 0;
#define DEBUG_LEVEL 1

static void my_debug(void *ctx, int level,
	const char *file, int line,
	const char *str)
{
	((void)level);

	mbedtls_fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
	fflush((FILE *)ctx);
}

/*
* Prepare for using the sockets interface
*/
static int net_prepare(void)
{
#if ( defined(_WIN32) || defined(_WIN32_WCE) ) && !defined(EFIX64) && \
    !defined(EFI32)
	WSADATA wsaData;

	if (wsa_init_done == 0)
	{
		if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
			return(MBEDTLS_ERR_NET_SOCKET_FAILED);

		wsa_init_done = 1;
	}
#else
#if !defined(EFIX64) && !defined(EFI32)
	signal(SIGPIPE, SIG_IGN);
#endif
#endif
	return(0);
}

CWebPageDownloader::CWebPageDownloader()
	: m_pConnection( 0 )
	, m_pFile( 0 )
	, m_session(_T("Noi"))
{}

CWebPageDownloader::~CWebPageDownloader()
{
	delete m_pFile;
	delete m_pConnection;
	m_session.Close();
}

int CWebPageDownloader::secure_net_connect(const char* server_name, const char* server_port, const char* inputContext,
	CString& strContent, CString& strError, int timeout)
{
	int ret, len;
	mbedtls_net_context server_fd;
	uint32_t flags;
	unsigned char buf[1024];
	const char *pers = "Noi";
	std::string t;

	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert;

#if defined(MBEDTLS_DEBUG_C)
	mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

	/*
	* 0. Initialize the RNG and the session data
	*/
	mbedtls_net_init(&server_fd);
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	mbedtls_x509_crt_init(&cacert);
	mbedtls_ctr_drbg_init(&ctr_drbg);

	/*
	* mbedtls_printf("\n  . Seeding the random number generator...");
	* fflush(stdout);
	*/

	mbedtls_entropy_init(&entropy);
	if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
		(const unsigned char *)pers,
		strlen(pers))) != 0)
	{
		strError.Format(_T(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n"), ret);
		goto exit;
	}

	//mbedtls_printf(" ok\n");

	/*
	* 0. Initialize certificates
	*
	* mbedtls_printf("  . Loading the CA root certificate ...");
	* fflush(stdout);
	*/

	ret = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)mbedtls_test_cas_pem,
		mbedtls_test_cas_pem_len);
	if (ret < 0)
	{
		strError.Format(_T(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n"), -ret);
		goto exit;
	}

	//mbedtls_printf(" ok (%d skipped)\n", ret);

	/*
	* 1. Start the connection
	*
	* mbedtls_printf("  . Connecting to tcp/%s/%s...", server_name, server_port);
	* fflush(stdout);
	*/

	if ((ret = mbedtls_net_connect(&server_fd, server_name,
		server_port, MBEDTLS_NET_PROTO_TCP)) != 0)
	{
		strError.Format(_T(" failed\n  ! mbedtls_net_connect returned %d\n\n"), ret);
		goto exit;
	}

	//mbedtls_printf(" ok\n");

	/*
	* 2. Setup stuff
	*
	* mbedtls_printf("  . Setting up the SSL/TLS structure...");
	* fflush(stdout);
	*/

	if ((ret = mbedtls_ssl_config_defaults(&conf,
		MBEDTLS_SSL_IS_CLIENT,
		MBEDTLS_SSL_TRANSPORT_STREAM,
		MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		strError.Format(_T(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n"), ret);
		goto exit;
	}

	//mbedtls_printf(" ok\n");

	/* OPTIONAL is not optimal for security,
	* but makes interop easier in this simplified example */
	mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
	mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
	mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
	mbedtls_ssl_conf_read_timeout(&conf, timeout);

	if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
	{
		strError.Format(_T(" failed\n  ! mbedtls_ssl_setup returned %d\n\n"), ret);
		goto exit;
	}

	if ((ret = mbedtls_ssl_set_hostname(&ssl, server_name)) != 0)
	{
		strError.Format(_T(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n"), ret);
		goto exit;
	}

	mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

	/*
	* 4. Handshake
	*
	* mbedtls_printf("  . Performing the SSL/TLS handshake...");
	* fflush(stdout);
	*/

	while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			strError.Format(_T(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n"), -ret);
			goto exit;
		}
	}

	//mbedtls_printf(" ok\n");

	/*
	* 5. Verify the server certificate
	*
	* mbedtls_printf("  . Verifying peer X.509 certificate...");
	*/

	/* In real life, we probably want to bail out when ret != 0 */
	if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
	{
		char vrfy_buf[512];

		//mbedtls_printf(" failed\n");

		mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

		//mbedtls_printf("%s\n", vrfy_buf);
	}
	//else
	//	mbedtls_printf(" ok\n");

	/*
	* 3. Write the request
	*
	* mbedtls_printf("  > Write to server:");
	* fflush(stdout);
	*/

	len = sprintf((char *)buf, inputContext);

	while ((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
		{
			strError.Format(_T(" failed\n  ! mbedtls_ssl_write returned %d\n\n"), ret);
			goto exit;
		}
	}
//	_CRT_SECURE_NO_WARNINGS.See online help for details.c : \noiservice\noiservice\webpagedownloader.cpp	251	1	NoiService


	//len = ret;
	//mbedtls_printf(" %d bytes written\n\n%s", len, (char *)buf);

	/*
	* 7. Read the HTTP response
	*
	* mbedtls_printf("  < Read from server:");
	* fflush(stdout);
	*/

	do
	{
		len = sizeof(buf) - 1;
		memset(buf, 0, sizeof(buf));
		ret = mbedtls_ssl_read(&ssl, buf, len);

		if (ret == MBEDTLS_ERR_SSL_WANT_READ ||
			ret == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;

		if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == MBEDTLS_ERR_SSL_TIMEOUT)
		{
			ret = 0;
			break;
		}

		if (ret < 0)
		{
			strError.Format(_T("failed\n  ! mbedtls_ssl_read returned %d\n\n"), ret);
			break;
		}

		if (ret == 0)
		{
			//mbedtls_printf("\n\nEOF\n\n");
			break;
		}

		len = ret;
		buf[len] = '\0';
		//mbedtls_printf(" %d bytes read\n\n%s", len, (char *)buf);
		std::string tt;
		tt.assign((char *)buf, len);
		t.append(tt.c_str());

	} while (1);

	strContent = CString(t.c_str());

	mbedtls_ssl_close_notify(&ssl);

exit:

	mbedtls_net_free(&server_fd);

	mbedtls_x509_crt_free(&cacert);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	return(ret);
}

int CWebPageDownloader::simple_net_connect(const char* server_name, const char* server_port, const char* inputContext, CString& strContent, CString& strError, int timeout)
{
	int ret;
	struct addrinfo hints, *addr_list, *cur;
	SOCKET sock(0);

	if ((ret = net_prepare()) != 0)
		return(ret);

	/* Do name resolution with both IPv6 and IPv4 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(server_name, server_port, &hints, &addr_list) != 0)
	{
		strError.Format(_T("Failed to get an IP address for the given hostname %s: %d"), CString(server_name), MBEDTLS_ERR_NET_UNKNOWN_HOST);
		return(MBEDTLS_ERR_NET_UNKNOWN_HOST);
	}

	/* Try the sockaddrs until a connection succeeds */
	ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
	for (cur = addr_list; cur != NULL; cur = cur->ai_next)
	{
		sock = socket(cur->ai_family, cur->ai_socktype,
			cur->ai_protocol);

		if (sock < 0)
		{
			ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
			strError.Format(_T("Failed to open a socket: %d"), ret);
			continue;
		}

		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(int));
		if (connect(sock, cur->ai_addr, (int)cur->ai_addrlen) == 0)
		{
			ret = 0;
			break;
		}

		closesocket(sock);
		ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
		strError.Format(_T("The connection to the given server %s / port %s failed: %d"), CString(server_name), CString(server_port), ret);
	}

	freeaddrinfo(addr_list);

	if (!ret)
	{
		send(sock, inputContext, strlen(inputContext), 0);

		char buffer[1024];
		int nDataLength;
		std::string t;

		while ((nDataLength = recv(sock, buffer, 1024, 0)) > 0){
			std::string tt;
			tt.assign(buffer, nDataLength);
			t.append(tt.c_str());
		}

		strContent = CString(t.c_str());
		closesocket(sock);
	}

	return(ret);
}

void CWebPageDownloader::parseHtmlContext(CString webContext, std::string &status, std::string &dataString)
{
	CT2CA pszConvertedAnsiretStrObject(webContext);
	std::string t(pszConvertedAnsiretStrObject);
	std::istringstream f(t.c_str());

	int cnt = 0;
	for (std::string line; std::getline(f, line);) {
		std::cout << line << std::endl;
		if (cnt == 0)
		{
			std::transform(line.begin(), line.end(), line.begin(), toupper);
			std::size_t found_http = line.find("HTTP");
			if (found_http != std::string::npos)
				status = line;
		}
		cnt++;
		if (!dataString.empty())
		{
			std::size_t found_1 = line.find("}");
			if (found_1 != std::string::npos)
			{
				line.resize(found_1 + 1);
				dataString.append(line);
				break;
			}
			dataString.append(line);
		}
		std::size_t found = line.find("{");
		if (found != std::string::npos)
		{
			dataString.append(line);
		}
	}

	if (!status.empty())
		status = status.substr(9, 3);
}

void CWebPageDownloader::composePOSTRequest(CString strServerName, CString strObject, const CString& sFormData, std::string &requestString)
{
	CT2CA pszConvertedAnsiServerName(strServerName);
	std::string ansiServerName(pszConvertedAnsiServerName);
	std::ostringstream FormBuffer;

	CString s;
	s.Format(_T("%ld"), sFormData.GetLength());
	// header
	CT2CA pszConvertedAnsiStrObject(strObject);
	std::string ansiStrObject(pszConvertedAnsiStrObject);
	FormBuffer << "POST " << ansiStrObject.c_str() << " HTTP/1.1\n";
	FormBuffer << "Content-Type: application/json\n";
	FormBuffer << "Host: " << ansiServerName.c_str() << "\n";
	FormBuffer << "Content-Length: " << s << "\n\n";

	CT2CA pszConvertedAnsiFormData(sFormData);
	std::string ansiFormData(pszConvertedAnsiFormData);
	FormBuffer << ansiFormData.c_str();

	requestString = FormBuffer.str();
}

void CWebPageDownloader::composePUTRequest(CString strServerName, CString strObject, const CString& sFormData, std::string &requestString)
{
	CT2CA pszConvertedAnsiServerName(strServerName);
	std::string ansiServerName(pszConvertedAnsiServerName);
	std::ostringstream FormBuffer;

	CString s;
	s.Format(_T("%ld"), sFormData.GetLength());
	// header
	CT2CA pszConvertedAnsiStrObject(strObject);
	std::string ansiStrObject(pszConvertedAnsiStrObject);
	FormBuffer << "PUT " << ansiStrObject.c_str() << " HTTP/1.1\n";
	FormBuffer << "Host: " << ansiServerName.c_str() << "\n";
	FormBuffer << "Content-Type: application/json\n";
	FormBuffer << "Content-Length: " << s << "\n\n";

	CT2CA pszConvertedAnsiFormData(sFormData);
	std::string ansiFormData(pszConvertedAnsiFormData);
	FormBuffer << ansiFormData.c_str();

	requestString = FormBuffer.str();
}


void CWebPageDownloader::composeGETRequest(CString strServerName, CString strObject, std::string &requestString)
{
	CT2CA pszConvertedAnsiServerName(strServerName);
	std::string ansiServerName(pszConvertedAnsiServerName);
	std::ostringstream FormBuffer;

	// header
	CT2CA pszConvertedAnsiStrObject(strObject);
	std::string ansiStrObject(pszConvertedAnsiStrObject);
	FormBuffer << "GET " << ansiStrObject.c_str() << " HTTP/1.1\n";
	FormBuffer << "Host: " << ansiServerName.c_str() << "\n\n\n";

	requestString = FormBuffer.str();
}

bool CWebPageDownloader::ConnectToWeb(const CString& strUrl, const CString& sFormData, CString& strContent, CString& strError, CString webMethod, int timeout)
{
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort = 0;
	DWORD dwServiceType = 0;
	
	if (!AfxParseURL(strUrl, dwServiceType, strServerName, strObject, nPort)
		|| dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS)
	{
		strError.Format(_T("Invalid HTTP URL: %s"), strUrl);
		return false;
	}
	char cPort[10];
	sprintf(cPort, "%d", nPort);


	CT2CA pszConvertedAnsiServerName(strServerName);
	std::string ansiServerName(pszConvertedAnsiServerName);

	std::string requestString;
	if (webMethod.Compare(_T("POST")) == 0)
	{
		composePOSTRequest(strServerName, strObject, sFormData, requestString);
	}
	else if (webMethod.Compare(_T("GET")) == 0)
	{
		composeGETRequest(strServerName, strObject, requestString);
	}
	else if (webMethod.Compare(_T("PUT")) == 0)
		composePUTRequest(strServerName, strObject, sFormData, requestString);

	int ret;
	if (dwServiceType == AFX_INET_SERVICE_HTTPS)
		ret = secure_net_connect(ansiServerName.c_str(), cPort, requestString.c_str(), strContent, strError, timeout);
	else
		ret = simple_net_connect(ansiServerName.c_str(), cPort, requestString.c_str(), strContent, strError, timeout);

	if (ret != 0)
		return false;

	return true;
}

bool CWebPageDownloader::RestApi(const CString& strMethod, const CString& strUrl, const CString& strRequest, CString& strResponse, CString& strError)
{
	if (!ConnectToWeb(strUrl, strRequest, strResponse, strError, strMethod))
		return false;
	std::string status;
	std::string dataString;
	parseHtmlContext(strResponse, status, dataString);

	CString cs(dataString.c_str());
	strResponse = cs;

	if (status.compare("200") != 0)
	{
		CString cs_status(status.c_str());
		strError.Format(_T("Invalid response from: %s\nStatus code=%s"), strUrl, cs_status);
		return false;
	}

	return true;
}


bool CWebPageDownloader::Download_impl(const CString& strUrl, CString& strContent, CString& strError, int nTimeOut)
{
	strContent.Empty();
	strError.Empty();

	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort = 0;
	DWORD dwServiceType = 0;
	if( !AfxParseURL(strUrl, dwServiceType, strServerName, strObject, nPort) 
		|| dwServiceType != AFX_INET_SERVICE_HTTP && dwServiceType != AFX_INET_SERVICE_HTTPS )
	{
		strError.Format(_T("Invalid HTTP URL: %s"), strUrl);
		return false;
	}

	m_pConnection = m_session.GetHttpConnection(strServerName, nPort);
	if( m_pConnection == 0 )
	{
		strError.Format(_T("Cannot create connection to: %s:%d"), strServerName, nPort);
		return false;
	}
	
	if (nTimeOut > 0)
	{
		m_session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, nTimeOut);
	}

	m_pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject, NULL, 1, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE);
	if( m_pFile == 0 )
	{
		strError.Format(_T("Cannot create request to: %s"), strUrl);
		return false;
	}
	
	m_pFile->SendRequest();
	
	DWORD dwRet = 0;
	if( !m_pFile->QueryInfoStatusCode(dwRet) || dwRet != HTTP_STATUS_OK )
	{
		strError.Format(_T("Invalid response from: %s\nStatus code=%d"), strUrl, dwRet);
		return false;
	}

	char szBuff[MAX_PATH] = { 0 };
	UINT nRead = m_pFile->Read(szBuff, MAX_PATH-1);
	while(nRead > 0)
	{
		szBuff[nRead] = 0;
		strContent += szBuff;
		nRead = m_pFile->Read(szBuff, MAX_PATH-1);
	}
	return true;
}


