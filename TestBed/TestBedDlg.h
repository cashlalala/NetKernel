// TestBedDlg.h : header file
//

#pragma once
#include "afxcmn.h"

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
	CTabCtrl m_ctrlTabView;
	int m_nCurTabIdx;
private:
	void InitTabCtrl(void);
	CDialog m_dlgTabItemAry[TOTAL_TEST_FUNC_COUNT];
public:
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
};