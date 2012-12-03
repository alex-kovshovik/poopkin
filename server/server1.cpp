#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "version.h"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)
////////////////////////////////////////////////////////////////////////////////
// Сервисные функции
////////////////////////////////////////////////////////////////////////////////

// Отсылает приветствие сервера
int CPushkinServClient::SendGreeting()
{
   const char *greet_msg = "Fuck off!!!";

   if (send(m_socket, greet_msg, (int)strlen(greet_msg), 0) == SOCKET_ERROR)
      return P_ERROR;

   return P_YES;
}


// Получение команды от клиента
int CPushkinServClient::GetCommand(UINT &command)
{
   int len = recv(m_socket, (char*)&command, sizeof(UINT), 0);
   if (len == SOCKET_ERROR)
      return P_ERROR;

   if (len != sizeof(UINT))
      return P_ERROR;

   return P_YES;
}

// Отсылка номера версии сервера
int CPushkinServClient::SendVersion()
{
   // Не требует авторизации!!!
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Отсылаем номер версии
   if (SendUINT(m_socket, SERVER_VERSION) != P_YES) return P_ERROR;

   return P_YES;
}

// Показ сообщения на экране
int CPushkinServClient::ShowMessage()
{
   // Получаем полностью все параметры сообщения (длину сообщения и само сообщение)
   String message;
   String caption;
   UINT flags;
   UINT b_async;  // Признак асинхронности сообщения

   // Проверяем авторизацию
   if (!m_bAuthorized)
      return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);
   else
      SendReply(m_socket, CC_CONFIRM);  // Шлем подтверждение приема данных

   if (ReceiveString(m_socket, message) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, caption) != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, flags) != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, b_async) != P_YES) return P_ERROR;

   // Показываем сообщение и отсылаем ответ
   // (пока будем показывать только синхронные сообщения)
   MessageBox(GetDesktopWindow(), message, caption, flags);

   return SendReply(m_socket, CC_CONFIRM); // Подтверждаем показ сообщения
}

#pragma comment(lib, "Winmm.lib")

// Открытие дверцы CD-ROM'a
int CPushkinServClient::OpenCDDoor()
{
   UINT b_open;
   
   // Получаем признак открытия/закрытия CD-ROM'a
   if (ReceiveUINT(m_socket, b_open) != P_YES) return P_ERROR;

   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Открываем или закрываем CD-ROM
   if (b_open)
      mciSendString("Set cdaudio door open wait",0,0,0);
   else
      mciSendString("Set cdaudio door closed wait",0,0,0);

   // Высылаем подтверждение
   return SendReply(m_socket, CC_CONFIRM);
}

// Запуск программы на выполнение
int CPushkinServClient::ServCreateProcess()
{
   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Получаем параметры запуска процесса
   String cmd_line;
   String working_dir;

   // Получаем командную строку и рабочий каталог
   if (ReceiveString(m_socket, cmd_line) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, working_dir) != P_YES) return P_ERROR;

   // Запускаем процесс
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // Запускаем программу обновления сервера
   if( !CreateProcess( NULL, (LPSTR)cmd_line.c_str(), // Command line.
                        NULL,             // Process handle not inheritable.
                        NULL,             // Thread handle not inheritable.
                        FALSE,            // Set handle inheritance to FALSE.
                        0,                // No creation flags.
                        NULL,             // Use parent's environment block.
                        (LPSTR)working_dir.c_str(),      // Рабочий каталог
                        &si,              // Pointer to STARTUPINFO structure.
                        &pi ) )           // Pointer to PROCESS_INFORMATION structure.
      return SendReply(m_socket, CC_ERROR, ERR_CREATING_PROCESS);

   // Отсылаем подтверждение того, что процесс запущен
   return SendReply(m_socket, CC_CONFIRM);
}
// Запуск какого-либо файла, например, картинки или MP3
#pragma warning(disable: 4311)
int CPushkinServClient::ServShellExecute()
{
   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение готовности принять параметры
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Получаем параметры с клиента
   String operation;
   String file;
   String dir;

   if (ReceiveString(m_socket, operation) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, file) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, dir) != P_YES) return P_ERROR;

   // Выполняем команду ShellExecute
   if ((int)ShellExecute(NULL, operation, file, NULL, dir, SW_RESTORE) <= 32)
      return SendReply(m_socket, CC_ERROR, ERR_SHELL_EXECUTE);

   return SendReply(m_socket, CC_CONFIRM);
}

// Сделать звук максимальным на Winamp'e
int CPushkinServClient::MaximizeWinampVolume()
{
   HWND hWinamp = FindWindow("Winamp v1.x", NULL);

   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   if (!hWinamp) return SendReply(m_socket, CC_ERROR, ERR_WINAMP_NOT_FOUND);

   for (int i=0; i<100; i++)
      SendMessage(hWinamp, WM_KEYDOWN, 38, 1);

   return SendReply(m_socket, CC_CONFIRM);
}

// Получение объема свободного места
int CPushkinServClient::SendFreeSpace()
{
   // Получаем свободное место
   String path;  // Путь к диску, на котором нужно определить свободное место
   DWORD sec_per_clust;
   DWORD bytes_per_sect;
   DWORD num_of_free_clust;
   DWORD tot_num_of_clust;

   // Проверяем авторизованность пользователя
   if (!m_bAuthorized)
      return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение готовности принять параметры
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Получаем параметры
   if (ReceiveString(m_socket, path) != P_YES)
      return P_ERROR;

   if (!GetDiskFreeSpace(path, &sec_per_clust, &bytes_per_sect, &num_of_free_clust, &tot_num_of_clust))
      return SendReply(m_socket, CC_ERROR, ERR_GETDISTFREESPACE);

   // Отсылаем подтверждение отсылки свободного места
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Отсылаем свободное место четырмя пакетами по 4 байта
   if (SendUINT(m_socket, sec_per_clust)     != P_YES) return P_ERROR;
   if (SendUINT(m_socket, bytes_per_sect)    != P_YES) return P_ERROR;
   if (SendUINT(m_socket, num_of_free_clust) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, tot_num_of_clust)  != P_YES) return P_ERROR;

   return P_YES;
}

// Удаление файла на сервере
int CPushkinServClient::DeleteFile()
{
   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение готовности принять параметры
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Принимаем параметр
   String fname;
   if (ReceiveString(m_socket, fname) != P_YES) return P_ERROR;

   // Удаляем файл
   if (!::DeleteFile(fname)) return SendReply(m_socket, CC_ERROR, ERR_DELETING_FILE);

   return SendReply(m_socket, CC_CONFIRM);
}
