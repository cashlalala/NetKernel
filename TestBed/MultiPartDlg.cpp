// MultiPartDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestBed.h"
#include "MultiPartDlg.h"
#include "Shlwapi.h"
#include <vector>

using std::vector;
// MultiPartDlg dialog

IMPLEMENT_DYNAMIC(MultiPartDlg, CDialog)

MultiPartDlg::MultiPartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MultiPartDlg::IDD, pParent)
	, m_szContent(_T(""))
	, m_szHeader(_T(""))
	, m_szFile(_T(""))
	, m_nSize(0)
	, m_szItemStrings(_T(""))
	, m_nContentSize(0)
	, m_nFileSize(0)
	, m_bIsFromSsave(false)
	, m_nHeaderSize(0)
	, m_szHeaderRef(_T(""))
{

}

MultiPartDlg::MultiPartDlg( std::vector<MultiPartInfo> vecItemList )
	: CDialog(MultiPartDlg::IDD, NULL)
	, m_szContent(_T(""))
	, m_szHeader(_T(""))
	, m_szFile(_T(""))
	, m_nSize(0)
	, m_szItemStrings(_T(""))
	, m_nContentSize(0)
	, m_nFileSize(0)
	, m_nHeaderSize(0)
{
	m_vecMultiPartInfo = vecItemList;
}

MultiPartDlg::~MultiPartDlg()
{
}

void MultiPartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX,IDC_EDIT_FILEPATH,m_szFile);
	DDX_Text(pDX,IDC_EDIT_HEADER,m_szHeader);
	DDX_Text(pDX,IDC_EDIT_CONTENT,m_szContent);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_nSize);
	DDX_Control(pDX, IDC_LIST_ITEMLIST, m_ctrlItemList);
	DDX_LBString(pDX, IDC_LIST_ITEMLIST, m_szItemStrings);
	DDX_Text(pDX, IDC_STATIC_ITEM_CONTENT, m_nContentSize);
	DDX_Text(pDX, IDC_STATIC_FILE_SIZE, m_nFileSize);
	DDX_Text(pDX, IDC_STATIC_HEADER_SIZE, m_nHeaderSize);
	DDX_Text(pDX, IDC_EDIT1, m_szHeaderRef);
}


BEGIN_MESSAGE_MAP(MultiPartDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OPENFILE, &MultiPartDlg::OnBnClickedButtonOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &MultiPartDlg::OnBnClickedButtonDelete)
	ON_LBN_SELCHANGE(IDC_LIST_ITEMLIST, &MultiPartDlg::OnLbnSelchangeListItemlist)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &MultiPartDlg::OnBnClickedButtonSave)
	ON_EN_CHANGE(IDC_EDIT_CONTENT, &MultiPartDlg::OnEnChangeEditContent)
	ON_EN_CHANGE(IDC_EDIT_HEADER, &MultiPartDlg::OnEnChangeEditHeader)
	ON_BN_CLICKED(IDC_BUTTON_REMOVEALL, &MultiPartDlg::OnBnClickedButtonRemoveall)
	ON_BN_CLICKED(IDC_BUTTON2, &MultiPartDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT_FILEPATH, &MultiPartDlg::OnEnChangeEditFilepath)
END_MESSAGE_MAP()


// MultiPartDlg message handlers


void MultiPartDlg::OnBnClickedButtonOpenfile()
{
	UpdateData(TRUE);
	CString szFilter = _T("*.*|*.*||");
	CFileDialog dlgFile(TRUE,_T("*"),_T("*.*"),OFN_HIDEREADONLY,szFilter,m_pParentWnd);
	if (dlgFile.DoModal()==IDOK)
	{
		m_szFile = dlgFile.GetPathName();
		if (PathFileExists(m_szFile))
		{
			CFile cFile;
			if (cFile.Open(m_szFile,CFile::modeRead)) 
			{
				m_nFileSize = cFile.GetLength();
				m_nSize = m_nContentSize + m_nFileSize + m_nHeaderSize;;
				cFile.Close();
			}
			this->GenRefFileHeaderString(m_szFile);
		}
		else
		{
			m_nFileSize = 0;
			m_szFile += _T("    ------->  File doesn't exist!");
		}
	}
	UpdateData(FALSE);
}

