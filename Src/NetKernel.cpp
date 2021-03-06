// NetKernel.cpp : Defines the entry point for the DLL application.
//
#include "NetKernel.h"
#include "..\..\utility\debug.h"
#include <string>
#include <iostream>
#include <fstream>
#include <shlobj.h>
#include <vector>
#include <sys/stat.h>
#include <Shlwapi.h>
#include <list>
#include <algorithm>
#include <ctime>

using std::list;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL g_bIsDumpRequest = false;
std::wstring g_wstrDumpFilePath = L"NetKernelDump.log";

const int MAX_PER_READ = 128 * 1024;	// Read 128 KByte (1 MBit) each time in while loop.
const int MAX_HEADER_SIZE = 1024;		// Read 1024 byte at most.
const int MAX_URI_SIZE = 1024;

void SetDumpFile(BOOL bIsDump, const WCHAR* lpwszPath)
{
	if(!lpwszPath)
	{
		bIsDump = false;
		return;
	}

	g_bIsDumpRequest = bIsDump;
	g_wstrDumpFilePath = lpwszPath;
}

// Text body dump
void dump(std::string desc, const char* lpszHeader, const char* lpBody, DWORD dwLength, const char* url, const wchar_t* lpwszDumpFile)
{
	if(!g_bIsDumpRequest)
		return;

	try
	{
		if(wcslen(lpwszDumpFile) == 0)
			lpwszDumpFile = NULL;

		if(lpwszDumpFile)
			dprintf(L"[NetKernel] Dump to: %s", lpwszDumpFile);
		else
			dprintf(L"[NetKernel] No file name to dump.");

		std::ofstream dumpfile;
		dumpfile.open(lpwszDumpFile ? lpwszDumpFile : g_wstrDumpFilePath.c_str(), std::ios_base::out | std::ios_base::app);
		dumpfile << desc << std::endl;
		if(url)
			dumpfile << "URL: " << url << std::endl;
		dumpfile << "Packet:\n" << lpszHeader;
		dumpfile.write(lpBody, dwLength);
		dumpfile << "\n==================================================\n";
		dumpfile.close();
	}
	catch(...)
	{
	}
}


// File list dump
void dump(std::string desc, const char* lpszHeader, std::vector<MultiPartInfo>& infoList, const char* url, const wchar_t* lpwszDumpFile)
{
	if(!g_bIsDumpRequest)
		return;

	try
	{
		if(wcslen(lpwszDumpFile) == 0)
			lpwszDumpFile = NULL;

		std::ofstream dumpfile;
		dumpfile.open(lpwszDumpFile ? lpwszDumpFile : g_wstrDumpFilePath.c_str(), std::ios_base::out | std::ios_base::app);
		dumpfile << desc << std::endl;
		if(url)
			dumpfile << "URL: " << url << std::endl;
		dumpfile << "Packet:\n" << lpszHeader;

		for(std::vector<MultiPartInfo>::iterator iter = infoList.begin(); iter < infoList.end(); iter++)
		{
			dumpfile << (*iter).header.c_str();
			if(!(*iter).filePath.empty())
				dumpfile << "[File Content... File size:" << (*iter).dwFileSize << "]";
		}

		dumpfile << "\n==================================================\n";
		dumpfile.close();
	}
	catch(...)
	{
	}
}

inline std::string genBoundary()
{
	// Boundary in header:	"somestring"
	// Boundary in body:	"--somestring"

	srand(static_cast<int>(time(NULL)));
	std::string boundary = "----------";
	for(int i=0; i<15; ++i)
		boundary += static_cast<char>(rand() % 26) + 'A';
	boundary += "\r\n";
	return boundary;
}


BOOL ResolveUri(const CHAR* lpszUri, std::string& strUrl, BOOL& bSecure, std::string& strHost, DWORD& dwPort)
{
	// Protocol strings.
	const char* pszHttp = "http://";
	const char* pszHttps = "https://";

	// Try to find the protocol HTTP or HTTPS.
	strUrl = lpszUri;
	bSecure = FALSE;
	if(strUrl.find(pszHttp) == 0)
		bSecure = FALSE;
	else if(strUrl.find(pszHttps) == 0)
		bSecure = TRUE;
	else
		return FALSE;

	// Cut out the protocol part.
	strUrl = strUrl.substr(bSecure ? strlen(pszHttps) : strlen(pszHttp));

	// Get the server (host) name.
	strHost;
	size_t iHostEnd = strUrl.find("/");
	if(iHostEnd == std::string::npos)
	{
		strHost = strUrl;
		strUrl = "";
	}
	else
	{
		strHost = strUrl.substr(0, iHostEnd);
		strUrl = strUrl.substr(iHostEnd);
	}

	// Host may contain port name. Or use the default value (80 and 443).
	dwPort = INTERNET_DEFAULT_HTTP_PORT;
	size_t iPort = strHost.find(":");
	if(iPort != std::string::npos)
	{
		dwPort = atoi(strHost.substr(iPort+1).c_str());
		strHost = strHost.substr(0, iPort);
	}
	else
	{
		dwPort = bSecure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
	}

	return TRUE;
}


CNetKernel::CNetKernel() :
m_hInternet(NULL),
m_hConnect(NULL),
m_hRequest(NULL),
m_bForceClose(FALSE),
m_bCacheDownload(FALSE),
m_hWnd(NULL),
m_cSimpleEvent(TRUE)
{
	memset(m_lpcwszCookieFileName,0x0,MAX_PATH+1);
}


CNetKernel::~CNetKernel()
{
	// Check the handles for safe.
	CloseInternetHandle(m_hRequest, m_hConnect,  m_hInternet);
}


//void PyNetKernel::SetCallback(PyObject* callback)
//{
//}


// Send text body request.
DWORD CNetKernel::SendHttpRequest(HttpRespValObj& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer, DWORD wPort,
	BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpszBody, const WCHAR* lpwszResponse, const WCHAR* lpwszDump)
{
	return SendHttpContent(httpResp, lpszApName, lpszMethod, lpszServer, wPort, bSecure, lpszUrl, lpszHeader,
		(CHAR*)lpszBody, lpszBody ? static_cast<DWORD>(strlen(lpszBody)) : 0, lpwszResponse, lpwszDump);
}


