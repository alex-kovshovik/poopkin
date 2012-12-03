// LoadFilesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "LoadFilesDialog.h"
#include ".\loadfilesdialog.h"

// CLoadFilesDialog dialog
#define WM_SET_PROGRESS WM_USER+1000

IMPLEMENT_DYNAMIC(CLoadFilesDialog, CDialog)
CLoadFilesDialog::CLoadFilesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadFilesDialog::IDD, pParent)
{
   bWantStop = false;
}

CLoadFilesDialog::~CLoadFilesDialog()
{
}

void CLoadFilesDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_COPY_PROGRESS, m_copy_progress);
   DDX_Control(pDX, IDC_FILENAME_EDIT, m_filename_edit);
   DDX_Control(pDX, IDC_SKIPPED_EDIT, m_errcount_edit);
}


BEGIN_MESSAGE_MAP(CLoadFilesDialog, CDialog)
   ON_MESSAGE(WM_SET_PROGRESS, OnUpdateProgressMessage)
   ON_MESSAGE(WM_CONFIRM_CONTINUE, OnConfirmContinue)
   ON_MESSAGE(WM_CLOSE_PROGRESS, OnCloseProgress)
   ON_MESSAGE(WM_SET_CAPTION, OnSetCaption)
   ON_MESSAGE(WM_SET_RANGE, OnSetRange)
   ON_MESSAGE(WM_SET_FILENAME, OnSetFilename)
   ON_MESSAGE(WM_SET_ERR_COUNT, OnSetErrCount)

   ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
   ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CLoadFilesDialog message handlers

LRESULT CLoadFilesDialog::OnUpdateProgressMessage(WPARAM wParam, LPARAM lParam)
{
   m_copy_progress.SetPos((int)wParam);
   return 0;
}

LRESULT CLoadFilesDialog::OnConfirmContinue(WPARAM wParam, LPARAM lParam)
{
   if (bWantStop)
      return 0;
   else
      return 1;
}
LRESULT CLoadFilesDialog::OnCloseProgress(WPARAM wParam, LPARAM lParam)
{
   // Закрываем диалог
   OnCancel();
   return 0;
}

LRESULT CLoadFilesDialog::OnSetCaption(WPARAM wParam, LPARAM lParam)
{
   if (!lParam) return 0;
   SetWindowText((LPCSTR)lParam);
   return 0;
}

LRESULT CLoadFilesDialog::OnSetRange(WPARAM wParam, LPARAM lParam)
{
   m_copy_progress.SetRange32((int)wParam, (int)lParam);
   return 0;
}

LRESULT CLoadFilesDialog::OnSetFilename(WPARAM wParam, LPARAM lParam)
{
   m_filename_edit.SetWindowText((LPCSTR)lParam);
   return 0;
}

LRESULT CLoadFilesDialog::OnSetErrCount(WPARAM wParam, LPARAM lParam)
{
   char buf[30];
   sprintf(buf, "%i", lParam);
   m_errcount_edit.SetWindowText(buf);
   return 0;
}
