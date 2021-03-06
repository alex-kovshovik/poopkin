// MyDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "FileList.h"
#include "common.h"
#include "afxwin.h"
#include "editex.hpp"
#include "params.hpp"
#include "ConnectDialog.h"

// CMyDlg dialog
class CMyDlg : public CDialog
{
// Construction
public:
	CMyDlg(CWnd* pParent = NULL);	// standard constructor
   ~CMyDlg(); // ����������

// Dialog Data
	enum { IDD = IDD_CLIENTMFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
   WSADATA      m_wsa_data;       // ���������� � �������


// Implementation
protected:
	HICON m_hIcon;

   PUSHKIN_CLIENT::CPushkinClient m_client;
   vector<String>    m_folder_stack;   // ��� �������� �� ���������
   CFileList         m_filelist_sort;  // ��������� � ��������� ������ ������

   int m_sort_mode;
   SParams params;                     // ��������� ���������

   // ��� ������� ������ � ��������
   int LoadFileList(const char *path); // �������� ������ ������ � ListBox
   int ReloadFileList();               // ������������ ������ ������ - ������� � �����������
   int CalcFreeSpace();                // ������ ���������� ����� �� �������

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
   CListCtrl m_file_list;
   CMenu     m_listpopup_menu;
   CMenu     m_main_menu;         // ������� ���� ���������

   CConnectDialog m_connect_dlg;  // ������ �������� �����������
   
   afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
   afx_msg void OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult);
   CString m_path_edit;
   CString m_totalfiles;
   afx_msg void OnBnClickedOk();
   
   afx_msg void OnHdnItemclickFileList(NMHDR *pNMHDR, LRESULT *pResult);
   CComboBox m_drives_combo;
   afx_msg void OnCbnSelchangeDrivesCombo();
   afx_msg void OnNMRclickFileList(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnPopupLoad();
   afx_msg void OnPopupDelete();
   afx_msg void OnSendMessage();
   CEditEx m_totalfiles_edit;
   afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
   afx_msg void CalcFolderSize();
   afx_msg void OnMmShellExecute();
   afx_msg void OnSendFiles();
   afx_msg void ExecuteCurrentFile();
   afx_msg void ExecuteProgram();
   CEdit m_free_space_edit;
   afx_msg void OnBnClickedButton1();
};
