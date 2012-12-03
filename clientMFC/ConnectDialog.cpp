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

// Структура для хранения 
struct SAutoConnect
{
   char            ip[20];
   int             port;
   CPushkinClient *client_p;
   HWND            hWnd;      // Дескриптор окна соединения (для передачи ему сообщений)
};

// Поток, в котором будет создаваться соединение с сервером
UINT ConnectThread(LPVOID pParam)
{
   // В переменной pParam будет передаваться указатель на структуру SAutoConnect
   // После использования, память по этому указателю надо освободить
   SAutoConnect *s = (SAutoConnect*)pParam;

   if (s->client_p->Connect(s->ip, s->port) != P_YES)
      SendMessage(s->hWnd, WM_ERROR_CONNECTING, 0, 0);
   else
      SendMessage(s->hWnd, WM_NOTIFY_CONNECTED, 0, 0);

   delete s;   // Обязательно нужно освободить память после использования этой переменной

   return 0;
}

// CConnectDialog message handlers
BOOL CConnectDialog::OnInitDialog()
{
   CDialog::OnInitDialog();

   // TODO:  Add extra initialization here
   if (!m_pClient)
   {
      MessageBox("Нулевой указатель на клиента!", "Error", MB_OK|MB_ICONSTOP);
      return FALSE;
   }

   if (b_auto_connect)
      SetWindowText("Соединение с сервером...");

   return TRUE;  // return TRUE unless you set the focus to a control
   // EXCEPTION: OCX Property Pages should return FALSE
}

// При нажатии на кнопку Connect
void CConnectDialog::OnBnClickedOk()
{
   // На всякий случай не даем нажать эту кнопку для случая автоматического коннекта
   if (b_auto_connect) return;

   SetWindowText("Соединение с сервером");
   CDataExchange dx(this, TRUE);
   this->DoDataExchange(&dx);
   MakeConnection();
}

// Сообщение об удачном соединении
LRESULT CConnectDialog::OnNotifyConnected(WPARAM wParam, LPARAM lParam)
{
   OnOK();  // Закрываем диалог и работаем с клиентом
   return 0;
}

LRESULT CConnectDialog::OnErrorConnecting(WPARAM wParam, LPARAM lParam)
{
   MessageBox("Ошибка соединения с сервером", "Error", MB_OK|MB_ICONSTOP);
   OnCancel();
   return 0;
}

// Функция создания соединения
int CConnectDialog::MakeConnection()
{
   // Перед началом потока отменяем кнопку "Connect", закрываем для редактирования
   // поля ip и port
   // и меняем заголовок окна
   m_connect_button.EnableWindow(0);
   m_ip_edit.SetReadOnly();
   m_port_edit.SetReadOnly();

   // Создаем поток, в котором будет создаваться подключение
   SAutoConnect *s = new SAutoConnect;
   if (!s) return FALSE;   // Не хватило памяти! - этого не может быть

   s->client_p = m_pClient;
   strcpy(s->ip, m_ip);
   s->port     = atoi(m_port);
   s->hWnd     = m_hWnd;

   if (!AfxBeginThread(ConnectThread, (LPVOID)s))
      return FALSE;  // Ошибка создания поток

   return TRUE;
}

// При показе окна
void CConnectDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CDialog::OnShowWindow(bShow, nStatus);

   // Тут же будем создавать процесс создания подключения, если оно автоматическое
   if (b_auto_connect)
      MakeConnection();
}
