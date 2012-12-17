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
}


BEGIN_MESSAGE_MAP(CDlgOpenUrl, CDialog)
END_MESSAGE_MAP()


// CDlgOpenUrl message handlers