// The function to do all send data jobs.
DWORD CNetKernel::SendHttpContent(HttpRespValObj& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer,
	DWORD dwPort, BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader,
	const CHAR* lpBody, DWORD dwLength, const WCHAR* lpwszResponse, const WCHAR* lpwszDump)
{
	BOOL bSuccess = FALSE;
	std::string strServerResponse;
	DWORD dwStatusCode = 0;
	DWORD dwError = 0;

	// Change the flag before unlock.
	m_bForceClose = FALSE;

	m_cSimpleEvent.Set();
	do{
		if(!PrepareConnection(m_hInternet, m_hConnect, m_hRequest, lpszApName, lpszMethod, lpszServer, dwPort, bSecure, lpszUrl))
			continue;

		if(g_bIsDumpRequest && (!g_wstrDumpFilePath.empty() || wcslen(lpwszDump) > 0))
			dump("NetKernel Request:", lpszHeader, lpBody, dwLength, lpszUrl, lpwszDump);

		BOOL bRequestSent = FALSE;
		BOOL bUseProxy = FALSE;
		do 
		{
			bRequestSent = SendHttpReq(m_hRequest, bUseProxy, lpszHeader, (LPVOID)(lpBody)?lpBody:"", dwLength, dwStatusCode);
			if(bRequestSent && !bUseProxy && dwStatusCode == HTTP_STATUS_PROXY_AUTH_REQ)
			{
				bUseProxy = TRUE;
				continue;
			}
			break;
		} while(1);

		if(!bRequestSent)
			continue;

		BOOL bReceive = ReceiveResponse(m_hRequest, dwStatusCode, lpwszResponse, strServerResponse, dwError, lpwszDump);
		if(!bReceive)
			continue;

		bSuccess = TRUE;
	} while(0);

	if(dwError == 0)
		dwError = ::GetLastError();

	// Close all handles.
	CloseInternetHandle(m_hRequest, m_hConnect,  m_hInternet);

	m_cSimpleEvent.Reset();
	httpResp.dwError = (m_bForceClose)? (int) ERROR_FORCECCLOSE : dwError;
	httpResp.dwStatusCode = dwStatusCode;
	httpResp.szResp = strServerResponse;
	return dwError;
}


BOOL CNetKernel::SendHttpReq(HINTERNET& hRequest, BOOL bUseProxy, const CHAR* lpszHeader, LPVOID lpOptional, DWORD dwLength, DWORD& dwStatusCode)
{
	dprintf("[NetKernel] Send HTTP request: %s", lpszHeader);
	if(bUseProxy)
	{
		DWORD dwLastError = GetLastError();
		dprintf(L"[NetKernel] Last error before internet error dialog: %X", dwLastError);
		DWORD ret = InternetErrorDlg(m_hWnd,
			hRequest,
			dwLastError,
			FLAGS_ERROR_UI_FILTER_FOR_ERRORS | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS | FLAGS_ERROR_UI_FLAGS_GENERATE_DATA, NULL);
		dprintf(L"[NetKernel] InternetErrorDlg result: %X", ret);

		if(ret != ERROR_SUCCESS && ret != ERROR_INTERNET_FORCE_RETRY)
			return FALSE;
	}

	BOOL bRequestSent = FALSE;
	bRequestSent = ::HttpSendRequestA(m_hRequest, lpszHeader, lpszHeader ? (DWORD)strlen(lpszHeader) : 0, lpOptional, dwLength);
	if(!bRequestSent)
		return FALSE;

	DWORD dwBufferLength = sizeof(dwStatusCode);
	BOOL bStatusRead = ::HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwBufferLength, 0);
	if(!bStatusRead)
		return FALSE;

	return TRUE;
}


BOOL CNetKernel::PrepareConnection(HINTERNET& hInternet, HINTERNET& hConnect, HINTERNET& hRequest, const CHAR* lpszApName,
	const CHAR* lpszMethod, const CHAR* lpszServer, DWORD dwPort, BOOL bSecure, const CHAR* lpszUrl)
{
	if(OnStateCallBack("connect", 0, 0) != CALLBACK_OK)
		return FALSE;

	hInternet = ::InternetOpenA(lpszApName, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(!hInternet)
		return FALSE;

	//DWORD dwFlag= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
	//::InternetSetOption(hInternet, INTERNET_OPTION_SECURITY_FLAGS, &dwFlag, sizeof(dwFlag));

	hConnect = ::InternetConnectA(hInternet, lpszServer, static_cast<INTERNET_PORT>(dwPort), NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
	if(!hConnect)
		return FALSE;

	DWORD dwFlags = (bSecure) ? (INTERNET_FLAG_SECURE | INTERNET_FLAG_RESYNCHRONIZE) : INTERNET_FLAG_RESYNCHRONIZE;
	hRequest = ::HttpOpenRequestA(hConnect, lpszMethod, lpszUrl, NULL, NULL, NULL, dwFlags, 1);
	if(!hRequest)
		return FALSE;

	return TRUE;
}


BOOL CNetKernel::QueryResponseInfo(HINTERNET& hRequest, DWORD& dwStatusCode, DWORD& dwResLength, std::string& contentType)
{
	DWORD dwBufferLength = sizeof(dwStatusCode);
	BOOL bStatusRead = ::HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwBufferLength, 0);
	if(!bStatusRead)
		return FALSE;

	dwBufferLength = sizeof(dwResLength);
	bStatusRead = ::HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwResLength, &dwBufferLength, 0);
	if(!bStatusRead)
		dwBufferLength = 0;

	char lpszContentType[_MAX_PATH] = {0};
	dwBufferLength = _MAX_PATH - 1;
	bStatusRead = ::HttpQueryInfoA(hRequest, HTTP_QUERY_CONTENT_TYPE, lpszContentType, &dwBufferLength, 0);
	if(dwBufferLength >= _MAX_PATH)
	{
		dprintf(L"[NetKernel] Warning, reading content type out of buffer size.");
		dwBufferLength = _MAX_PATH - 1;
	}
	lpszContentType[dwBufferLength] = 0;
	if(bStatusRead)
		contentType = lpszContentType;
	dprintf("[NetKernel] Content-Type: %s", lpszContentType);

	return TRUE;
}


BOOL CNetKernel::ReceiveTextResponse(HINTERNET& hRequest, DWORD dwContentLength, std::string& strServerResponse)
{
	char szBuffer[_MAX_PATH];
	DWORD dwRead = 0;
	DWORD dwTotalRead = 0;
	do{
		if(OnStateCallBack("receive", (int)dwTotalRead, (int)dwContentLength) != CALLBACK_OK)
			return FALSE;

		if(::InternetReadFile(hRequest, szBuffer, _MAX_PATH, &dwRead))
		{
			strServerResponse.append(szBuffer, dwRead);
			dwTotalRead += dwRead;
		}
	} while(dwRead != 0);

	return TRUE;
}


