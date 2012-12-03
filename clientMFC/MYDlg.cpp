// MYDlg.cpp : implementation file
//

#include "stdafx.h"
#include "clientMFC.h"
#include "MyDlg.h"
#include "LoginDialog.h"
#include "pint.hpp"
#include "commands.h"
#include "mydlg.h"
#include "load_files.hpp"
#include "ShellExecuteDialog.h"
#include "SendMessageDialog.h"
#include "pdouble.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMyDlg dialog

CMyDlg::CMyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMyDlg::IDD, pParent)
   , m_path_edit(_T(""))
   , m_totalfiles(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

   m_sort_mode = 0;
}

CMyDlg::~CMyDlg()
{
   WSACleanup();
}

void CMyDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_FILE_LIST, m_file_list);
   DDX_Text(pDX, IDC_PATH_EDIT, m_path_edit);
   DDX_Text(pDX, IDC_TOTALFILES_EDIT, m_totalfiles);
   DDX_Control(pDX, IDC_DRIVES_COMBO, m_drives_combo);
   DDX_Control(pDX, IDC_TOTALFILES_EDIT, m_totalfiles_edit);
   DDX_Control(pDX, IDC_FREE_SPACE, m_free_space_edit);
}

BEGIN_MESSAGE_MAP(CMyDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_WM_CREATE()
   ON_NOTIFY(NM_DBLCLK, IDC_FILE_LIST, OnNMDblclkFileList)
   ON_BN_CLICKED(IDOK, OnBnClickedOk)
   ON_NOTIFY(HDN_ITEMCLICK, 0, OnHdnItemclickFileList)
   ON_CBN_SELCHANGE(IDC_DRIVES_COMBO, OnCbnSelchangeDrivesCombo)
   ON_NOTIFY(NM_RCLICK, IDC_FILE_LIST, OnNMRclickFileList)
   ON_COMMAND(ID_POPUP_LOAD, OnPopupLoad)
   ON_COMMAND(ID_POPUP_DELETE, OnPopupDelete)
   ON_COMMAND(ID_SEND_MESSAGE, OnSendMessage)
   ON_WM_SHOWWINDOW()
   ON_COMMAND(ID__CALC_SIZE, CalcFolderSize)
   ON_COMMAND(ID_MM_SHELL_EXECUTE, OnMmShellExecute)
   ON_COMMAND(ID_32782, OnSendFiles)
   ON_COMMAND(ID__32783, ExecuteCurrentFile)
   ON_COMMAND(ID__32784, ExecuteProgram)
   ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()

// CMyDlg message handlers

BOOL CMyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

   // Загружаем выпадающее меню
   m_listpopup_menu.LoadMenu(IDR_LISTPOPUP_MENU);

	// TODO: Add extra initialization here
   m_file_list.InsertColumn(0, "Имя файла/каталога", LVCFMT_LEFT, 270);
   m_file_list.InsertColumn(1, "Размер", LVCFMT_RIGHT, 100);

   m_file_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);

   m_totalfiles_edit.bkColor(RGB(220,220,220));
   m_totalfiles_edit.textColor(RGB(50,0,0));


   return TRUE;  // return TRUE  unless you set the focus to a control
}

// При показе главного окна программы
void CMyDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
   CDialog::OnShowWindow(bShow, nStatus);

   // Показываем список логических дисков сервера
   UINT drives;
   if (m_client.GetDrivesList(drives) != P_YES)
      AfxMessageBox(m_client.GetErrorMessage());

   char c = 'A';
   UINT t = 1;
   char buf[2];
   buf[1] = 0;
   for (int i=0; i<26; i++)
   {
      if ((drives & t) && c!='A')
      {
         buf[0] = c;
         m_drives_combo.AddString(buf);
      }

      t <<= 1;
      c++;
   }

   m_drives_combo.SetCurSel(0);

   CString cur_drive;
   m_drives_combo.GetWindowText(cur_drive);
   cur_drive += ":";

   m_filelist_sort.Init(&m_client);

   LoadFileList(cur_drive);
   m_folder_stack.push_back((String)cur_drive);

   CDataExchange pdx(this, FALSE);
   m_path_edit = (String)cur_drive + "\\";
   DoDataExchange(&pdx);

   CalcFreeSpace();
}

void CMyDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CMyDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if (CDialog::OnCreate(lpCreateStruct) == -1)
      return -1;

   // Инициализация сокетов - вынесена за пределы инициализации клиента
   memset(&m_wsa_data, 0, sizeof(m_wsa_data));  // Данные интерфейса сокетов
   if (WSAStartup(0x101,&m_wsa_data) != 0)
      return -1;

   params.Reset();

   // Загружаем параметры из INI файла
   LoadParams(params);

   // Показываем диалог соединения
   char buf[128];
   sprintf(buf, "%i", params.port);
   m_connect_dlg.m_ip   = params.ip;
   m_connect_dlg.m_port = buf;

   // Соединение будет создаваться в отдельном потоке, созданном из диалога CConnectDialog (m_connect_dlg)
   m_connect_dlg.m_pClient = &m_client;
   m_connect_dlg.b_auto_connect = params.autoconnect?true:false;

   if (m_connect_dlg.DoModal() != IDOK)
      return -1;  // Отказ приконнектиться или ошибка соединения

   // Показываем IP и порт машины, к которой приконнектились
   SetWindowText((String)"Офигенный клиент. IP:"+(const char*)m_connect_dlg.m_ip+", port:"+(const char*)m_connect_dlg.m_port);

   CLoginDialog login;
   login.m_login = params.login;
   login.m_pwd = params.pwd;
   
   // Цикл попыток авторизоваться на сервере
   if (!params.autologin)
      while (1)
      {
         if (login.DoModal() == IDOK)
         {
            // Попытка залогиниться
            if (m_client.Login(login.m_login, login.m_pwd) != P_YES)
               AfxMessageBox(m_client.GetErrorMessage());
            else
               break;
         }
         else
            return -1;
      }
   else

   // Пока логин автоматический - уже нет
   if (m_client.Login(params.login, params.pwd) != P_YES)
   {
      AfxMessageBox(m_client.GetErrorMessage());
      return -1;
   }

   // Загрузка меню
   if (!m_main_menu.LoadMenu(IDR_MAIN_MENU))
   {
      AfxMessageBox("Ошибка загрузки главного меню");
      return -1;
   }

   this->SetMenu(&m_main_menu);
   
   return 0;
}

// Загрузка списка файлов в ListBox
int CMyDlg::LoadFileList(const char *path)
{
   if (!path) return P_ERROR;
   vector<SFileInfo> temp_list;

   BeginWaitCursor();

   if (m_filelist_sort.Load(path) != P_YES)
   {
      AfxMessageBox(m_client.GetErrorMessage());
      return P_ERROR;
   }

   // Удаляем старое содержимое списка
   int nCount = m_file_list.GetItemCount();
   for (int i=0; i<nCount; i++)
      m_file_list.DeleteItem(0);

   // Выводим путь в заголовке и общее количество файлов и папок
   nCount = m_filelist_sort.GetCount();
   CDataExchange pdx(this, FALSE);
   m_path_edit = path;
   m_totalfiles = (Int)(nCount-2);
   DoDataExchange(&pdx);

   // Сортируем список
   if (m_filelist_sort.Sort(m_sort_mode) != P_YES)
   {
      AfxMessageBox("Ошибка при сортировке списка");
      return P_ERROR;
   }

   SFileInfo **fi = m_filelist_sort.fi;

   // Выводим список
   for (int i=0; i<nCount; i++)
   {
      if (fi[i]->name == ".") continue;  // Такие пути пропускаем мимо ушей

      int index = m_file_list.InsertItem(i, fi[i]->name.c_str(), 0);
      m_file_list.SetItemData(index, i);

      if (fi[i]->attrs & FILE_ATTRIBUTE_DIRECTORY)
         m_file_list.SetItemText(index, 1, "<DIR>");
      else
      {
         __int64 size = fi[i]->size_high;
         size <<= 32;
         size |= fi[i]->size_low;
         double size_d = (double)size;

         char *str_tmp = " b";
         if (size_d > 1024)
         {
            size_d /= 1024;
            str_tmp = " K";
         }

         if (size_d > 1024)
         {
            size_d /= 1024;
            str_tmp = " M";
         }

         m_file_list.SetItemText(index, 1, (String)(Double)size_d + str_tmp);
      }
   }

   EndWaitCursor();

   return P_YES;
}