void MultiPartDlg::OnBnClickedButtonDelete()
{
	UpdateData(TRUE);
	if (m_ctrlItemList.GetCurSel()==LB_ERR)
	{
		MessageBox(_T("Please select one item"));
		return;
	}
	int sel = m_ctrlItemList.GetCurSel();
	m_ctrlItemList.DeleteString(m_ctrlItemList.GetCurSel());
	m_vecMultiPartInfo.erase((m_vecMultiPartInfo.begin()+sel));

	RefreshLayout();
	m_ctrlItemList.SetCurSel(LB_ERR);
	UpdateData(FALSE);
}

BOOL MultiPartDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	for (int i =0;i<m_vecMultiPartInfo.size();++i)
	{
		CString szItem;
		CString szHeader = CA2W(m_vecMultiPartInfo[i].header.c_str());
		CString szContent = CA2W(m_vecMultiPartInfo[i].content.c_str());
		CString szFilePath(m_vecMultiPartInfo[i].filePath.c_str());
		int nSize = 0;
		if (PathFileExists(szFilePath)) 
		{
			CFile cFIle(szFilePath,CFile::modeRead);
			nSize += cFIle.GetLength();
		}
		nSize += strlen(m_vecMultiPartInfo[i].content.c_str());
		nSize += strlen(m_vecMultiPartInfo[i].header.c_str());
		szItem.Format(_T("%d. Header:[%s] Content:[%s] FilePath:[%s] TotalSize:[%d]"),i+1,szHeader,szContent,szFilePath,nSize);
		m_ctrlItemList.AddString(szItem);
	}
	return TRUE;
}


void MultiPartDlg::OnLbnSelchangeListItemlist()
{
	UpdateData(TRUE);
	// TODO: Add your control notification handler code here
	this->Clear();

	CString szItem;
	int nCurItem = m_ctrlItemList.GetCurSel();
	if (nCurItem == LB_ERR) return;
	m_szFile = CString(m_vecMultiPartInfo[nCurItem].filePath.c_str());
	m_szContent = CA2W(m_vecMultiPartInfo[nCurItem].content.c_str());
	m_szHeader = CA2W(m_vecMultiPartInfo[nCurItem].header.c_str());
	m_nHeaderSize = strlen(m_vecMultiPartInfo[nCurItem].header.c_str());
	m_nContentSize = strlen(m_vecMultiPartInfo[nCurItem].content.c_str());
	m_nSize = m_vecMultiPartInfo[nCurItem].dwFileSize + m_nContentSize + m_nHeaderSize;
	if (!m_vecMultiPartInfo[nCurItem].filePath.empty())
	{
		CFile cFile(m_vecMultiPartInfo[nCurItem].filePath.c_str(),CFile::modeRead);
		m_nFileSize = cFile.GetLength();
	}
	m_ctrlItemList.SetCurSel(nCurItem);
	UpdateData(FALSE);
}


void MultiPartDlg::OnBnClickedButtonSave()
{
	UpdateData(TRUE);
	int nCurIndex = m_ctrlItemList.GetCurSel();
	CString szTempItem;

	if (m_vecMultiPartInfo.size()==0 || nCurIndex==LB_ERR)
	{
		nCurIndex = m_vecMultiPartInfo.size();
		m_vecMultiPartInfo.push_back(MultiPartInfo());
	}

	//Fill the data model
	MultiPartInfo* cMulktiPartInfo = &m_vecMultiPartInfo.at(nCurIndex);
	cMulktiPartInfo->content = CT2CA(m_szContent);	
	cMulktiPartInfo->header = CT2CA(m_szHeader);

	cMulktiPartInfo->filePath = m_szFile;
	cMulktiPartInfo->dwFileSize = m_nFileSize;

	m_nSize = m_nFileSize + m_nContentSize + m_nHeaderSize;

	RefreshLayout();

	this->Clear();

	UpdateData(FALSE);
}