BOOL CNetKernel::ReceiveResponseToFile(HINTERNET& hRequest, DWORD dwContentLength, const WCHAR* lpwszResponse)
{
	std::ofstream outResponse;
	try
	{
		outResponse.open(lpwszResponse, std::ios::out | std::ios::binary);
	}
	catch(...)
	{
		return FALSE;
	}

	if (outResponse.fail()) return FALSE;

	char *pBuffer = new char[MAX_PER_READ];
	DWORD dwRead = 0;
	DWORD dwTotalRead = 0;
	do{
		if(OnStateCallBack("receive", (int)dwTotalRead, (int)dwContentLength) != CALLBACK_OK)
		{
			outResponse.close();
			delete [] pBuffer;
			return FALSE;
		}

		if(::InternetReadFile(hRequest, pBuffer, MAX_PER_READ, &dwRead))
		{
			outResponse.write(pBuffer, dwRead);
			dwTotalRead += dwRead;
		}

		Sleep(1);	// If download a large file, this prevent CPU goes 100%.
	} while(dwRead != 0);

	outResponse.close();

	delete [] pBuffer;

	return TRUE;
}


// This function will allocate the buffer use new operator.
// The response data maybe raw data or string.
BOOL CNetKernel::ReceiveResponseToBuffer(HINTERNET& hRequest, DWORD& dwContentLength, char* &pResBuffer)
{
	DWORD dwRead = 0;
	DWORD dwTotalRead = 0;

	DWORD dwBufSize = 1024;
	if(dwContentLength != 0)
	{
		// Note: +1 because if the caller really know the data length,
		// add more 1 byte to escape the size check in while loop.
		// So that this function will not allocate too much unnecessary memory.
		dwBufSize = dwContentLength + 1;
		pResBuffer = new char[dwBufSize];
	}
	else
	{
		pResBuffer = new char[dwBufSize];
	}
	memset(pResBuffer,0x0,dwBufSize);

	do{
		if(OnStateCallBack("receive", (int)dwTotalRead, (int)dwContentLength) != CALLBACK_OK)
			return FALSE;

		// Check if the buffer is full.
		if(dwTotalRead == dwBufSize)
		{
			dwBufSize *= 2;
			char* pNewBuffer = new char[dwBufSize];
			memset(pNewBuffer,0x0,dwBufSize);
			memcpy_s(pNewBuffer, dwBufSize, pResBuffer, dwBufSize / 2);
			delete [] pResBuffer;
			pResBuffer = pNewBuffer;
		}

		if(pResBuffer && ::InternetReadFile(hRequest, &pResBuffer[dwTotalRead], dwBufSize - dwTotalRead, &dwRead))
		{
			dwTotalRead += dwRead;
		}
	} while(dwRead != 0);

	dwContentLength = dwTotalRead;
	
	pResBuffer[dwContentLength] = '\0';
	dwContentLength += 1;

	return TRUE;
}


BOOL CNetKernel::ReceiveResponse(HINTERNET& hRequest, DWORD& dwStatusCode, const WCHAR* lpwszResponse,
								  std::string& strServerResponse, DWORD& dwError, const WCHAR* lpwszDump)
{
	DWORD dwContentLength = 0;	// "Content-Length" field in response header.
	std::string contentType;	// "Content-Type" field in response header.

	// Query response info.
	BOOL bQueryInfo = QueryResponseInfo(hRequest, dwStatusCode, dwContentLength, contentType);
	if(!bQueryInfo)
		return FALSE;

	// Get the response.
	if(lpwszResponse && wcslen(lpwszResponse) > 0)
	{
		dprintf(L"[NetKernel] ReceiveResponseToFile");

		if(!ReceiveResponseToFile(hRequest, dwContentLength, lpwszResponse))
			return FALSE;

		if(g_bIsDumpRequest && (!g_wstrDumpFilePath.empty() || wcslen(lpwszDump) > 0))
		{
			std::string desc = "[Binary response]";
			dump("NetKernel Response:", "", desc.c_str(), static_cast<DWORD>(desc.length()), NULL, lpwszDump);
		}
	}
	else
	{
		dprintf(L"[NetKernel] ReceiveTextResponse");

		if(!ReceiveTextResponse(hRequest, dwContentLength, strServerResponse))
			return FALSE;

		if(g_bIsDumpRequest && (!g_wstrDumpFilePath.empty() || wcslen(lpwszDump) > 0))
			dump("NetKernel Response:", "", strServerResponse.c_str(), static_cast<DWORD>(strServerResponse.length()), NULL, lpwszDump);
	}

	return TRUE;
}


void CNetKernel::CloseInternetHandle(HINTERNET& hRequest, HINTERNET& hConnect, HINTERNET& hInternet)
{
	if(hRequest)
	{
		::InternetCloseHandle(hRequest);
		hRequest = NULL;
	}
	if(hConnect)
	{
		::InternetCloseHandle(hConnect);
		hConnect = NULL;
	}
	if(hInternet)
	{
		::InternetCloseHandle(hInternet);
		hInternet = NULL;
	}
}


BOOL CNetKernel::WriteInternetContent(HINTERNET& hRequest, const char* lpContent, DWORD dwLength, DWORD& dwSent, DWORD dwContentLength)
{
	DWORD written = 0;
	DWORD totalWrite = 0;
	
	if(!lpContent)
		return FALSE;

	while(totalWrite != dwLength)
	{
		BOOL result = ::InternetWriteFile(hRequest, (LPCVOID)(&lpContent[totalWrite]), dwLength - totalWrite, &written);
		if(!result)
			return FALSE;

		totalWrite += written;
		dwSent += written;

		if(OnStateCallBack("transmit", (int)dwSent, (int)dwContentLength) != CALLBACK_OK)
			return FALSE;
	}

	return TRUE;
}


