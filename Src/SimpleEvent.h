#pragma once

#include <Windows.h>

class __declspec(dllexport) CSimpleEvent
{
	// make copy constructor and assignment operator inaccessible
	//CSimpleEvent(const CSimpleEvent &refEvent);
	//CSimpleEvent &operator=(const CSimpleEvent &refEvent);

	HANDLE m_hEvent;    

public:

	// The 3rd parameter of constructor is FALSE, so the initial state is non-signaled.
	CSimpleEvent(BOOL fManualReset = FALSE) { m_hEvent = CreateEvent(NULL, fManualReset, FALSE, NULL); };
	~CSimpleEvent() { if (m_hEvent) CloseHandle(m_hEvent); };

	// Cast to HANDLE - we don't support this as an lvalue
	operator HANDLE () const { return m_hEvent; };

	void Set() { SetEvent(m_hEvent);} ;
	// Note: Wait() return TRUE means that the wait object is signaled;
	//				return FALSE means Wait-Timeout.
	//		 Remember, unless you use CSimpleEvent(TRUE) to create wait object, 
	//		 the object will become non-signaled state after Wait() function returning TRUE.	
	BOOL Wait(DWORD dwTimeout = INFINITE) { return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);};

	void Reset() { ResetEvent(m_hEvent); };
	// Note: If you want to use Check() to determine if the object signaled, you should use CSimpleEvent(TRUE) to create event object.
	//		 It will keep the signaled state for your polling; or everytime the object signaled it will become non-signaled automatically.	
	BOOL Check() { return Wait(0); };
};

