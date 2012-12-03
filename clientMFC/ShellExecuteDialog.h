#pragma once
#include "afxwin.h"


// CShellExecuteDialog dialog

class CShellExecuteDialog : public CDialog
{
	DECLARE_DYNAMIC(CShellExecuteDialog)

public:
	CShellExecuteDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShellExecuteDialog();

// Dialog Data
	enum { IDD = IDD_SHELLEXECUTE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CComboBox m_operation;
   CString m_operation_s;
   CString m_filename;
   CString m_filedir;
   virtual BOOL OnInitDialog();
};