BOOL CNetKernel::WriteMultipartBody(HINTERNET& hRequest, std::vector<MultiPartInfo>& infoList, DWORD dwContentLength)
{
	dprintf(L"[NetKernel] Now write:");

	DWORD dwSent = 0;

	for(std::vector<MultiPartInfo>::iterator iter = infoList.begin(); iter<infoList.end(); iter++)
	{
		if(OnStateCallBack("transmit", (int)dwSent, (int)dwContentLength) != CALLBACK_OK)
			return FALSE;

		// Debug
		dprintf("%s\n%s", (*iter).header.c_str(), "==================================================");

		// Write the header.
		if(!WriteInternetContent(hRequest, (*iter).header.c_str(), static_cast<DWORD>((*iter).header.length()), dwSent, dwContentLength))
			return FALSE;

		dprintf(L"[NetKernel] WriteInternetContent header success.");

		if(!(*iter).filePath.empty())
		{
			char* lpRawBuffer = new char[MAX_PER_READ];

			DWORD dwLeftSize = (*iter).dwFileSize;
			std::ifstream input;
			input.open((*iter).filePath.c_str(), std::ios::binary);
			while(dwLeftSize > 0)
			{
				DWORD dwCurrentRead = dwLeftSize > MAX_PER_READ ? MAX_PER_READ : dwLeftSize;
				input.read(lpRawBuffer, dwCurrentRead);

				if(!WriteInternetContent(hRequest, lpRawBuffer, dwCurrentRead, dwSent, dwContentLength))
				{
					input.close();
					delete [] lpRawBuffer;
					return FALSE;
				}

				dwLeftSize -= dwCurrentRead;
			}

			input.close();
			dprintf(L"[NetKernel] WriteInternetContent file content success.");

			delete [] lpRawBuffer;
		}

		if(!(*iter).content.empty())
		{
			DWORD dwTotal = static_cast<DWORD>((*iter).content.length());
			DWORD dwCurrentSent = 0;

			while(dwCurrentSent < dwTotal)
			{
				DWORD dwCurrentWrite = (dwTotal - dwCurrentSent) > MAX_PER_READ ? MAX_PER_READ : (dwTotal - dwCurrentSent);
				if(!WriteInternetContent(hRequest, &((*iter).content.c_str()[dwCurrentSent]), dwCurrentWrite, dwSent, dwContentLength))
					return FALSE;

				dwCurrentSent += dwCurrentWrite;
			}
		}
	}

	dprintf(L"[NetKernel] WriteMultipartBody success return");

	return TRUE;
}

// Send a file with (or not) plain text body, and files in the list.
//
// pPyMultiPart should give format like this (in python level):
// [{'header':'multi-part header', 'filename':u'c:\\somefile.raw', 'filesize':100, 'content':'content', 'contentlength':100}, {...}]
//
// header		: (UTF-8) The multi-part header in body. Including all boundary and \r\n.
// filename		: (Unicode) A file that want to send.
// filesize		: (Integer) Size of file.
// content		: (UTF-8) The content want to send in one of multi-part.
//
// The parameters above must give, even the data is empty, caller should give like: 'filename':u''
//
DWORD CNetKernel::SendHttpRequestMultipart(HttpRespValObj& httpResp, const CHAR* lpszApName, const CHAR* lpszUri, const CHAR* lpszMethod,
	const WCHAR* lpwszProxy, const CHAR* lpszHeader, std::vector<MultiPartInfo> vecMultiPart, DWORD dwContentLength,
	const WCHAR* lpwszResponse, const WCHAR* lpwszDump)
{
	std::string strUrl;
	BOOL bSecure = FALSE;
	std::string strHost;
	DWORD dwPort = 0;
	ResolveUri(lpszUri, strUrl, bSecure, strHost, dwPort);

	dprintf(L"[NetKernel] SendHttpRequestWithFile");

	std::string strServerResponse;
	DWORD dwStatusCode = 0;
	DWORD dwError = 0;
	std::vector<MultiPartInfo> infoList;

	dprintf(L"[NetKernel] Read the multipart list.");

	// Change the flag before unlock.
	m_bForceClose = FALSE;
	BOOL bSuccess = FALSE;

	m_cSimpleEvent.Set();
	do{
		if(!PrepareConnection(m_hInternet, m_hConnect, m_hRequest, lpszApName, lpszMethod, strHost.c_str(), dwPort, bSecure, strUrl.c_str()))
			continue;

		dprintf(L"[NetKernel] PrepareConnection success");
		std::string header(lpszHeader);

		if(g_bIsDumpRequest && (!g_wstrDumpFilePath.empty() || wcslen(lpwszDump) > 0))
			dump("NetKernel Request:", header.c_str(), vecMultiPart, strUrl.c_str(), lpwszDump);

		//Prepare the Buffers to be passed to HttpSendRequestEx
		INTERNET_BUFFERSA BuffersIn;
		ZeroMemory(&BuffersIn, sizeof(INTERNET_BUFFERSA));
		BuffersIn.dwStructSize = sizeof(INTERNET_BUFFERSA);
		BuffersIn.lpvBuffer = NULL;
		BuffersIn.dwBufferLength = 0;
		BuffersIn.lpcszHeader = header.c_str();
		BuffersIn.dwHeadersLength = static_cast<DWORD>(header.length());
		BuffersIn.dwBufferTotal = dwContentLength; // content-length of data

		BOOL bRequestSent = ::HttpSendRequestExA(m_hRequest, &BuffersIn, NULL, 0, NULL);
		if(!bRequestSent)
			continue;
		dprintf(L"[NetKernel] HttpSendRequestExA success");

		if(!WriteMultipartBody(m_hRequest, vecMultiPart, dwContentLength))
			continue;
		dprintf(L"[NetKernel] WriteMultipartBody success");

		// Stop the write file.
		BOOL bRequestEnd = HttpEndRequest(m_hRequest, NULL, 0, 0);
		if(!bRequestEnd)
			continue;
		dprintf(L"[NetKernel] HttpEndRequest success");

		BOOL bReceive = ReceiveResponse(m_hRequest, dwStatusCode, lpwszResponse, strServerResponse, dwError, lpwszDump);
		if(!bReceive)
			continue;

		bSuccess = TRUE;
		dprintf(L"[NetKernel] Send request success end.");

	} while(0);

	if(dwError == 0)
		dwError = ::GetLastError();

	// Close all handles.
	CloseInternetHandle(m_hRequest, m_hConnect,  m_hInternet);

	m_cSimpleEvent.Reset();

	if(m_bForceClose)
	{
		httpResp.dwError = ERROR_FORCECCLOSE;
		httpResp.dwStatusCode = (int) dwStatusCode;
		return ERROR_FORCECCLOSE;
		//return Py_BuildValue("iis", ERROR_FORCECCLOSE, (int)dwStatusCode, "");
	}

	if(bSuccess)
	{
		httpResp.dwError = 0;
		httpResp.dwStatusCode = dwStatusCode;
		//SetRespViaStdStr(httpResp.szResp,strServerResponse);
		httpResp.szResp = strServerResponse;
		return 0;
		//return Py_BuildValue("iis", (int)0, (int)dwStatusCode, (LPCSTR)strServerResponse.c_str());
	}
	else if(dwError == (DWORD) ERROR_PROCESS)
	{
		httpResp.dwError = ERROR_PROCESS;
		httpResp.dwStatusCode = 0;
		return ERROR_PROCESS;
		//return Py_BuildValue("iis", ERROR_PROCESS, 0, "");
	}
	else
	{
		httpResp.dwError = dwError;
		httpResp.dwStatusCode = dwStatusCode;
		return dwError;
		//return Py_BuildValue("iis", (int)dwError, (int)dwStatusCode, "");
	}
	//if(bSuccess)
	//	return Py_BuildValue("iis", (int)0, (int)dwStatusCode, (LPCSTR)strServerResponse.c_str());
	//else if(dwError == (DWORD) ERROR_PROCESS)
	//	return Py_BuildValue("iis", ERROR_PROCESS, 0, "");
	//else
	//	return Py_BuildValue("iis", (int)dwError, (int)dwStatusCode, "");
}


