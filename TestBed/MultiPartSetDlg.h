#pragma once


// MultiPartSetDlg dialog

class MultiPartSetDlg : public CDialog
{
	DECLARE_DYNAMIC(MultiPartSetDlg)

public:
	MultiPartSetDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~MultiPartSetDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MULTIPART };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
