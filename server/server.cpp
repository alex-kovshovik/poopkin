// Реализация класса CPushkinServer
#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "version.h"

// Подключаем библиотеку WSock32.lib
#pragma comment(lib, "WSock32.lib")
#pragma comment(lib, "GenType.lib")

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)
//////////////////////////////////////////////////////////////////////////////
// Конструкторы и деструктор
//////////////////////////////////////////////////////////////////////////////
CPushkinServer::CPushkinServer()
{
   InitStructures();  // Инициализация структур данных нулями
}

CPushkinServer::~CPushkinServer()
{
   // Закрываем серверный сокет, если он был открыт
   if (m_socket) closesocket(m_socket);
   if (m_bWinsockInited) WSACleanup();
}
//////////////////////////////////////////////////////////////////////////////
// Инициализация структур данных
//////////////////////////////////////////////////////////////////////////////
void CPushkinServer::InitStructures()
{
   memset(&m_wsa_data, 0, sizeof(m_wsa_data));  // Данные интерфейса сокетов
   m_socket = NULL;                             // Серверный сокет

   m_bWinsockInited = false;
   m_bStopServer = false;
}

//////////////////////////////////////////////////////////////////////////////
// Инициализация сервера
//////////////////////////////////////////////////////////////////////////////
int CPushkinServer::Init(int port_num)
{
   // Обработка ошибок пока производиться не будет
   
   // 1. Инициализируем сокеты
   if (WSAStartup(0x101,&m_wsa_data) != 0)
      return P_ERROR;

   m_bWinsockInited = true;  // Выставляем галочку, что WINSOCK был проинициализирован

   // 2. Открываем сокет
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket == INVALID_SOCKET) return P_ERROR;

   // 3. Привязываем сокет к адресу
   sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family            = AF_INET;
   addr.sin_addr.S_un.S_addr  = INADDR_ANY;
   addr.sin_port              = htons(port_num);  // Наш порт, который будем открывать для коннектов
   if (bind(m_socket, (LPSOCKADDR)&addr, sizeof(addr)) != 0)  return P_ERROR;

   // 4. Устанавливаем сокет в режим прослушивания
   if (listen(m_socket, 1) == SOCKET_ERROR) return P_ERROR;

   return P_YES;
}

DWORD WINAPI ChatWithClient(LPVOID pParam);  // Прототип потока, работающего с клиентом

//////////////////////////////////////////////////////////////////////////////
// Запуск сервера
//////////////////////////////////////////////////////////////////////////////
int CPushkinServer::Run()
{
   int         from_len;      // Длина структуры данных подключенного клиента
   SOCKET      client_socket; // Сокет клиента
   sockaddr_in client_addr;   // Структура, хранящая адрес клиента

   // Бесконечный цикл получения соединений
   while (true)
   {
      // Принимаем соединения от клиентов
      from_len      = sizeof(client_addr);
      client_socket = accept(m_socket, (LPSOCKADDR)&client_addr, &from_len);

      // Проверяем ошибочность соединения
      if (client_socket == INVALID_SOCKET)
      {
         // Это может быть в случае, когда один из потоков клиента закрыл серверный сокет
         // Тогда эта функция возвращает ошибочный сокет и мы можем проверить, нужно ли остановить сервер
         if (m_bStopServer)
         {
//            MessageBox(NULL, "Сервер остановлен", "Fuck", MB_OK);
            return P_YES;  // Остановка сервера
         }
         continue; // Если что опять переходим в режим ожидания соединения
      }

      // Создаем клиента, записываем его в массив клиентов и передаем управление новому потоку
      CPushkinServClient *cl = new CPushkinServClient();
      cl->m_socket      = client_socket;
      cl->m_client_addr = client_addr;
      cl->server_p      = this;

      cout << "Client connected..." << endl;

      // Передаем управление новому потоку, который будет работать с этим клиентом
      DWORD thread_id;
      HANDLE hThread = CreateThread(NULL, 0, ChatWithClient, cl, 0, &thread_id);
   }

   return P_YES;  // Нормальный выход
}