BOOL CNetKernel::SendUrlRequestImpl(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
									  const CHAR* pBodyBuffer, DWORD dwSendLength, DWORD& dwStatusCode, DWORD& dwAvailableData, DWORD& dwError)
{
	std::string strUrl;
	BOOL bSecure = FALSE;
	std::string strHost;
	DWORD dwPort = 0;
	ResolveUri(lpszUri, strUrl, bSecure, strHost, dwPort);

	// Check method.
	std::string strMethod;
	if(lpszMethod && strlen(lpszMethod) > 0)
		strMethod = lpszMethod;
	else if(pBodyBuffer && dwSendLength > 0)
		strMethod = "POST";
	else
		strMethod = "GET";

	// Construct the header.
	std::string strHeader;

	if(lpszHeader && strlen(lpszHeader) > 0)
		strHeader = lpszHeader;
	else
	{
		strHeader += "Host: ";
		strHeader += strHost + "\r\n";
	}

	BOOL bSuccess = FALSE;
	const CHAR* lpszApName = "CL-NetKernel";

	// Change the flag before unlock.
	m_bForceClose = FALSE;

	do{
		if(!PrepareConnection(m_hInternet, m_hConnect, m_hRequest, lpszApName, strMethod.c_str(), strHost.c_str(), dwPort, bSecure, strUrl.c_str()))
			continue;

		// Setup proxy.
		INTERNET_PROXY_INFO proxy;
		if(lpwszProxy && wcslen(lpwszProxy) > 0)
		{	
			proxy.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
			proxy.lpszProxy = lpwszProxy;
			proxy.lpszProxyBypass = NULL;
			BOOL bProxy = InternetSetOption(m_hInternet, INTERNET_OPTION_PROXY, (LPVOID)&proxy, sizeof(INTERNET_PROXY_INFO));
			if(!bProxy)
				continue;
		}

		if(pBodyBuffer && dwSendLength > 0)
		{
			//Prepare the Buffers to be passed to HttpSendRequestEx
			INTERNET_BUFFERSA BuffersIn;
			ZeroMemory(&BuffersIn, sizeof(INTERNET_BUFFERSA));
			BuffersIn.dwStructSize = sizeof(INTERNET_BUFFERSA);
			BuffersIn.lpvBuffer = NULL;
			BuffersIn.dwBufferLength = 0;
			BuffersIn.lpcszHeader = strHeader.c_str();
			BuffersIn.dwHeadersLength = static_cast<DWORD>(strHeader.length());
			BuffersIn.dwBufferTotal = dwSendLength; // content-length of data

			BOOL bRequestSent = ::HttpSendRequestExA(m_hRequest, &BuffersIn, NULL, 0, NULL);
			if(!bRequestSent)
				continue;

			DWORD dwSent = 0;
			//BOOL bWriteContent = WriteInternetContent(m_hRequest, pBodyBuffer, dwSendLength, dwSent, dwSendLength);
			BOOL bWriteContent = FALSE;
			DWORD dwCurrentSent = 0;
			while(dwCurrentSent < dwSendLength)
			{
				DWORD dwCurrentWrite = (dwSendLength - dwCurrentSent) > MAX_PER_READ ? MAX_PER_READ : (dwSendLength - dwCurrentSent);
				bWriteContent = WriteInternetContent(m_hRequest, &(pBodyBuffer[dwCurrentSent]), dwCurrentWrite, dwSent, dwSendLength);
				if(!bWriteContent) break;
				dwCurrentSent += dwCurrentWrite;
			}

			if(!bWriteContent)
				continue;

			// Stop the write file.
			BOOL bRequestEnd = HttpEndRequest(m_hRequest, NULL, 0, 0);
			if(!bRequestEnd)
				continue;
		}
		else
		{
			std::string strServerResponse;
			BOOL bRequestSent = FALSE;
			BOOL bUseProxy = FALSE;
			do
			{
				bRequestSent = SendHttpReq(m_hRequest, bUseProxy, strHeader.c_str(), (LPVOID)"", 0, dwStatusCode);
				if(bRequestSent && !bUseProxy && dwStatusCode == HTTP_STATUS_PROXY_AUTH_REQ)
				{
					bUseProxy = TRUE;
					continue;
				}
				break;
			} while(1);
		}

		// Query response info.
		std::string contentType;	// "Content-Type" field in response header.
		BOOL bQueryInfo = QueryResponseInfo(m_hRequest, dwStatusCode, dwAvailableData, contentType);
		if(!bQueryInfo)
			continue;

		bSuccess = TRUE;
	}while(0);

	if(!bSuccess)
	{
		dwError = ::GetLastError();
		CloseInternetHandle(m_hRequest, m_hConnect,  m_hInternet);
	}

	return bSuccess;
}


BOOL CNetKernel::ReceiveUrlDataImpl(DWORD& dwContentLength, const WCHAR* lpwszResponse, char*& pResBuffer, DWORD& dwError)
{
	BOOL bSuccess = FALSE;

	do{

		BOOL bReceive;
		if(lpwszResponse && wcslen(lpwszResponse) > 0)
		{
			// If caller give a file, store the response to this file.
			bReceive = ReceiveResponseToFile(m_hRequest, dwContentLength, lpwszResponse);

			// The response buffer is empty.
			dwContentLength = 0;
		}
		else
		{
			// Receive the data and the pResBuffer will point to response buffer.
			// The dwAvailableData will be change to the available data size in byte.
			bReceive = ReceiveResponseToBuffer(m_hRequest, dwContentLength, pResBuffer);
		}
		if(!bReceive)
			continue;

		bSuccess = TRUE;
	} while(0);

	if(!bSuccess)
		dwError = ::GetLastError();

	// Close all handles.
	CloseInternetHandle(m_hRequest, m_hConnect,  m_hInternet);

	return bSuccess;
}

