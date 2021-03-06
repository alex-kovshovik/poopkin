// ConnectDialog.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "client.h"
#include "ConnectDialog.h"
#include "common.h"

using namespace PUSHKIN_CLIENT;
// CConnectDialog dialog

IMPLEMENT_DYNAMIC(CConnectDialog, CDialog)
CConnectDialog::CConnectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CConnectDialog::IDD, pParent)
   , m_ip(_T(""))
   , m_port(_T(""))
{
   m_pClient = NULL;
   b_auto_connect = false;
}

CConnectDialog::~CConnectDialog()
{
}

void CConnectDialog::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Text(pDX, IDC_EDIT1, m_ip);
   DDX_Text(pDX, IDC_EDIT2, m_port);
   DDX_Control(pDX, IDOK, m_connect_button);
   DDX_Control(pDX, IDC_EDIT1, m_ip_edit);
   DDX_Control(pDX, IDC_EDIT2, m_port_edit);
}


BEGIN_MESSAGE_MAP(CConnectDialog, CDialog)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_MESSAGE(WM_NOTIFY_CONNECTED, OnNotifyConnected)
   ON_MESSAGE(WM_ERROR_CONNECTING, OnErrorConnecting)
   ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// ��������� ��� �������� 
struct SAutoConnect
{
   char            ip[20];
   int             port;
   CPushkinClient *client_p;
   HWND            hWnd;      // ���������� ���� ���������� (��� �������� ��� ���������)
};

// �����, � ������� ����� ����������� ���������� � ��������
UINT ConnectThread(LPVOID pParam)
{
   // � ���������� pParam ����� ������������ ��������� �� ��������� SAutoConnect
   // ����� �������������, ������ �� ����� ��������� ���� ����������
   SAutoConnect *s = (SAutoConnect*)pParam;

   if (s->client_p->Connect(s->ip, s->port) != P_YES)
      SendMessage(s->hWnd, WM_ERROR_CONNECTING, 0, 0);
   else
      SendMessage(s->hWnd, WM_NOTIFY_CONNECTED, 0, 0);

   delete s;   // ����������� ����� ���������� ������ ����� ������������� ���� ����������

   return 0;
}

// CConnectDialog message handlers
BOOL CConnectDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   // TODO:  Add extra initialization here
   if (!m_pClient)
   {
      MessageBox("������� ��������� �� �������!", "Error", MB_OK|MB_ICONSTOP);
      return FALSE;
   }

   if (b_auto_connect)
      SetWindowText("���������� � ��������...");

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

// ��� ������� �� ������ Connect
void CConnectDialog::OnBnClickedOk()
{
   // �� ������ ������ �� ���� ������ ��� ������ ��� ������ ��������������� ��������
   if (b_auto_connect) return;

   SetWindowText("���������� � ��������");
   CDataExchange dx(this, TRUE);
   this->DoDataExchange(&dx);
   MakeConnection();
}

// ��������� �� ������� ����������
LRESULT CConnectDialog::OnNotifyConnected(WPARAM wParam, LPARAM lParam)
{
   OnOK();  // ��������� ������ � �������� � ��������
   return 0;
}

LRESULT CConnectDialog::OnErrorConnecting(WPARAM wParam, LPARAM lParam)
{
   MessageBox("������ ���������� � ��������", "Error", MB_OK|MB_ICONSTOP);
   OnCancel();
   return 0;
}

// ������� �������� ����������
int CConnectDialog::MakeConnection()
{
   // ����� ������� ������ �������� ������ "Connect", ��������� ��� ��������������
   // ���� ip � port
   // � ������ ��������� ����
   m_connect_button.EnableWindow(0);
   m_ip_edit.SetReadOnly();
   m_port_edit.SetReadOnly();

   // ������� �����, � ������� ����� ����������� �����������
   SAutoConnect *s = new SAutoConnect;
   if (!s) return FALSE;   // �� ������� ������! - ����� �� ����� ����

   s->client_p = m_pClient;
   strcpy(s->ip, m_ip);
   s->port     = atoi(m_port);
   s->hWnd     = m_hWnd;

   if (!AfxBeginThread(ConnectThread, (LPVOID)s))
      return FALSE;  // ������ �������� �����

   return TRUE;
}

// ��� ������ ����
void CConnectDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CDialog::OnShowWindow(bShow, nStatus);

   // ��� �� ����� ��������� ������� �������� �����������, ���� ��� ��������������
   if (b_auto_connect)
      MakeConnection();
}
