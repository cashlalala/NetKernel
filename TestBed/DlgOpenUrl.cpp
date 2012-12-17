// DlgOpenUrl.cpp : implementation file
//

#include "stdafx.h"
#include "TestBed.h"
#include "DlgOpenUrl.h"


// CDlgOpenUrl dialog

IMPLEMENT_DYNAMIC(CDlgOpenUrl, CDialog)

CDlgOpenUrl::CDlgOpenUrl(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgOpenUrl::IDD, pParent)
	, m_szUrl(_T("http://www.google.com"))
	, m_szMethod(_T("GET"))
	, m_szProxy(_T(""))
	, m_szHeader(_T(""))
	, m_szBody(_T(""))
	, m_nContentLen(0)
{

}

CDlgOpenUrl::~CDlgOpenUrl()
{
}

void CDlgOpenUrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_OPENURL_URL, m_szUrl);
	DDX_Text(pDX, IDC_EDIT_OPENURL_METHOD, m_szMethod);
	DDX_Text(pDX, IDC_EDIT_OPENURL_PROXY, m_szProxy);
	DDX_Text(pDX, IDC_EDIT_OPENURL_HEADER, m_szHeader);
	DDX_Text(pDX, IDC_EDIT_OPENURL_BODY, m_szBody);
	DDX_Text(pDX, IDC_EDIT_OPENURL_CONTENTLEN, m_nContentLen);
	DDX_Control(pDX, IDC_BUTTON_SEND, m_btnSend);
}


BEGIN_MESSAGE_MAP(CDlgOpenUrl, CDialog)
	//ON_BN_CLICKED(IDC_BUTTON_SEND, &CDlgOpenUrl::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CDlgOpenUrl::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgOpenUrl::OnBnClickedButton2)
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


// CDlgOpenUrl message handlers


void CDlgOpenUrl::OnBnClickedButtonSend()
{
	// TODO: Add your control notification handler code here
	HttpResponse httpResp;
	const wchar_t* lpszUrl = (LPCTSTR)m_szUrl;
	char* lpszUrlTmp = new char[m_szUrl.GetLength()+1];
	WideCharToMultiByte(CP_ACP,NULL,lpszUrl,sizeof(lpszUrl),lpszUrlTmp,m_szUrl.GetLength()+1,0,0);
	((CTestBedApp*)AfxGetApp())->m_pNetKernel->OpenUrl(httpResp, lpszUrlTmp);
}



void CDlgOpenUrl::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	HttpResponse httpResp;
	const wchar_t* lpszUrl = (LPCTSTR)m_szUrl;
	char* lpszUrlTmp = new char[m_szUrl.GetLength()+1];
	WideCharToMultiByte(CP_ACP,NULL,lpszUrl,sizeof(lpszUrl),lpszUrlTmp,m_szUrl.GetLength()+1,0,0);
	((CTestBedApp*)AfxGetApp())->m_pNetKernel->OpenUrl(httpResp, lpszUrlTmp);
}


void CDlgOpenUrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnLButtonDblClk(nFlags, point);
}