// Перезагрузка списка файлов - связана с сортировкой
int CMyDlg::ReloadFileList()
{
   return LoadFileList(m_path_edit);
}

void CMyDlg::OnNMDblclkFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
   // TODO: Add your control notification handler code here
   *pResult = 0;

   int index = m_file_list.GetSelectionMark();
   if (index == -1) return;

   int num = (int)m_file_list.GetItemData(index);

   String path;

   SFileInfo **fi = m_filelist_sort.fi;

   // Если перед нами не каталог, то мы не можем в него войти
   if (!(fi[num]->attrs & FILE_ATTRIBUTE_DIRECTORY)) return;

   if (fi[num]->name == "..")
      m_folder_stack.pop_back();
   else
      m_folder_stack.push_back(fi[num]->name);

   // Формируем составной путь из стека путей
   for (int k=0; k<(int)m_folder_stack.size(); k++)
   {
      path += m_folder_stack[k];
      path += "\\";
   }

   // Загружаем новый список файлов
   if (LoadFileList(path) != P_YES)
      m_folder_stack.pop_back();
}

void CMyDlg::OnBnClickedOk()
{
   // TODO: Add your control notification handler code here
   OnOK();
}

void CMyDlg::OnHdnItemclickFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
   LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
   // TODO: Add your control notification handler code here
   *pResult = 0;

   if (phdr->iItem == 0) m_sort_mode = 0;
   else
   if (phdr->iItem == 1) m_sort_mode = 1;

   ReloadFileList();
}

void CMyDlg::OnCbnSelchangeDrivesCombo()
{
   // При изменении текущего диска
   CString drive;
   m_drives_combo.GetWindowText(drive);

   drive += ":";

   LoadFileList(drive);
   m_folder_stack.clear();
   m_folder_stack.push_back((String)drive);

   CDataExchange pdx(this, FALSE);
   m_path_edit = (String)drive + "\\";
   DoDataExchange(&pdx);

   // Тут же расчитываем свободное место на этом диске
   CalcFreeSpace();
}

void CMyDlg::OnNMRclickFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
   // При нажатии правой кнопки мыши на списке (на одном из элементов)
   CPoint point;
   GetCursorPos(&point);
   m_listpopup_menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, point.x, point.y, this);

   *pResult = 0;
}

