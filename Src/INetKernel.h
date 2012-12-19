#pragma once

/********************************************************************
	created:	2012/12/18
	created:	18:12:2012   17:26
	filename: 	F:\power2go9\subsys\NetKernel\Src\INetKernel.h
	file path:	F:\power2go9\subsys\NetKernel\Src
	file base:	INetKernel
	file ext:	h
	author:		Cash_Chang
	dependency: VC9.0
	purpose:	
*********************************************************************/

#include <string>
#include <vector>


struct HttpResponseValueObject
{
	HttpResponseValueObject()
	{
		dwError = 0;
		dwStatusCode = 0;
	}
	DWORD dwError; //The error code returned by Win32 API
	DWORD dwStatusCode; //The status code of HTTP
	std::string strResponse; //The response of HTTP
};

struct UriValueObject
{
	/*
	* sample url: https://maps.google.com/maps?hl=zh-TW&tab=wl
	* strRqstUrl: /maps?hl=zh-TW&tab=wl
	* bSecure: True
	* strHost: maps.google.com
	* dwPort : 0 ---(default)-->443 (the default port of https is 443, and the one of http is 80)
	*/
	UriValueObject()
	{
		bSecure = FALSE;
		dwPort = 0;
	}
	std::string strRqstUrl; //the path without protocol and host
	BOOL bSecure; //if the protocol is https, this value will be set
	std::string strHost; //the host
	DWORD dwPort; //port
};


// Structure for file send request.
struct MultiPartInfo
{
	MultiPartInfo()
	{
		dwFileSize = 0;
	}
	std::wstring filePath;	// Full file path.
	std::string content;	// multi part content
	std::string header;		// Each multipart header.
	DWORD dwFileSize;		// The file size.
};

enum DEL_CACHE_TYPE
{
	File,
	Cookie
};

struct INetKernel
{
	//void SetCallback(PyObject* callback);

	//not used 
	//virtual DWORD SendHttpContent(HttpResponse& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer,
	//	DWORD wPort, BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpBody, DWORD dwLength,
	//	const WCHAR* lpwszResponse, const WCHAR* lpwszDump = NULL) = 0;

	virtual DWORD SendHttpRequestMultipart(HttpResponseValueObject& httpResp, const CHAR* lpszApName, const CHAR* lpszUri, const CHAR* lpszMethod,
		const WCHAR* lpwszProxy, const CHAR* lpszHeader, std::vector<MultiPartInfo> vecMultiPart, DWORD dwContentLength,
		const WCHAR* lpwszResponse = NULL, const WCHAR* lpwszDump = NULL) = 0;

	virtual DWORD SendHttpRequest(HttpResponseValueObject& httpResp, 
														const CHAR* lpszApName, 
														const CHAR* lpszMethod, 
														const CHAR* lpszServer, 
														DWORD dwPort,
														BOOL bSecure, 
														const CHAR* lpszUrl, 
														const CHAR* lpszHeader, 
														const CHAR* lpszBody = NULL, 
														const WCHAR* lpwszResponse = NULL,
														const WCHAR* lpwszDump = NULL) 
														= 0;

	virtual DWORD OpenUrl(HttpResponseValueObject& httpResp, 
											const CHAR* lpszUri, 
											const CHAR* lpszMethod = NULL, 
											const WCHAR* lpwszProxy = NULL, 
											const CHAR* lpszHeader = NULL, 
											const WCHAR* lpwszResponse = NULL, 
											const CHAR* pBodyBuffer = NULL, 
											DWORD dwContentLength = 0) //The size of body in Bytes. If the Body is mixed with unicode and ascii, please assign the right size of the string. 
											= 0;

	//PyObject* SendUrlRequest(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
	//	const CHAR* pBodyBuffer = NULL, DWORD dwBodyLength = 0);

	//not used
	//PyObject* ReceiveUrlData(DWORD dwContentLength, const WCHAR* lpwszResponse = NULL);

	virtual BOOL DeleteUrlCache(int type, const WCHAR* lpwszCookieName) = 0;

	virtual void ForceStop() = 0;
	virtual void SetDownloadCache(BOOL bCacheDownload) = 0;

	virtual BOOL ResolveUrl(const CHAR* lpszUri, UriValueObject& cUriVO) = 0;

	//For debug use
	virtual void GetCacheFileName(WCHAR* lpwszFileName) = 0;

	typedef  INetKernel* (*PFNGETINSTANCE)();
	typedef  void (*PFNDELINSTANCE)();
};

