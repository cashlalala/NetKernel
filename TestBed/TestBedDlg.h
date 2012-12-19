// TestBedDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#define TOTAL_TEST_FUNC_COUNT 2

// CTestBedDlg dialog
class CTestBedDlg : public CDialog
{
// Construction
public:
	CTestBedDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTBED_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	/*CTabCtrl m_ctrlTabView;
	int m_nCurTabIdx;*/
private:
	//void InitTabCtrl(void);
	//CDialog m_dlgTabItemAry[TOTAL_TEST_FUNC_COUNT];
public:
	//afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonOpenurl();
	CString m_szUrl;
	CString m_szMethod;
	CString m_szProxy;
	CString m_szHeader;
	CString m_szBody;
	UINT m_nContentLen;
	CString m_szOutput;
	afx_msg void OnBnClickedButtonSendhttprequest();
private:
	INetKernel* m_pNetKernel;
public:
	UINT m_nPort;
	afx_msg void OnEnChangeEditOpenurlBody();
	CComboBox m_ctrlComboMethod;
	afx_msg void OnCbnSelchangeComboMethod();
	afx_msg void OnBnClickedCheckDownloadcache();
	BOOL m_bIsCache;
	CString m_szCacheName;
};
