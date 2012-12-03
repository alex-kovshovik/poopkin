// ShellExecuteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "ShellExecuteDialog.h"
#include ".\shellexecutedialog.h"


// CShellExecuteDialog dialog

IMPLEMENT_DYNAMIC(CShellExecuteDialog, CDialog)
CShellExecuteDialog::CShellExecuteDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CShellExecuteDialog::IDD, pParent)
   , m_operation_s(_T(""))
   , m_filename(_T(""))
   , m_filedir(_T(""))
{
}

CShellExecuteDialog::~CShellExecuteDialog()
{
}

void CShellExecuteDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_SE_OPER_COMBO, m_operation);
   DDX_CBString(pDX, IDC_SE_OPER_COMBO, m_operation_s);
   DDX_Text(pDX, IDC_SE_FILENAME_EDIT, m_filename);
   DDX_Text(pDX, IDC_SE_FILEDIR_EDIT, m_filedir);
}


BEGIN_MESSAGE_MAP(CShellExecuteDialog, CDialog)
END_MESSAGE_MAP()


// CShellExecuteDialog message handlers

BOOL CShellExecuteDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   // TODO:  Add extra initialization here
   m_operation.AddString("open");
   m_operation.SetCurSel(0);

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}
