#pragma once


// CDlgOpenUrl dialog

class CDlgOpenUrl : public CDialog
{
	DECLARE_DYNAMIC(CDlgOpenUrl)

public:
	CDlgOpenUrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgOpenUrl();

// Dialog Data
	enum { IDD = IDD_DIALOG_OpenUrl };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szUrl;
	CString m_szMethod;
	CString m_szProxy;
	CString m_szHeader;
	CString m_szBody;
	UINT m_nContentLen;
};
