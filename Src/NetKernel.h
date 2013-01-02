// NetKernel.h: interface for the PyNetKernel class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "koan.h"


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <wininet.h>
#include <vector>
#include <Urlmon.h>
#include "SimpleEvent.h"

#include "INetKernel.h"

const int CALLBACK_OK = 1;
const int CALLBACK_CANCEL = 0;
const int ERROR_PROCESS		= -1;	// Prepare data error or process data error.
const int ERROR_FORCECCLOSE	= -2;	// Force close the internet handle.
const BOOL CACHE_USE	= TRUE;
const BOOL CACHE_NOUSE	= FALSE;

#ifndef __SWIG__
class CacheCallbacker;
class PyNetKernel;
#endif

void SetDumpFile(BOOL isDump, const WCHAR* lpwszPath);

inline std::string genBoundary();
PyObject* genPyBoundary();

class PyNetKernel : public PyCallback, public INetKernel
{
public:
	friend CacheCallbacker;
	
	//static PyNetKernel* m_pInstance;

	PyNetKernel();
	virtual ~PyNetKernel();

	void SetCallback(PyObject* callback);

	DWORD SendHttpRequest(HttpRespValObj& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer, DWORD dwPort,
		BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpszBody = NULL, const WCHAR* lpwszResponse = NULL,
		const WCHAR* lpwszDump = NULL);

	DWORD SendHttpContent(HttpRespValObj& httpResp, const CHAR* lpszApName, const CHAR* lpszMethod, const CHAR* lpszServer,
		DWORD wPort, BOOL bSecure, const CHAR* lpszUrl, const CHAR* lpszHeader, const CHAR* lpBody, DWORD dwLength,
		const WCHAR* lpwszResponse, const WCHAR* lpwszDump = NULL);

	DWORD SendHttpRequestMultipart(HttpRespValObj& httpResp, const CHAR* lpszApName, const CHAR* lpszUri, const CHAR* lpszMethod,
		const WCHAR* lpwszProxy, const CHAR* lpszHeader, std::vector<MultiPartInfo> vecMultiPart, DWORD dwContentLength,
		const WCHAR* lpwszResponse = NULL, const WCHAR* lpwszDump = NULL);

	DWORD OpenUrl(HttpRespValObj& httpResp, const CHAR* lpszUri, const CHAR* lpszMethod = NULL, const WCHAR* lpwszProxy = NULL, const CHAR* lpszHeader = NULL, const WCHAR* lpwszResponse = NULL, const CHAR* pBodyBuffer = NULL, DWORD dwContentLength = 0);

	PyObject* SendUrlRequest(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
		const CHAR* pBodyBuffer = NULL, DWORD dwBodyLength = 0);

	PyObject* ReceiveUrlData(DWORD dwContentLength, const WCHAR* lpwszResponse = NULL);

	BOOL ResolveUrl(const CHAR* lpszUrl, UrlValueObject& cUriVO);

	BOOL DeleteUrlCache(int type, const WCHAR* lpwszCookieName);

	//PyObject* OpenUrlCache(const CHAR* lpszUri);

	void ForceStop();
	void SetWindowHandle(HWND hWnd);
	void SetDownloadCache(BOOL bCacheDownload);
	PyObject* GetDZRegParams();
	void SetHaveRegToOLREG();

	void GetCacheFilePath(WCHAR* lpwszFileName);

#ifndef __SWIG__
private:
	long OnStateCallBack(const char* lpszState, int nCurrent, int nTotal);

	BOOL SendHttpReq(HINTERNET& hRequest, BOOL bUseProxy, const CHAR* lpszHeader, LPVOID lpOptional, DWORD dwLength, DWORD& dwStatusCode);

	BOOL PrepareConnection(HINTERNET& hInternet, HINTERNET& hConnect, HINTERNET& hRequest, const CHAR* lpszApName,
		const CHAR* lpszMethod, const CHAR* lpszServer, DWORD dwPort, BOOL bSecure, const CHAR* lpszUrl);

	BOOL QueryResponseInfo(HINTERNET& hRequest, DWORD& dwStatusCode, DWORD& dwResLength, std::string& contentType);
	
	BOOL ReceiveTextResponse(HINTERNET& hRequest, DWORD dwContentLength, std::string& strServerResponse);

	BOOL ReceiveResponseToFile(HINTERNET& hRequest, DWORD dwContentLength, const WCHAR* lpwszResponse);

	BOOL ReceiveResponseToBuffer(HINTERNET& hRequest, DWORD& dwAvailableData, char* &pResBuffer);

	BOOL ReceiveResponse(HINTERNET& hRequest, DWORD& dwStatusCode, const WCHAR* lpwszResponse,
		std::string& strServerResponse, DWORD& dwError, const WCHAR* lpwszDump);
	
	void CloseInternetHandle(HINTERNET& hRequest, HINTERNET& hConnect, HINTERNET& hInternet);
	
	BOOL WriteInternetContent(HINTERNET& hRequest, const char* lpContent, DWORD dwLength, DWORD& dwSent, DWORD dwContentLength);

	BOOL WriteMultipartBody(HINTERNET& hRequest, std::vector<MultiPartInfo>& infoList, DWORD dwContentLength);

	BOOL SendUrlRequestImpl(const CHAR* lpszUri, const CHAR* lpszMethod, const WCHAR* lpwszProxy, const CHAR* lpszHeader,
		const CHAR* pBodyBuffer, DWORD dwContentLengthm, DWORD& dwStatusCode, DWORD& dwAvailableData, DWORD& dwError);

	BOOL ReceiveUrlDataImpl(DWORD& dwContentLength, const WCHAR* lpwszResponse, char*& pResBuffer, DWORD& dwError);


private:
	HWND m_hWnd;
	HINTERNET m_hInternet;
	HINTERNET m_hConnect;
	HINTERNET m_hRequest;

	BOOL m_bForceClose;
	BOOL m_bCacheDownload;
#endif
	WCHAR m_lpcwszCookieFileName[MAX_PATH+1];

	CSimpleEvent m_cSimpleEvent;
};

#ifndef __SWIG__
class CacheCallbacker : public IBindStatusCallback
{
public:
	CacheCallbacker(PyNetKernel* pNetKernel) : m_cRef(0), m_pNetKernel(pNetKernel) {}
	virtual ~CacheCallbacker() {}

	virtual HRESULT STDMETHODCALLTYPE GetBindInfo(DWORD *grfBINDF, BINDINFO *pbindinfo) {return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE GetPriority(LONG *pnPriority) {return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnLowResource(DWORD reserved) {return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed) {return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(REFIID riid, IUnknown *punk ) {return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnProgress(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText);
	virtual HRESULT STDMETHODCALLTYPE OnStartBinding(DWORD dwReserved, IBinding *pib) {return S_OK;}
	virtual HRESULT STDMETHODCALLTYPE OnStopBinding(HRESULT hresult, LPCWSTR szError) {return S_OK;}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)
	{
		if (!ppv)
			return E_POINTER;
		IUnknown *punk = NULL;
		if(riid == IID_IUnknown)
			punk = static_cast<IUnknown*>(this);
		*ppv = punk;
		if(!punk)
			return E_NOINTERFACE;
		punk->AddRef();
		return S_OK;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return ++m_cRef;
	}
	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		int cRef = --m_cRef;
		if(cRef == 0)
			delete this;
		return cRef;
	};

private:
	int m_cRef;
	PyNetKernel* m_pNetKernel;
};
#endif