void MultiPartDlg::OnEnChangeEditContent()
{
	UpdateData(TRUE);
	// TODO:  Add your control notification handler code here
	CW2A szcContentSize(m_szContent);
	m_nContentSize = strlen(szcContentSize);

	m_nSize = m_nContentSize + m_nFileSize + m_nHeaderSize;
	UpdateData(FALSE);
}


void MultiPartDlg::Clear(void)
{
	m_szHeader = _T("");
	m_szContent = _T("");
	m_szFile = _T("");
	m_nSize = 0;
	m_nFileSize = 0;
	m_nContentSize = 0;
	m_nHeaderSize = 0;
	m_szHeaderRef = _T("");
}


void MultiPartDlg::OnEnChangeEditHeader()
{
	UpdateData(TRUE);
	// TODO:  Add your control notification handler code here
	CW2A szHeader(m_szHeader);
	m_nHeaderSize = strlen(szHeader);

	m_nSize = m_nContentSize + m_nFileSize + m_nHeaderSize;
	UpdateData(FALSE);
}


void MultiPartDlg::RefreshLayout(void)
{
	m_ctrlItemList.ResetContent();
	CString szTempItem;
	for (vector<MultiPartInfo>::iterator it = m_vecMultiPartInfo.begin();it!=m_vecMultiPartInfo.end();++it)
	{
		MultiPartInfo cMultiPartInfo = *it;
		CString szHeader(CA2W(cMultiPartInfo.header.c_str())) ;
		CString szContent(CA2W(cMultiPartInfo.content.c_str()));
		DWORD dwSize = cMultiPartInfo.header.length() + cMultiPartInfo.content.length();
		if (PathFileExists(cMultiPartInfo.filePath.c_str()))
		{
			CFile cFile(cMultiPartInfo.filePath.c_str(),CFile::modeRead);
			dwSize += cFile.GetLength();
			szTempItem.Format(_T("%d, Header:[%s] Contnet:[%s] FilePath:[%s] TotalSize(Head+Content+File):[%d]"),
											m_ctrlItemList.GetCount()+1,szHeader,
											szContent, cMultiPartInfo.filePath.c_str(),dwSize);
		}
		else
		{
			szTempItem.Format(_T("%d, Header:[%s] Contnet:[%s] TotalSize(Head+Content+File):[%d]"),
				m_ctrlItemList.GetCount()+1,szHeader,
				szContent, dwSize);
		}

		m_ctrlItemList.AddString(szTempItem);
	}
}


void MultiPartDlg::OnBnClickedButtonRemoveall()
{
	m_ctrlItemList.ResetContent();
	m_vecMultiPartInfo.clear();
}


void MultiPartDlg::OnBnClickedButton2()
{
	this->Clear();
	UpdateData(FALSE);
}


void MultiPartDlg::OnEnChangeEditFilepath()
{
	UpdateData(TRUE);
	GenRefFileHeaderString(m_szFile);
	UpdateData(FALSE);
}

void MultiPartDlg::GenRefFileHeaderString( CString szFile )
{
	CString szBuffer = szFile;
	TCHAR* lpszBuffer = szBuffer.GetBuffer(MAX_PATH);
	PathStripPath(lpszBuffer);

	m_szHeaderRef.Format(
		_T("¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ¡õ\r\n------------\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n\r\n\r\n¡ô¡ô¡ô¡ô¡ô¡ô¡ô¡ô¡ô¡ô"),
		lpszBuffer,lpszBuffer);

	szBuffer.ReleaseBuffer();
}