//PyObject* PyNetKernel::SendUrlRequest(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
//						 const CHAR* pBodyBuffer, DWORD dwBodyLength)
//{
//	DWORD dwStatusCode = 0;
//	DWORD dwAvailableData = 0;
//
//	BOOL bSuccess = FALSE;
//	DWORD dwError = 0;
//
//	// Change the flag before unlock.
//	m_bForceClose = FALSE;
//
//	char lpszHeaderBuffer[MAX_HEADER_SIZE];
//	DWORD dwHeaderSize = sizeof(lpszHeaderBuffer);
//
//	m_cSimpleEvent.Set();
//	do{
//		if(!SendUrlRequestImpl(lpszUri, lpszMethod, lpwszProxy, lpszHeader, pBodyBuffer, dwBodyLength, dwStatusCode, dwAvailableData, dwError))
//			continue;
//
//		if(!HttpQueryInfoA(m_hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, lpszHeaderBuffer, &dwHeaderSize, 0))
//			continue;
//
//		bSuccess = TRUE;
//	}while(0);
//	m_cSimpleEvent.Reset();
//
//	PyObject* pRet = NULL;
//	if(m_bForceClose)
//	{
//		pRet = Py_BuildValue("iis", (int)dwError, (int)dwStatusCode, "");
//	}
//	else if(bSuccess)
//	{
//		pRet = Py_BuildValue("iis#", (int)dwError, (int)dwStatusCode, lpszHeaderBuffer, dwHeaderSize);
//	}
//	else
//		pRet = Py_BuildValue("iis#", (int)dwError, (int)dwStatusCode, "", 0);
//
//	return pRet;
//}


//PyObject* PyNetKernel::ReceiveUrlData(DWORD dwContentLength, const WCHAR* lpwszResponse)
//{
//	char* pResBuffer = NULL;
//	BOOL bSuccess = FALSE;
//	DWORD dwError = 0;
//
//	// Change the flag before unlock.
//	m_bForceClose = FALSE;
//
//	m_cSimpleEvent.Set();
//	do{
//		if(!ReceiveUrlDataImpl(dwContentLength, lpwszResponse, pResBuffer, dwError))
//			continue;
//
//		bSuccess = TRUE;
//	}while(0);
//	m_cSimpleEvent.Reset();
//
//	PyObject* pRet = NULL;
//	if(m_bForceClose)
//	{
//		pRet = Py_BuildValue("is", (int)dwError, "");
//
//		delete [] pResBuffer;
//		pResBuffer = NULL;
//	}
//	else if(bSuccess)
//	{
//		pRet = Py_BuildValue("is#", (int)dwError, pResBuffer, dwContentLength);
//		delete [] pResBuffer;
//		pResBuffer = NULL;
//	}
//	else
//		pRet = Py_BuildValue("is#", (int)dwError, "", 0);
//
//	return pRet;
//}