// Шаблон класса, который в своем деструкторе удалит переданный указатель на объект
template <class T> class CDeleteObj
{
   T *&m_p;
public:
   CDeleteObj(T *&p) : m_p(p) {}
   ~CDeleteObj()
   {
      if (m_p) {delete m_p; m_p=NULL;}
      cout << "...client disconnected" << endl;
   }
};

// Поток работы с одним клиентом
DWORD WINAPI ChatWithClient(LPVOID pParam)
{
   if (!pParam) return 0;

   CPushkinServClient *cl = (CPushkinServClient*)pParam;
   CDeleteObj<CPushkinServClient> del_obj(cl);

   // Сразу отсылаем приветствие клиенту
   if (cl->SendGreeting() != P_YES) return P_ERROR;

   UINT command;          // Для хранения номера команды

   // Бесконечный цикл работы с одним клиентом
   while (true)
   {
      // 1. Получение команды (каждая команда пронумерована в commands.h)
      if (cl->GetCommand(command) != P_YES)
         break;  // Заканчиваем работу с клиентом

      // 2. Проверяем команду на отключение сервера
      if (command == SC_SHUTDOWN_SERVER)
      {
         if (cl->m_bAuthorized)
         {
            if (SendReply(cl->m_socket, CC_CONFIRM) == P_ERROR) break;
            // Тут надо как-то сообщить серверу, что его хотят остановить
            // Попробуем просто закрыть серверный сокет
            cl->server_p->m_bStopServer = true;  // Признак того, что сервер нужно остановить
            closesocket(cl->server_p->m_socket);
         }
         else
         {
            if (SendReply(cl->m_socket, CC_ERROR, ERR_NOT_AUTHORIZED) == P_ERROR) break;
            continue; // Продолжаем работать с клиентом
         }
      }

      // 3. Обработка команды
      int ret = cl->ProcessCommand(command);
      if (ret == P_ERROR) break;  // Заканчиваем работу с клиентом
   };

   // Закрываем соединение с клиентом
   closesocket(cl->m_socket);

   return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Работа с клиентом
//////////////////////////////////////////////////////////////////////////////
// Обработка команды
int CPushkinServClient::ProcessCommand(UINT command)
{
   switch (command)
   {
   case SC_AUTHORIZE:        return Authorize();
   case SC_GET_FILE:         return SendFile();
   case SC_GETVERSION:       return SendVersion();
   case SC_UPDATE_SERVER:    return UpdateServer();
   case SC_GET_FILE_LIST:    return GetFileList();
   case SC_GET_DRIVES_LIST:  return SendDrivesList();
   case SC_SHOW_MESSAGE:     return ShowMessage();
   case SC_GET_PROCESS_LIST: return GetProcessList();
   case SC_OPEN_CD_DOOR:     return OpenCDDoor();
   case SC_GET_SCREENSHOT:   return GetScreenShot();
   case SC_UPLOAD_FILE:      return UploadFile();
   case SC_CREATE_PROCESS:   return ServCreateProcess();
   case SC_SHELL_EXECUTE:    return ServShellExecute();
   case SC_MAX_WINAMP_VOL:   return MaximizeWinampVolume();
   case SC_GET_FILES_TREE:   return GetFilesTree();
   case SC_GET_FILES_LIST:   return SendFilesList();
   case SC_GET_FREE_SPACE:   return SendFreeSpace();
   case SC_DIRECT_UPLOAD:    return DirectFileUpload();
   case SC_DELETE_FILE:      return DeleteFile();
   case SC_GET_FILE_EX:      return GetFileEx();
   default: return SendReply(m_socket, CC_ERROR, ERR_UNKNOWN_COMMAND);
   }
   return P_YES;
}
