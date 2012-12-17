#include "StdAfx.h"
#include "NetKernelLoader.h"
#include <Shlwapi.h>
#include "..\..\Utility\Debug.h"

NetKernelLoader::NetKernelLoader(void)
{
	LoadDll();
}


NetKernelLoader::~NetKernelLoader(void)
{
	FreeLibrary(m_hNetKernel);
	m_hNetKernel = NULL;
}


BOOL NetKernelLoader::LoadDll( void )
{
	CString strModuleFolder;
	TCHAR szExePath[_MAX_PATH];
	GetModuleFileName(NULL, szExePath, _MAX_PATH);
	PathRemoveFileSpec(szExePath);
	PathAddBackslash(szExePath);
	strModuleFolder = szExePath;
	_tcscat_s(szExePath, _T("NetKernel.dll"));
	m_hNetKernel = LoadLibrary(szExePath);

	if (!m_hNetKernel)
	{
		dprintf(L"[PyOnlineMgr] LoadDLL(): The OnlineMgr DLL object load failed! Error code: %d", GetLastError());
		return FALSE;
	}

	return TRUE;
}


INetKernel* NetKernelLoader::GetInstance( void )
{
	//PFNGETINSTANCE pFn = (PFNGETINSTANCE) GetProcAddress(m_hNetKernel,"_GetNetKernelInstance@0");
	PFNGETINSTANCE pFn = (PFNGETINSTANCE) GetProcAddress(m_hNetKernel,"GetNetKernelInstance");
	return pFn();
}


void NetKernelLoader::DelInstance(void)
{
	//PFNDELINSTANCE pFn = (PFNDELINSTANCE) GetProcAddress(m_hNetKernel,"_DelInstance@0");
	PFNDELINSTANCE pFn = (PFNDELINSTANCE) GetProcAddress(m_hNetKernel,"DelInstance");
	pFn();
}
