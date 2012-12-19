#pragma once


// MultiPartDlg dialog

class MultiPartDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MultiPartDlg)

public:
	MultiPartDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~MultiPartDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MULTIPART };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
