#pragma once


// CSendMessageDialog dialog

class CSendMessageDialog : public CDialog
{
	DECLARE_DYNAMIC(CSendMessageDialog)

public:
	CSendMessageDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSendMessageDialog();

// Dialog Data
	enum { IDD = IDD_SEND_MESSAGE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_caption;
   CString m_message;
};