void CMyDlg::OnPopupLoad()
{
   // При выборе из выпадающего меню пункта "Загрузить..."
   BROWSEINFO              bi;
   TCHAR                   szDestFolder[MAX_PATH]; 
   LPITEMIDLIST            pidl; 
   LPMALLOC  pMalloc = NULL;
   ZeroMemory(&bi, sizeof(bi));

   bi.hwndOwner           = this->m_hWnd;
   bi.pszDisplayName      = szDestFolder;
   bi.lpszTitle           = TEXT("Выберите каталог, куда скидывать файлы");
   bi.ulFlags             = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
   pidl = SHBrowseForFolder(&bi);
   if  (pidl)
   {
	   SHGetPathFromIDList(pidl, szDestFolder);

      // Загружаем все выделенные файлы в указанное место на диске
      SFileInfo **fi = m_filelist_sort.fi;
      SLoadFilesInfo lfi(m_client);
      lfi.hWnd = NULL;  // Нам ни к чему это пока
      lfi.b_fast_copy = params.b_fast_copy;

      // Формируем составной путь из стека путей
      for (int k=0; k<(int)m_folder_stack.size(); k++)
      {
         lfi.src_path += m_folder_stack[k];
         if (k != (int)m_folder_stack.size()-1) lfi.src_path += "\\";
      }

      if (szDestFolder[strlen(szDestFolder)-1] == '\\')
         szDestFolder[strlen(szDestFolder)-1] = 0;

      lfi.dest_path = szDestFolder;

      // Грузим файлы в отдельном потоке. Для этого сначала формируем отдельный список
      // файлов и папок, которые надо загрузить.
      POSITION pos = m_file_list.GetFirstSelectedItemPosition();
      while (pos)
      {
         int index = m_file_list.GetNextSelectedItem(pos); // Номер элемента в экранном списке
         int num   = (int)m_file_list.GetItemData(index);  // Номер элемента в массиве в памяти

         lfi.list.push_back(*fi[num]);
      }

      if (lfi.list.size() == 0) return;  // Нечего загружать!

      b_load_files_ready = 0;  // Будем проверять эту переменную. Если она станет == 1,
                               // то можно выходить из этой функции (уничтожение lfi)

      CWinThread *t = AfxBeginThread(AsyncLoadFiles, &lfi);

      while (!b_load_files_ready) Sleep(1);
   }
}

void CMyDlg::OnPopupDelete()
{
   // При выборе из выпадающего меню пункта "Удалить..."
   if (MessageBox("Ты уверен, что хочешь удалить выбранные файлы и папки", "Охуеть!", MB_YESNO|MB_ICONQUESTION) == IDYES)
      MessageBox("Нифига, такого я позволить не могу!\nЯ даже сделал так, чтобы сервер сам не мог удалять файлы!", "Ну ты вообще пиздец...", MB_ICONSTOP);
}

void CMyDlg::OnSendMessage()
{
   // Главное меню->послать сообщение
   CSendMessageDialog dlg;

   if (dlg.DoModal() == IDOK)
   {
      if (m_client.SendMessage(dlg.m_message, dlg.m_caption, MB_OK|MB_ICONSTOP) != P_YES)
         MessageBox(m_client.GetErrorMessage(), 0, MB_OK|MB_ICONSTOP);
      else
         MessageBox("Сообщение успешно отправлено", 0, MB_OK|MB_ICONINFORMATION);
   }
}

// Расчитать объем выделенного каталога
void CMyDlg::CalcFolderSize()
{
   // Пока это будет работать только для одного каталога
   if (m_file_list.GetSelectedCount() != 1)
   {
      MessageBox("Выделите один каталог!");
      return;
   }
/*
   POSITION pos = m_file_list.GetFirstSelectedItemPosition();
   while (pos)
   {
      int index = m_file_list.GetNextSelectedItem(pos); // Номер элемента в экранном списке
      int num   = (int)m_file_list.GetItemData(index);  // Номер элемента в массиве в памяти

      lfi.list.push_back(*fi[num]);
   }
*/
}

void CMyDlg::OnMmShellExecute()
{
   // Выполнить ShellExecute на сервере
   CShellExecuteDialog box;

   if (box.DoModal() == IDOK)
   {
      if (m_client.ServShellExecute(box.m_operation_s, box.m_filename, box.m_filedir) != P_YES)
         MessageBox(m_client.GetErrorMessage(), 0, MB_OK|MB_ICONSTOP);
      else
         MessageBox("Команда выполнена успешно!", 0, MB_OK|MB_ICONINFORMATION);
   }
}

