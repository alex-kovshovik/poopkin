// SendMessageDialog.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "SendMessageDialog.h"


// CSendMessageDialog dialog

IMPLEMENT_DYNAMIC(CSendMessageDialog, CDialog)
CSendMessageDialog::CSendMessageDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSendMessageDialog::IDD, pParent)
   , m_caption(_T(""))
   , m_message(_T(""))
{
}

CSendMessageDialog::~CSendMessageDialog()
{
}

void CSendMessageDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_EDIT2, m_caption);
   DDX_Text(pDX, IDC_MESSAGE_EDIT, m_message);
}


BEGIN_MESSAGE_MAP(CSendMessageDialog, CDialog)
END_MESSAGE_MAP()


// CSendMessageDialog message handlers
