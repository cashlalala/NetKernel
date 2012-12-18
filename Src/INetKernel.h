#pragma once

#include <string>
#include <vector>

struct HttpResponse
{
	HttpResponse()
	{
		dwError = 0;
		dwStatusCode = 0;
		strResponse.clear();
	}
	DWORD dwError;
	DWORD dwStatusCode;
	std::string strResponse;
};

// Structure for file send request.
struct MultiPartInfo
{
	MultiPartInfo()
	{
		filePath.clear();
		content.clear();
		header.clear();
		dwFileSize = 0;
	}
	std::wstring filePath;	// Full file path.
	std::string content;	// multi part content
	std::string header;		// Each multipart header.
	DWORD dwFileSize;		// The file size.
};

struct INetKernel
{
	//void SetCallback(PyObject* callback);

	virtual DWORD SendHttpRequest(HttpResponse& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer, DWORD dwPort,
		BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpszBody = NULL, const WCHAR* lpwszResponse = NULL,
		const WCHAR* lpwszDump = NULL) = 0;

	//not used 
	//virtual DWORD SendHttpContent(HttpResponse& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer,
	//	DWORD wPort, BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpBody, DWORD dwLength,
	//	const WCHAR* lpwszResponse, const WCHAR* lpwszDump = NULL) = 0;

	virtual DWORD SendHttpRequestMultipart(HttpResponse& httpResp, const CHAR* lpszApName, const CHAR* lpszUri, const CHAR* lpszMethod,
		const WCHAR* lpwszProxy, const CHAR* lpszHeader, std::vector<MultiPartInfo> vecMultiPart, DWORD dwContentLength,
		const WCHAR* lpwszResponse = NULL, const WCHAR* lpwszDump = NULL) = 0;

	virtual DWORD OpenUrl(HttpResponse& httpResp, const CHAR* lpszUri, const CHAR* lpszMethod = NULL, const WCHAR* lpwszProxy = NULL, const CHAR* lpszHeader = NULL, const WCHAR* lpwszResponse = NULL, const CHAR* pBodyBuffer = NULL, DWORD dwContentLength = 0) = 0;

	//PyObject* SendUrlRequest(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
	//	const CHAR* pBodyBuffer = NULL, DWORD dwBodyLength = 0);

	//not used
	//PyObject* ReceiveUrlData(DWORD dwContentLength, const WCHAR* lpwszResponse = NULL);

	virtual BOOL DeleteUrlCache(int type, const WCHAR* lpwszCookieName) = 0;

	virtual void ForceStop() = 0;
	virtual void SetDownloadCache(BOOL bCacheDownload) = 0;

	typedef  INetKernel* (*PFNGETINSTANCE)();
	typedef  void (*PFNDELINSTANCE)();
};

