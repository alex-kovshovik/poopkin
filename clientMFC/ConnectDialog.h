#pragma once
#include "afxwin.h"

#define WM_NOTIFY_CONNECTED   WM_USER+2000    // ���������� ����������, ����� ��������� ������
#define WM_ERROR_CONNECTING   WM_USER+2001    // ������ �������� ����������

// CConnectDialog dialog
class CConnectDialog : public CDialog
{
	DECLARE_DYNAMIC(CConnectDialog)

   int MakeConnection();   // ������� �������� ����������

public:
	CConnectDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConnectDialog();

// Dialog Data
	enum { IDD = IDD_CONNECT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CString m_ip;
   CString m_port;
   bool b_auto_connect;       // ������� ��������������� �����������

   PUSHKIN_CLIENT::CPushkinClient *m_pClient;   // ��������� �� �������
   virtual BOOL OnInitDialog();
   afx_msg void OnBnClickedOk();
   CButton m_connect_button;
   CEdit m_ip_edit;
   CEdit m_port_edit;

   // ����������� ������� ���������
   LRESULT OnNotifyConnected(WPARAM wParam, LPARAM lParam);
   LRESULT OnErrorConnecting(WPARAM wParam, LPARAM lParam);
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
