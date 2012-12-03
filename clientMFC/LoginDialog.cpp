// LoginDialog.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "LoginDialog.h"


// CLoginDialog dialog

IMPLEMENT_DYNAMIC(CLoginDialog, CDialog)
CLoginDialog::CLoginDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoginDialog::IDD, pParent)
   , m_login(_T(""))
   , m_pwd(_T(""))
{
}

CLoginDialog::~CLoginDialog()
{
}

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_LOGIN_EDIT, m_login);
   DDX_Text(pDX, IDC_PWD_EDIT, m_pwd);
}


BEGIN_MESSAGE_MAP(CLoginDialog, CDialog)
END_MESSAGE_MAP()


// CLoginDialog message handlers