DWORD CNetKernel::OpenUrl(HttpRespValObj& httpResp, const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,const WCHAR* lpwszResponse, const CHAR* pBodyBuffer, DWORD dwBodyLength)
{
	DWORD dwStatusCode = 0;
	DWORD dwAvailableData = 0;

	BOOL bSuccess = FALSE;
	DWORD dwError = 0;
	char* pResBuffer = NULL;

	// Change the flag before unlock.
	m_bForceClose = FALSE;

	if (pBodyBuffer && dwBodyLength == 0)
		dwBodyLength = strlen(pBodyBuffer);
	m_cSimpleEvent.Set();
	if(m_bCacheDownload)
	{
		// CacheCallbacker is a fake COM object and Release function would delete itself.
		// Ignore error 429 here.
		//lint -e429
		do 
		{
			WCHAR pwszFilename[_MAX_PATH] = {0};
			CacheCallbacker* callbacker = new CacheCallbacker(this);
			callbacker->AddRef();
			WCHAR pwszUri[MAX_URI_SIZE] = {0};
			if (lpszUri)
				MultiByteToWideChar(CP_ACP, MB_COMPOSITE, lpszUri, strlen(lpszUri), pwszUri, MAX_URI_SIZE);
			HRESULT hr = URLDownloadToCacheFile(NULL, pwszUri, pwszFilename, _MAX_PATH, 0, callbacker);
			if(FAILED(hr)) break;
			wcscpy_s(m_lpcwszCookieFileName,wcslen(pwszFilename)+1,pwszFilename);
			if(lpwszResponse && wcslen(lpwszResponse) > 0)
				bSuccess = CopyFile(pwszFilename, lpwszResponse, FALSE);
			else
			{
				HANDLE hFile = CreateFile(pwszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if(hFile == INVALID_HANDLE_VALUE) break;
				dwAvailableData = GetFileSize(hFile, NULL);
				pResBuffer = new char[dwAvailableData+1];
				//memset(pResBuffer,0x0,dwAvailableData+1);
				DWORD dwFileRead = 0, dwTotalRead = 0;
				while(dwTotalRead < dwAvailableData)
				{
					char* pReadBuffer = NULL;
					ReadFile(hFile, &pResBuffer[dwTotalRead], dwAvailableData - dwTotalRead, &dwFileRead, NULL);
					dwTotalRead += dwFileRead;
				}
				bSuccess = TRUE;
			}
			callbacker->Release();
		} while(0);
		//lint +e429
	}
	else
	{
		do{
			if(!SendUrlRequestImpl(lpszUri, lpszMethod, lpwszProxy, lpszHeader, pBodyBuffer, dwBodyLength, dwStatusCode, dwAvailableData, dwError))
				continue;

			if(!ReceiveUrlDataImpl(dwAvailableData, lpwszResponse, pResBuffer, dwError))
				continue;

			bSuccess = TRUE;
		} while(0);
	}

	m_cSimpleEvent.Reset();
	httpResp.dwError = dwError;
	httpResp.dwStatusCode = dwStatusCode;
	if (pResBuffer)
	{
		httpResp.szResp = std::string(pResBuffer);
		delete[] pResBuffer;
		pResBuffer = NULL;
	}
	return dwError;
}

BOOL CNetKernel::DeleteUrlCache(int type, const WCHAR* lpwszCookieName)
{
	BOOL bRet = FALSE;
	HANDLE hEntry;
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;  
	DWORD dwEntrySize;

	//delete the files
	dwEntrySize = 0;
	hEntry = FindFirstUrlCacheEntry(NULL, NULL, &dwEntrySize);
	lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
	hEntry = FindFirstUrlCacheEntry(NULL, lpCacheEntry, &dwEntrySize);
	if (!hEntry && lpCacheEntry)
	{
		delete [] lpCacheEntry; 
		//return Py_BuildValue("i", bRet);
		return bRet;
	}

	do
	{
		if (type == File && !(lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
		{
			DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
		}
		else if (type == Cookie && (lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY))
		{
			if (lpwszCookieName && wcsstr(lpCacheEntry->lpszSourceUrlName, lpwszCookieName))
			{
				DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
			}
		}

		dwEntrySize = 0;
		FindNextUrlCacheEntry(hEntry, NULL, &dwEntrySize);
		delete [] lpCacheEntry; 
		lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
	}
	while (FindNextUrlCacheEntry(hEntry, lpCacheEntry, &dwEntrySize));

	bRet = TRUE;

	if (lpCacheEntry)
	{
		delete [] lpCacheEntry; 
	}
	/*return Py_BuildValue("i", bRet);*/
	return bRet;
}

void CNetKernel::ForceStop()
{
	m_bForceClose = TRUE;
	CloseInternetHandle(m_hRequest, m_hConnect,  m_hInternet);
}

void CNetKernel::SetWindowHandle(HWND hWnd)
{
	m_hWnd = hWnd;
}

long CNetKernel::OnStateCallBack(const char* lpszState, int nCurrent, int nTotal)
{
	return 1;
}

void CNetKernel::SetDownloadCache(BOOL bCacheDownload)
{
	m_bCacheDownload = bCacheDownload;
}

//PyObject* PyNetKernel::GetDZRegParams()
//{
//	HRESULT hr = E_FAIL;
//	WCHAR szCurrentDir[_MAX_PATH] = L"";
//	WCHAR szCurrentOLReg[_MAX_PATH] = L"";
//	WCHAR szCurrentOLRState[_MAX_PATH] = L"";
//	WCHAR szParam[_MAX_PATH] = L"";
//	WCHAR szIsRegister[_MAX_PATH] = L"";
//	WCHAR szDZRegParams[4*1024] = L"";
//
//	// Get OLReg/OLRStateCheck file path
//	GetModuleFileName(NULL, szCurrentDir, _MAX_PATH);
//	PathRemoveFileSpec(szCurrentDir);
//	PathAppend(szCurrentDir, L"OLRSubmission");
//	wcscpy_s(szCurrentOLReg, _MAX_PATH, szCurrentDir);
//	wcscpy_s(szCurrentOLRState, _MAX_PATH, szCurrentDir);
//	PathAppend(szCurrentOLReg, L"OLRSubmission.exe");
//	PathAppend(szCurrentOLRState, L"OLRStateCheck.exe");
//
//	// Set parameter to OLReg/OLRStateCheck
//	wcscpy_s(szParam, _MAX_PATH, L" /IsRegister");
//
//	if (PathFileExists(szCurrentDir))
//	{
//		STARTUPINFO 				StartupInfo;
//		PROCESS_INFORMATION	        ProcessInfo;
//		ZeroMemory(&StartupInfo, sizeof(StartupInfo));
//		ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
//
//		StartupInfo.cb				= sizeof(StartupInfo);
//		StartupInfo.dwFlags			= STARTF_USESHOWWINDOW;
//		StartupInfo.wShowWindow	    = SW_SHOWNORMAL;
//		StartupInfo.lpReserved		= NULL;
//		StartupInfo.lpDesktop		= NULL;
//		StartupInfo.lpTitle			= NULL;
//		StartupInfo.cbReserved2		= 0;
//		StartupInfo.lpReserved2		= NULL;
//
//		// Use "CreateProcess" instead of "ShellExecuteEx" because ShellExecuteEx may cause a bug that WaitForSingleObject may not work.
//		// Some computers return immediately from WaitForSingleObject when the process has not terminated yet.
//		// ShellExecuteEx is not stable, and it is recommended that UI should use "CreatePorcess" instead of "ShellExecuteEx".
//		WCHAR szCommandLine[_MAX_PATH] = L"";
//		wcscat_s(szCommandLine, szCurrentOLRState);
//		wcscat_s(szCommandLine, szParam);
//
//		// create shared memory for OLReg/OLRStateCheck
//		//================================================================================
//		HANDLE hFileMapping = CreateFileMapping (INVALID_HANDLE_VALUE, NULL, 
//			PAGE_READWRITE, 0, 4*1024, TEXT("OLRegSharedMemory"));
//			
//		if (!hFileMapping)
//		{
//			dprintf(L"[NetKernel] PyNetKernel::GetDZRegParams():FileMapping is wrong");
//			DeleteObject(hFileMapping);
//			if (ProcessInfo.hProcess)
//				CloseHandle(ProcessInfo.hProcess);
//			if (ProcessInfo.hThread)
//				CloseHandle(ProcessInfo.hThread);
//			return Py_BuildValue("bu", false, L"");
//		}
//		//================================================================================
//
//		if (!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 
//			NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &ProcessInfo))
//		{
//			dprintf(L"[NetKernel] PyNetKernel::GetDZRegParams():Fail to create OLRStateCheck process: %s", szCommandLine);
//			DeleteObject(hFileMapping);
//			if (ProcessInfo.hProcess)
//				CloseHandle(ProcessInfo.hProcess);
//			if (ProcessInfo.hThread)
//				CloseHandle(ProcessInfo.hThread);
//			return Py_BuildValue("bu", false, L"");
//		}
//
//		// wait for OLRStateCheck.exe 
//		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
//
//		// read shared memory
//		WCHAR* lpcwData = (WCHAR*)MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE , 0, 0, 0);
//		if (!lpcwData)
//		{
//			dprintf(L"[NetKernel] PyNetKernel::GetDZRegParams():MapViewOfFile is NULL");
//		}
//		else
//		{
//			wcscpy_s(szIsRegister, _MAX_PATH, lpcwData);
//
//			UnmapViewOfFile(lpcwData);
//			ZeroMemory(szCommandLine, sizeof(szCommandLine));
//			ZeroMemory(szParam, sizeof(szParam));
//			wcscpy_s(szParam, L" /DirectZone");
//			wcscat_s(szCommandLine, szCurrentOLReg);
//			wcscat_s(szCommandLine, szParam);
//
//			if (!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 
//				NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &ProcessInfo))
//			{
//				dprintf(L"[NetKernel] PyNetKernel::GetDZRegParams():Fail to create OLRSubmission process: %s", szCommandLine);
//				DeleteObject(hFileMapping);
//				if (ProcessInfo.hProcess)
//					CloseHandle(ProcessInfo.hProcess);
//				if (ProcessInfo.hThread)
//					CloseHandle(ProcessInfo.hThread);
//				return Py_BuildValue("bu", false, L"");
//			}
//
//			// wait for OLReg
//			WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
//
//			// read shared memory
//			WCHAR* lpcwURL = (WCHAR*)MapViewOfFile(hFileMapping, FILE_MAP_READ | FILE_MAP_WRITE , 0, 0, 0);
//			if (!lpcwURL)
//			{
//				dprintf(L"[NetKernel] PyNetKernel::GetDZRegParams():MapViewOfFile is NULL");
//			}
//			else
//			{
//				wcscpy_s(szDZRegParams, 4*1024, lpcwURL);
//				UnmapViewOfFile(lpcwURL);
//			}
//		} 
//		DeleteObject(hFileMapping);
//
//		DWORD dwRet = 1;
//		BOOL bRet = GetExitCodeProcess(ProcessInfo.hProcess, &dwRet);
//		if (bRet && 0 == dwRet)	
//		{
//			hr = S_OK;
//		}
//
//		if (ProcessInfo.hProcess)
//			CloseHandle(ProcessInfo.hProcess);
//		if (ProcessInfo.hThread)
//			CloseHandle(ProcessInfo.hThread);
//	}
//
//	if (hr == S_OK && szIsRegister && szDZRegParams)
//	{
//		if (wcsstr(szIsRegister, L"FALSE") != NULL)
//			return Py_BuildValue("bu", false, szDZRegParams);
//		else
//			return Py_BuildValue("bu", true, szDZRegParams);
//	}
//
//	return Py_BuildValue("bu", false, L"");
//}

void CNetKernel::SetHaveRegToOLREG()
{
	WCHAR szCurrentDir[_MAX_PATH] = L"";
	WCHAR szCurrentOLReg[_MAX_PATH] = L"";
	WCHAR szParam[_MAX_PATH] = L"";
	WCHAR szHasRegister[_MAX_PATH] = L"";

	// Get OLReg/OLRStateCheck file path
	GetModuleFileName(NULL, szCurrentDir, _MAX_PATH);
	PathRemoveFileSpec(szCurrentDir);
	PathAppend(szCurrentDir, L"OLRSubmission");
	wcscpy_s(szCurrentOLReg, _MAX_PATH, szCurrentDir);
	PathAppend(szCurrentOLReg, L"OLRSubmission.exe");

	// Set parameter to OLReg/OLRStateCheck
	wcscpy_s(szParam, _MAX_PATH, L" /HasRegister");

	if (PathFileExists(szCurrentDir))
	{
		STARTUPINFO 				StartupInfo;
		PROCESS_INFORMATION	        ProcessInfo;
		ZeroMemory(&StartupInfo, sizeof(StartupInfo));
		ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

		StartupInfo.cb				= sizeof(StartupInfo);
		StartupInfo.dwFlags			= STARTF_USESHOWWINDOW;
		StartupInfo.wShowWindow	    = SW_SHOWNORMAL;
		StartupInfo.lpReserved		= NULL;
		StartupInfo.lpDesktop		= NULL;
		StartupInfo.lpTitle			= NULL;
		StartupInfo.cbReserved2		= 0;
		StartupInfo.lpReserved2		= NULL;

		// Use "CreateProcess" instead of "ShellExecuteEx" because ShellExecuteEx may cause a bug that WaitForSingleObject may not work.
		// Some computers return immediately from WaitForSingleObject when the process has not terminated yet.
		// ShellExecuteEx is not stable, and it is recommended that UI should use "CreatePorcess" instead of "ShellExecuteEx".
		WCHAR szCommandLine[_MAX_PATH] = L"";
		wcscat_s(szCommandLine, szCurrentOLReg);
		wcscat_s(szCommandLine, szParam);

		if (!CreateProcess(NULL, szCommandLine, NULL, NULL, TRUE, 
			NORMAL_PRIORITY_CLASS, NULL, NULL, &StartupInfo, &ProcessInfo))
		{
			dprintf(L"[NetKernel] PyNetKernel::SetHaveRegToOLREG():Fail to create process: %s", szCommandLine);
			if (ProcessInfo.hProcess)
				CloseHandle(ProcessInfo.hProcess);
			if (ProcessInfo.hThread)
				CloseHandle(ProcessInfo.hThread);
			return;
		}

		// wait for OLRSubmission.exe 
		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);

		if (ProcessInfo.hProcess)
			CloseHandle(ProcessInfo.hProcess);
		if (ProcessInfo.hThread)
			CloseHandle(ProcessInfo.hThread);
	}
}


BOOL CNetKernel::ResolveUrl(const CHAR* lpszUri, UrlValueObject& cUriVO)
{
	return ResolveUri(lpszUri, cUriVO.strRqstUrl, cUriVO.bSecure, cUriVO.strHost, cUriVO.dwPort);
}

void CNetKernel::GetCacheFilePath( WCHAR* lpwszFileName )
{
	if (wcslen(m_lpcwszCookieFileName)==0) return;
	wcscpy_s(lpwszFileName,wcslen(m_lpcwszCookieFileName)+1,m_lpcwszCookieFileName);
}


HRESULT CacheCallbacker::OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
	if (!m_pNetKernel)
		return E_POINTER;
	//dprintf(L"[CacheCallbacker] Progress: %d  ProgressMax: %d  Status: %d  Text: %s",
	//	ulProgress, ulProgressMax, ulStatusCode, szStatusText);
	if(ulStatusCode == BINDSTATUS_DOWNLOADINGDATA ||
		ulStatusCode == BINDSTATUS_BEGINDOWNLOADDATA ||
		ulStatusCode == BINDSTATUS_ENDDOWNLOADDATA)
		m_pNetKernel->OnStateCallBack("receive", ulProgress, ulProgressMax);
	return S_OK;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//Init with dll main
static list<INetKernel*> g_listNetKernel;

extern "C" __declspec(dllexport) INetKernel* __cdecl GetSingletonInstance()
{
	static CNetKernel pyNetKernel;
	return &pyNetKernel;
}

extern "C" __declspec(dllexport) INetKernel* __cdecl GetInstance()
{
	CNetKernel* pInst = new CNetKernel();
	g_listNetKernel.push_back(pInst);
	return pInst;
}


extern "C" __declspec(dllexport) void __cdecl DeleteInstance (INetKernel* pInst)
{
	if (pInst!=NULL)
	{
		list<INetKernel*>::iterator it = find(g_listNetKernel.begin(),g_listNetKernel.end(),pInst);
		if (it!=g_listNetKernel.end() && *it == NULL)
		{
			delete *it;
			*it = NULL;
			g_listNetKernel.erase(it);
			pInst = NULL;
			return;
		}
	}

	for (list<INetKernel*>::iterator it=g_listNetKernel.begin();it!=g_listNetKernel.end();++it)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
	g_listNetKernel.clear();

}

#ifdef _MANAGED
#pragma managed(pop)
#endif

