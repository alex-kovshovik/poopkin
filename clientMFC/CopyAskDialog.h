#pragma once
#include "editex.hpp"
#include "afxwin.h"


// CCopyAskDialog dialog

class CCopyAskDialog : public CDialog
{
	DECLARE_DYNAMIC(CCopyAskDialog)

public:
	CCopyAskDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCopyAskDialog();

// Dialog Data
	enum { IDD = IDD_COPYASK_DIALOG };
   enum { BN_SKIP, BN_SKIPALL, BN_RETRY} m_button;
   
   // Разные способы работы диалога
   enum { KND_COPY, KND_FAST_COPY} m_dlg_kind;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CEditEx m_filename_edit;
   afx_msg void OnBnClickedOk();
   afx_msg void OnBnClickedSkipallButton();
   afx_msg void OnBnClickedRetryButton();
   CString m_filename;
   CString m_caption;
   virtual BOOL OnInitDialog();
   CButton m_retry_button;
};