void CMyDlg::OnSendFiles()
{
   // Выслать файлы на сервер
   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));

   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner   = m_hWnd;
   ofn.lpstrFile   = (LPSTR)malloc(65536*10);
   *ofn.lpstrFile  = 0;
   ofn.nMaxFile    = 65536*10;
   ofn.Flags       = OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER;

   vector<String> file_list;
   String src_path;     // Путь к каталогу, откуда брать файлы
                        // Путь, куда нужно класть файлы, хранится в m_path_edit

   if (GetOpenFileName(&ofn))
   {
      // Сначала создаем список файлов - чтоб подсчитать их количество
      const char *str_p = ofn.lpstrFile;
      src_path = str_p;   // Сразу запоминаем исходный путь
      while (*str_p)
      {
         // Пропускаем один файл
         while (*str_p++);

         // Если следующий символ - NULL, то выход из цикла
         if (!*str_p) break;

         // Иначе запоминаем следующий файл
         file_list.push_back(String(str_p));
      }

      free(ofn.lpstrFile);

      // Спрашиваем метод копирования
      int ret = MessageBox("Копировать минуя клиента?", "Вопрос", MB_OK|MB_YESNOCANCEL);
      if (ret == IDCANCEL) return;

      // Копируем файлы на сервер
      for (int i=0; i<(int)file_list.size(); i++)
      {
         String src  = src_path + "\\" + file_list[i].c_str();
         String dest = m_path_edit + file_list[i].c_str();

         // Выбираем метод копирования
         if (ret == IDYES)
            ret = m_client.SendFileDirect(src, dest);
         else
            ret = m_client.SendFile(src, dest);
         
         if (ret != P_YES)
         {
            MessageBox(m_client.GetErrorMessage(), "Ошибка копирования", MB_OK|MB_ICONSTOP);
            return;
         }
      }
   }

   MessageBox("Файлы отправлены", 0 , MB_OK|MB_ICONINFORMATION);
}

void CMyDlg::ExecuteCurrentFile()
{
   // Запуск выбранных файла!
   // Если выбрано несколько, то будут открыты сразу все!!!
   POSITION pos = m_file_list.GetFirstSelectedItemPosition();

   int errors = 0;

   while (pos)
   {
      int index = m_file_list.GetNextSelectedItem(pos); // Номер элемента в экранном списке
      int num   = (int)m_file_list.GetItemData(index);  // Номер элемента в массиве в памяти

      String path = m_path_edit + m_filelist_sort.fi[num]->name.c_str();

      if (m_client.ServShellExecute("open", path, "") != P_YES)
         errors++;

      Sleep(200);
   }

   if (errors!=0) MessageBox((String)"Произошло " +Int(errors) + " ошибок при выполнении ShellExecute");
}

void CMyDlg::ExecuteProgram()
{
   // Запуск выбранных программ!
   POSITION pos = m_file_list.GetFirstSelectedItemPosition();

   int errors = 0;

   while (pos)
   {
      int index = m_file_list.GetNextSelectedItem(pos); // Номер элемента в экранном списке
      int num   = (int)m_file_list.GetItemData(index);  // Номер элемента в массиве в памяти

      String path = m_path_edit + m_filelist_sort.fi[num]->name.c_str();

      if (m_client.ServCreateProcess(path, m_path_edit) != P_YES)
         errors++;
   }

   if (errors!=0) MessageBox((String)"Произошло " +Int(errors) + " ошибок при выполнении CreateProcess");
}

// Расчет свободного места на сервере
int CMyDlg::CalcFreeSpace()
{
   __int64 free_space = 0;
   if (m_client.GetServFreeSpace(m_path_edit, free_space) != P_YES)
   {
      m_free_space_edit.SetWindowText(m_client.GetErrorMessage());
      return P_ERROR;
   }

   free_space /= 1024*1024;

   m_free_space_edit.SetWindowText((String)(Int)(int)free_space + " Mb");

   return P_YES;
}

void CMyDlg::OnBnClickedButton1()
{
   // Сделать скриншот
   if (m_client.GetScreenShot("c:\\screen.jpg") != P_YES)
      MessageBox(m_client.GetErrorMessage());
}
