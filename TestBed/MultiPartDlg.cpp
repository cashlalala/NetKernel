// MultiPartDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestBed.h"
#include "MultiPartDlg.h"
#include "Shlwapi.h"
//#include "afxdialogex.h"


// MultiPartDlg dialog

IMPLEMENT_DYNAMIC(MultiPartDlg, CDialog)

MultiPartDlg::MultiPartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(MultiPartDlg::IDD, pParent)
	, m_szContent(_T(""))
	, m_szHeader(_T(""))
	, m_szFile(_T(""))
	, m_nSize(0)
	, m_szItemStrings(_T(""))
{

}

MultiPartDlg::MultiPartDlg( std::vector<MultiPartInfo> vecItemList )
	: CDialog(MultiPartDlg::IDD, NULL)
	, m_szContent(_T(""))
	, m_szHeader(_T(""))
	, m_szFile(_T(""))
	, m_nSize(0)
	, m_szItemStrings(_T(""))
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
}


BEGIN_MESSAGE_MAP(MultiPartDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OPENFILE, &MultiPartDlg::OnBnClickedButtonOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &MultiPartDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &MultiPartDlg::OnBnClickedButtonDelete)
	ON_LBN_SELCHANGE(IDC_LIST_ITEMLIST, &MultiPartDlg::OnLbnSelchangeListItemlist)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &MultiPartDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// MultiPartDlg message handlers


void MultiPartDlg::OnBnClickedButtonOpenfile()
{
	UpdateData(TRUE);
	CString szFilter = _T("*.*|*.*||");
	CFileDialog dlgFile(TRUE,_T("*"),_T("*.*"),OFN_HIDEREADONLY,szFilter,m_pParentWnd);
	if (dlgFile.DoModal()==IDOK)
	{
		//m_szFile +=dlgFile.GetFolderPath();
		m_szFile += dlgFile.GetPathName();
	}
	UpdateData(FALSE);
}

void MultiPartDlg::OnBnClickedButtonAdd()
{


	UpdateData(TRUE);
	CString szTempItem;
	szTempItem.Format(_T("%d:"),m_ctrlItemList.GetCount()+1);
	m_ctrlItemList.AddString(szTempItem);
	m_ctrlItemList.SetCurSel(m_ctrlItemList.GetCount()-1);

	m_szContent = _T("");
	m_szHeader = _T("");
	m_szFile = _T("");
	UpdateData(FALSE);
}


void MultiPartDlg::OnBnClickedButtonDelete()
{
	UpdateData(TRUE);
	m_ctrlItemList.DeleteString(m_ctrlItemList.GetCurSel());
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
		szItem.Format(_T("%d. Header:[%s] Content:[%s] FilePath:[%s]"),i+1,szHeader,szContent,szFilePath);
		m_ctrlItemList.AddString(szItem);
	}
	return TRUE;
}


void MultiPartDlg::OnLbnSelchangeListItemlist()
{
	// TODO: Add your control notification handler code here
	CString szItem;
	int nCurItem = m_ctrlItemList.GetCurSel();
	if (nCurItem == LB_ERR) return;
	m_szFile = CString(m_vecMultiPartInfo[nCurItem].filePath.c_str());
	m_szContent = CA2W(m_vecMultiPartInfo[nCurItem].content.c_str());
	m_szHeader = CA2W(m_vecMultiPartInfo[nCurItem].header.c_str());
	
	UpdateData(FALSE);
}


void MultiPartDlg::OnBnClickedButtonSave()
{
	UpdateData(TRUE);

	CString szTempItem;
	int nCurIndex = m_ctrlItemList.GetCurSel();
	if (nCurIndex==LB_ERR) nCurIndex = m_ctrlItemList.GetCount()-1;
	m_ctrlItemList.DeleteString(nCurIndex);

	if (PathFileExists(m_szFile))
		szTempItem.Format(_T("%d: %s %s %s"),m_ctrlItemList.GetCount()+1,m_szHeader,m_szContent, m_szFile);
	else
		szTempItem.Format(_T("%d: %s %s"),m_ctrlItemList.GetCount()+1,m_szHeader,m_szContent);
	m_ctrlItemList.AddString(szTempItem);


	MultiPartInfo cMulktiPartInfo;
	cMulktiPartInfo.content = CT2CA(m_szContent);	
	cMulktiPartInfo.header = CT2CA(m_szHeader);

	cMulktiPartInfo.filePath = m_szFile;
	if (PathFileExists(m_szFile))
	{
		CFile cFile;
		if (cFile.Open(m_szFile,CFile::modeRead)) 
		{
			cMulktiPartInfo.dwFileSize = cFile.GetLength();
			cFile.Close();
		}
	}
	else
		m_szFile += _T("    ------->  File doesn't exist!");

	m_vecMultiPartInfo.push_back(cMulktiPartInfo);

	m_ctrlItemList.SetCurSel(m_ctrlItemList.GetCount()-1);

	UpdateData(FALSE);
}
