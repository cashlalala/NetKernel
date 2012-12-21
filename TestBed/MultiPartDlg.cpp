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
	, m_nContentSize(0)
	, m_nFileSize(0)
	, m_bIsFromSsave(false)
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
}


BEGIN_MESSAGE_MAP(MultiPartDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OPENFILE, &MultiPartDlg::OnBnClickedButtonOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &MultiPartDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &MultiPartDlg::OnBnClickedButtonDelete)
	ON_LBN_SELCHANGE(IDC_LIST_ITEMLIST, &MultiPartDlg::OnLbnSelchangeListItemlist)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &MultiPartDlg::OnBnClickedButtonSave)
	ON_EN_CHANGE(IDC_EDIT_CONTENT, &MultiPartDlg::OnEnChangeEditContent)
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
				m_nSize = m_nContentSize + m_nFileSize;
				cFile.Close();
			}
		}
		else
		{
			m_nFileSize = 0;
			m_szFile += _T("    ------->  File doesn't exist!");
		}
	}
	UpdateData(FALSE);
}

void MultiPartDlg::OnBnClickedButtonAdd()
{
	UpdateData(TRUE);
	CString szTempItem;
	MultiPartInfo cMultiPartInfo;	
	if (m_bIsFromSsave)
	{
		//szTempItem.Format(_T("%d. Header:[%s] Content:[%s] Filepath:[%s]"),m_ctrlItemList.GetCount()+1,m_szHeader,m_szContent, m_szFile);
		szTempItem.Format(_T("%d, Header:[%s] Contnet:[%s] FilePath:[%s] File+ContentSize:[%d]"),m_ctrlItemList.GetCount()+1,m_szHeader,m_szContent, m_szFile,m_nSize);
		
		cMultiPartInfo.content = CW2A(m_szContent);
		cMultiPartInfo.dwFileSize = m_nSize;
		cMultiPartInfo.filePath = m_szFile;
		cMultiPartInfo.header = CW2A(m_szHeader);		
	}
	else 
	{	
		szTempItem.Format(_T("%d,"),m_ctrlItemList.GetCount()+1);
	}
	m_vecMultiPartInfo.push_back(cMultiPartInfo);
	m_ctrlItemList.AddString(szTempItem);
	m_bIsFromSsave = false;
	m_ctrlItemList.SetCurSel(LB_ERR);
	this->Clear();

	UpdateData(FALSE);
}


void MultiPartDlg::OnBnClickedButtonDelete()
{
	UpdateData(TRUE);
	m_ctrlItemList.DeleteString(m_ctrlItemList.GetCurSel());
	m_vecMultiPartInfo.erase(m_vecMultiPartInfo.begin()+m_ctrlItemList.GetCurSel());
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
	this->Clear();

	CString szItem;
	int nCurItem = m_ctrlItemList.GetCurSel();
	if (nCurItem == LB_ERR) return;
	m_szFile = CString(m_vecMultiPartInfo[nCurItem].filePath.c_str());
	m_szContent = CA2W(m_vecMultiPartInfo[nCurItem].content.c_str());
	m_szHeader = CA2W(m_vecMultiPartInfo[nCurItem].header.c_str());
	m_nSize = m_vecMultiPartInfo[nCurItem].dwFileSize;
	m_nContentSize = strlen(m_vecMultiPartInfo[nCurItem].content.c_str());
	if (!m_vecMultiPartInfo[nCurItem].filePath.empty())
	{
		CFile cFile(m_vecMultiPartInfo[nCurItem].filePath.c_str(),CFile::modeRead);
		m_nFileSize = cFile.GetLength();
	}
	UpdateData(FALSE);
}


void MultiPartDlg::OnBnClickedButtonSave()
{
	UpdateData(TRUE);
	int nCurIndex = m_ctrlItemList.GetCurSel();
	CString szTempItem;

	if (m_ctrlItemList.GetCount()==0 || nCurIndex==LB_ERR)
	{
		m_bIsFromSsave = true;
		this->OnBnClickedButtonAdd();
		return;
	}

	m_ctrlItemList.DeleteString(nCurIndex);
	m_vecMultiPartInfo.erase(m_vecMultiPartInfo.begin()+nCurIndex);

	if (PathFileExists(m_szFile))
		szTempItem.Format(_T("%d, Header:[%s] Contnet:[%s] FilePath:[%s] File+ContentSize:[%d]"),m_ctrlItemList.GetCount()+1,m_szHeader,m_szContent, m_szFile,m_nSize);
	else
		szTempItem.Format(_T("%d, Header:[%s] Contnet:[%s]"),m_ctrlItemList.GetCount()+1,m_szHeader,m_szContent);
	m_ctrlItemList.AddString(szTempItem);

	//Fill the data model
	MultiPartInfo cMulktiPartInfo;
	cMulktiPartInfo.content = CT2CA(m_szContent);	
	cMulktiPartInfo.header = CT2CA(m_szHeader);

	cMulktiPartInfo.filePath = m_szFile;
	cMulktiPartInfo.dwFileSize = m_nFileSize;

	m_nSize = m_nFileSize + m_nContentSize;
	m_vecMultiPartInfo.insert(m_vecMultiPartInfo.begin()+nCurIndex,cMulktiPartInfo);

	//set the selected one on UI
	m_ctrlItemList.SetCurSel(LB_ERR);

	this->Clear();

	UpdateData(FALSE);
}


void MultiPartDlg::OnEnChangeEditContent()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	UpdateData(TRUE);
	// TODO:  Add your control notification handler code here
	CW2A szcContentSize(m_szContent);
	m_nContentSize = strlen(szcContentSize);

	m_nSize = m_nContentSize + m_nFileSize;
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
}
