#pragma once

#include <string>

struct HttpResponse
{
	DWORD dwError;
	DWORD dwStatusCode;
	std::string strResponse;
};

struct INetKernel
{
	//void SetCallback(PyObject* callback);

	//PyObject* SendHttpRequest(const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer, DWORD dwPort,
	//	BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpszBody = NULL, const WCHAR* lpwszResponse = NULL,
	//	const WCHAR* lpwszDump = NULL);

	//PyObject* SendHttpContent(const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer,
	//	DWORD wPort, BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpBody, DWORD dwLength,
	//	const WCHAR* lpwszResponse, const WCHAR* lpwszDump = NULL);

	//PyObject* SendHttpRequestMultipart(const CHAR* lpszApName, const CHAR* lpszUri, const CHAR* lpszMethod,
	//	const WCHAR* lpwszProxy, const CHAR* lpszHeader, PyObject* pPyMultiPart, DWORD dwContentLength,
	//	const WCHAR* lpwszResponse = NULL, const WCHAR* lpwszDump = NULL);

	virtual DWORD OpenUrl(HttpResponse& httpResp, const CHAR* lpszUri, const CHAR* lpszMethod = NULL, const WCHAR* lpwszProxy = NULL, const CHAR* lpszHeader = NULL, const WCHAR* lpwszResponse = NULL, const CHAR* pBodyBuffer = NULL, DWORD dwContentLength = 0) = 0;

	//PyObject* SendUrlRequest(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
	//	const CHAR* pBodyBuffer = NULL, DWORD dwBodyLength = 0);

	//PyObject* ReceiveUrlData(DWORD dwContentLength, const WCHAR* lpwszResponse = NULL);

	//PyObject* DeleteUrlCache(int type, const WCHAR* lpwszCookieName);

	typedef  INetKernel* (*PFNGETINSTANCE)();
	typedef  void (*PFNDELINSTANCE)();
};

