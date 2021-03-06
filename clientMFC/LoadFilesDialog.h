#pragma once

#include "afxcmn.h"
#include "afxwin.h"
#include "resource.h"
#include "editex.hpp"

// CLoadFilesDialog dialog
#define WM_SET_PROGRESS       WM_USER+1000    // ��������� �������� �������� �������� �����������
#define WM_CONFIRM_CONTINUE   WM_USER+1001    // ����� �� ���������� ����������� ������
#define WM_CLOSE_PROGRESS     WM_USER+1002    // �������� ������� ����� ��������� �����������
#define WM_SET_CAPTION        WM_USER+1003    // ��������� ��������� ���� (LPARAM)
#define WM_SET_RANGE          WM_USER+1004    // ��������� �������� ��������
#define WM_SET_FILENAME       WM_USER+1005    // ��������� ����� ����� � �������� ��������������
#define WM_SET_ERR_COUNT      WM_USER+1006    // ��������� ���������� ������������ ������

class CLoadFilesDialog : public CDialog
{
	DECLARE_DYNAMIC(CLoadFilesDialog)
   bool bWantStop;

public:
	CLoadFilesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoadFilesDialog();

// Dialog Data
	enum { IDD = IDD_LOADFILES_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CProgressCtrl m_copy_progress;
   CEditEx       m_filename_edit;

   // ��������� �� ������������ ������
   LRESULT OnUpdateProgressMessage(WPARAM wParam, LPARAM lParam);
   LRESULT OnConfirmContinue(WPARAM wParam, LPARAM lParam);
   LRESULT OnCloseProgress(WPARAM wParam, LPARAM lParam);
   LRESULT OnSetCaption(WPARAM wParam, LPARAM lParam);
   LRESULT OnSetRange(WPARAM wParam, LPARAM lParam);
   LRESULT OnSetFilename(WPARAM wParam, LPARAM lParam);
   LRESULT OnSetErrCount(WPARAM wParam, LPARAM lParam);

   afx_msg void OnBnClickedCancel();

   LPVOID m_pParam;  // ��������� �� ���������, ������� ���� �������� � ����� �����������
   virtual BOOL OnInitDialog();
   CEditEx m_errcount_edit;
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
