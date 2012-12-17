#pragma once
#include "..\Src\INetKernel.h"

class NetKernelLoader
{
public:
	NetKernelLoader(void);
	~NetKernelLoader(void);
private:
	BOOL LoadDll(void);
public:
	HMODULE m_hNetKernel;
	INetKernel* GetInstance(void);
	void DelInstance(void);
};

