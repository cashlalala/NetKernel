// TestBedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestBed.h"
#include "TestBedDlg.h"
#include <WinInet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTestBedDlg dialog




CTestBedDlg::CTestBedDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestBedDlg::IDD, pParent)
	, m_szUrl(_T("http://www.google.com"))
	, m_szMethod(_T("GET"))
	, m_szProxy(_T(""))
	, m_szHeader(_T(""))
	, m_szBody(_T(""))
	, m_nContentLen(0)
	, m_szOutput(_T(""))
	, m_nPort((int)INTERNET_DEFAULT_HTTP_PORT)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pNetKernel = ((CTestBedApp*)AfxGetApp())->m_pNetKernel;
}

void CTestBedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_TAB1, m_ctrlTabView);
	DDX_Text(pDX, IDC_EDIT_OPENURL_URL, m_szUrl);
	DDX_Text(pDX, IDC_EDIT_OPENURL_METHOD, m_szMethod);
	DDX_Text(pDX, IDC_EDIT_OPENURL_PROXY, m_szProxy);
	DDX_Text(pDX, IDC_EDIT_OPENURL_HEADER, m_szHeader);
	DDX_Text(pDX, IDC_EDIT_OPENURL_BODY, m_szBody);
	DDX_Text(pDX, IDC_EDIT_OPENURL_CONTENTLEN, m_nContentLen);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_szOutput);
	DDX_Text(pDX, IDC_EDIT_OPENURL_CONTENTLEN2, m_nPort);
}

BEGIN_MESSAGE_MAP(CTestBedDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	//ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CTestBedDlg::OnTcnSelchangeTab1)
	//ON_NOTIFY(NM_CLICK, IDC_TAB1, &CTestBedDlg::OnNMClickTab1)
	ON_BN_CLICKED(IDC_BUTTON_OpenUrl, &CTestBedDlg::OnBnClickedButtonOpenurl)
	ON_BN_CLICKED(IDC_BUTTON_SendHttpRequest, &CTestBedDlg::OnBnClickedButtonSendhttprequest)
END_MESSAGE_MAP()


// CTestBedDlg message handlers

BOOL CTestBedDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//this->InitTabCtrl();
	this->UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestBedDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestBedDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestBedDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//void CTestBedDlg::InitTabCtrl(void)
//{
//	m_ctrlTabView.InsertItem(0,_T("OpenUrl"));
//	m_ctrlTabView.InsertItem(1,_T("SendHttpRequest "));
//
//	m_dlgTabItemAry[0].Create(IDD_DIALOG_OpenUrl, GetDlgItem(IDC_TAB1));
//	m_dlgTabItemAry[1].Create(IDD_DIALOG_SendHttpRequest,GetDlgItem(IDC_TAB1));
//
//	CRect rc;
//	m_ctrlTabView.GetClientRect(rc);
//	
//	rc.top += 25;
//	rc.bottom -= 5;
//	rc.left += 5;
//	rc.right -= 5;
//
//	m_dlgTabItemAry[0].MoveWindow(&rc);
//	m_dlgTabItemAry[1].MoveWindow(&rc);
//
//	m_dlgTabItemAry[0].ShowWindow(SW_SHOW);
//	m_dlgTabItemAry[1].ShowWindow(SW_HIDE);
//
//	m_ctrlTabView.SetCurSel(0);
//}


//void CTestBedDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
//{
//	// TODO: Add your control notification handler code here
//	m_nCurTabIdx = m_ctrlTabView.GetCurSel();
//	for (int i=0;i<TOTAL_TEST_FUNC_COUNT;++i)
//	{
//		if (i == m_nCurTabIdx)
//		{
//			m_dlgTabItemAry[i].ShowWindow(SW_SHOW);
//		}
//		else
//		{
//			m_dlgTabItemAry[i].ShowWindow(SW_HIDE);
//		}
//	}
//
//	*pResult = 0;
//}



void CTestBedDlg::OnBnClickedButtonOpenurl()
{
	UpdateData(TRUE);
	// TODO: 在此加入控制項告知處理常式程式碼
	HttpResponse httpResp;

	m_pNetKernel->OpenUrl(httpResp, CT2CA(m_szUrl),CT2CA(m_szMethod));

	CString szResp(httpResp.strResponse);
	m_szOutput.Format(_T("Error: %d, HttpStatus: %d \r\n%s"),httpResp.dwError, httpResp.dwStatusCode,szResp);

	UpdateData(FALSE);
}


void CTestBedDlg::OnBnClickedButtonSendhttprequest()
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	HttpResponse httpResp;
	const wchar_t* lpszUrl = (LPCTSTR)m_szUrl;
	char* lpszUrlTmp = new char[m_szUrl.GetLength()+1];

	WideCharToMultiByte(CP_ACP,NULL,lpszUrl,-1,lpszUrlTmp,m_szUrl.GetLength()+1,0,0);

	UriValueObject cUriVO;
	m_pNetKernel->ResolveUrl(lpszUrlTmp,cUriVO);

	m_pNetKernel->SendHttpRequest(httpResp,
														"NetKernelTestBed",
														CStringA(m_szMethod),
														cUriVO.strHost.c_str(),
														m_nPort,FALSE, 
														cUriVO.strUrl.c_str(),
														CStringA(m_szHeader));

	CString szResp(httpResp.strResponse);
	m_szOutput.Format(_T("Error: %d, HttpStatus: %d \r\n%s"),httpResp.dwError, httpResp.dwStatusCode,szResp);
	delete[] lpszUrlTmp;

	UpdateData(FALSE);

}
