// CopyAskDialog.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "CopyAskDialog.h"
#include ".\copyaskdialog.h"


// CCopyAskDialog dialog

IMPLEMENT_DYNAMIC(CCopyAskDialog, CDialog)
CCopyAskDialog::CCopyAskDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCopyAskDialog::IDD, pParent)
   , m_filename(_T(""))
{
   m_caption = "Ошибка копирования файла";
   m_dlg_kind = KND_COPY;   // Тип диалога по умолчанию
}

CCopyAskDialog::~CCopyAskDialog()
{
}

void CCopyAskDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_FILENAME_EDIT, m_filename_edit);
   DDX_Text(pDX, IDC_FILENAME_EDIT, m_filename);
   DDX_Control(pDX, IDC_RETRY_BUTTON, m_retry_button);
}


BEGIN_MESSAGE_MAP(CCopyAskDialog, CDialog)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_BN_CLICKED(IDC_SKIPALL_BUTTON, OnBnClickedSkipallButton)
   ON_BN_CLICKED(IDC_RETRY_BUTTON, OnBnClickedRetryButton)
END_MESSAGE_MAP()


// CCopyAskDialog message handlers

void CCopyAskDialog::OnBnClickedOk()
{
   // TODO: Add your control notification handler code here
   m_button = BN_SKIP;
   OnOK();
}

void CCopyAskDialog::OnBnClickedSkipallButton()
{
   // TODO: Add your control notification handler code here
   m_button = BN_SKIPALL;
   OnOK();
}

void CCopyAskDialog::OnBnClickedRetryButton()
{
   // TODO: Add your control notification handler code here
   m_button = BN_RETRY;
   OnOK();
}

BOOL CCopyAskDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   // TODO:  Add extra initialization here
   SetWindowText(m_caption);
   
   // Кнопка "Повторить" не доступна в режиме быстрого копирования
   if (m_dlg_kind == KND_FAST_COPY)
      m_retry_button.EnableWindow(0);

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}
