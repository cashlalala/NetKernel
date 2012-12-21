#pragma once
#include "afxwin.h"


// MultiPartDlg dialog

class MultiPartDlg : public CDialog
{
	DECLARE_DYNAMIC(MultiPartDlg)

public:
	MultiPartDlg(CWnd* pParent = NULL);   // standard constructor
	MultiPartDlg(std::vector<MultiPartInfo> vecItemList);
	virtual ~MultiPartDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MULTIPART };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CString m_szContent;
	CString m_szHeader;
	CString m_szFile;
	DWORD m_nSize;
	afx_msg void OnBnClickedButtonOpenfile();
	afx_msg void OnBnClickedButtonAdd();
	CListBox m_ctrlItemList;
	afx_msg void OnBnClickedButtonDelete();
	std::vector<MultiPartInfo> m_vecMultiPartInfo;
	CString m_szItemStrings;
	afx_msg void OnLbnSelchangeListItemlist();
	afx_msg void OnBnClickedButtonSave();
	int m_nContentSize;
	ULONGLONG m_nFileSize;
	afx_msg void OnEnChangeEditContent();
	void Clear(void);
	bool m_bIsFromSsave;
	afx_msg void OnEnChangeEditHeader();
	int m_nHeaderSize;
};
