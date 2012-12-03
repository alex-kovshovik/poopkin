// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Сервисные функции
//////////////////////////////////////////////////////////////////////////////

// Получение приветствия от сервера
int CPushkinClient::ReceiveGreeting()
{
   m_err_code = 0;

   char buf[64];
   int len = recv(m_socket, buf, 64, 0);
   if (len == SOCKET_ERROR)
      return P_ERROR;

   // Мы должны получить ответ: "Fuck off!!!"
   const char *fuck = "Fuck off!!!";
   const char *hello_error = "Ошибочное приветствие сервера";
   if (len != strlen(fuck))
   {
      m_err_msg = hello_error;
      return P_ERROR;
   }

   buf[len] = 0;

   if (strcmp(buf, fuck) != 0)
   {
      m_err_msg = hello_error;
      return P_ERROR;
   }

   return P_YES;
}

// Отсылает команду
int CPushkinClient::SendCommand(UINT code)
{
   m_err_code = 0;
   if (send(m_socket, (char*)&code, sizeof(code), 0) == SOCKET_ERROR)
   {
      m_err_code = 0;
      m_err_msg = "Ошибка отсылки команды серверу";
      return P_ERROR;
   }

   return P_YES;
}

// Отключение сервера и, соответственно, автоматический дисконнект
int CPushkinClient::ShutdownServer()
{
   m_err_code = 0;

   // Отсылаем команду серверу
   if (SendCommand(SC_SHUTDOWN_SERVER) != P_YES)
      return P_ERROR;  // Ошибка уже заполнена

   // Принимаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно остановить сервер. ";
      m_err_msg += GetErrorText(err_code); 
      return P_ERROR;
   }

   Disconnect();

   return P_YES;
}

// Получение версии сервера
int CPushkinClient::GetServerVersion(UINT &ver)
{
   m_err_code = 0;

   // Отсылаем команду серверу
   if (SendCommand(SC_GETVERSION) != P_YES) return P_ERROR;

   // Принимаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Ошибка получения версии сервера. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Принимаем номер версии
   if (ReceiveUINT(m_socket, ver) != P_YES)
   {
      m_err_msg  = err_getting_data;
      return P_ERROR;
   }

   return P_YES;
}

// Отсылка сообщения серверу (показ на экране)
int CPushkinClient::SendMessage(const char *message, const char *caption, int flags, int b_async)
{
   // b_async - признак асинхронного сообщения
   m_err_code = 0;
   m_err_msg  = NULL;
   if (!message)
   {
      m_err_msg = "Ошибка отсылки сообщения на сервер: сообщение пустое";
      return P_ERROR;
   }

   // Шлем команду серверу
   if (SendCommand(SC_SHOW_MESSAGE) != P_YES) return P_ERROR;  // Ошибка уже заполнена внутри

   // Получаем подтверждение отсылки данных сообщения
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Ошибка отсылки сообщения на сервер. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Сразу отсылаем текст сообщения,заголовок,флаги и признак асинхронности
   if (SendString(m_socket, (String)message) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)caption) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, (UINT)flags) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, (UINT)b_async) != P_YES) return P_ERROR;

   // Получаем подтверждение показа сообщения
   // (если оно асинхронное, то мы получим подтверждение сразу, иначе только когда его прочитают)
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Показа сообщения на экране сервера. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }
   return ret;
}

// Открыть дверцу CD-ROM
int CPushkinClient::OpenCDDoor(UINT b_open)
{
   m_err_code = 0;

   // Отсылаем команду на открытие или закрытие CD-ROM'a
   if (SendCommand(SC_OPEN_CD_DOOR) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, b_open) != P_YES) return P_ERROR;

   // Получаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Не могу открыть лоток CD-ROM'a. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}

// Запустить процесс на сервере
int CPushkinClient::ServCreateProcess(const char *cmd_line, const char *working_dir)
{
   m_err_code = 0;

   if (!cmd_line)
   {
      m_err_msg = "Невозможно создать процесс. Пустая командная строка";
      return P_ERROR;
   }

   // Отсылаем команду на открытие или закрытие CD-ROM'a
   if (SendCommand(SC_CREATE_PROCESS) != P_YES) return P_ERROR;

   // Получаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно создать процесс. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Отсылаем параметры
   if (SendString(m_socket, (String)cmd_line) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)working_dir) != P_YES) return P_ERROR;

   // Получаем подтверждение
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = 0;
      m_err_msg  = "Невозможно создать процесс. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }
   return P_YES;
}

// Увеличить звук Winamp'a до максимума
int CPushkinClient::MaximizeWinampVolume()
{
   m_err_code = 0;

   // Отсылаем команду для увеличения звука Winamp'a до максимума
   if (SendCommand(SC_MAX_WINAMP_VOL) != P_YES) return P_ERROR;

   // Получаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно максимизировать звук Winamp'a. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}

// Выполнение функции ShellExecute на сервере
int CPushkinClient::ServShellExecute(const char *operation, const char *file, const char *dir)
{
   m_err_code = 0;

   if (!operation || !file || !dir)
   {
      m_err_msg = "Ошибка выполнения команды ShellExecute: нулевые параметры";
      return P_ERROR;
   }

   // Отсылаем команду и получаем подтверждение
   if (SendCommand(SC_SHELL_EXECUTE) != P_YES) return P_ERROR;

   // Получаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно выполнить команду ShellExecute на сервере. Ошибка ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Отсылаем три строки-параметра
   if (SendString(m_socket, (String)operation) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)file) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)dir) != P_YES) return P_ERROR;

   // Получаем подтверждение выполнения команды
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно выполнить команду ShellExecute на сервере. Ошибка ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}

// Получение свободного места на сервере
int CPushkinClient::GetServFreeSpace(const char *path_in, __int64 &free_space)
{
   m_err_code = 0;
   m_err_msg = "";
   if (!path_in) return P_ERROR;

   // Длина строки должна превышать 3 символа
   if (strlen(path_in)<3) return P_ERROR;

   // Нужно обрезать отсальную часть пути
   String path = path_in;
   char *str_p = (char*)path.c_str();
   str_p[3] = 0;  // Обрезаем остаток пути

   // Отсылаем команду на получение свободного места
   if (SendCommand(SC_GET_FREE_SPACE) != P_YES) return P_ERROR;

   // Принимаем подтверждение готовности принять параметры
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно выполнить команду GetDiskFreeSpace. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Сразу отсылаем параметр
   if (SendString(m_socket, path) != P_YES) return P_ERROR;

   // Принимаем подтверждение выполнения команды
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "Невозможно выполнить команду GetDiskFreeSpace2. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Принимаем все параметры
   UINT sec_per_clust;
   UINT bytes_per_sect;
   UINT num_of_free_clust;
   UINT tot_num_of_clust;

   if (ReceiveUINT(m_socket, sec_per_clust)     != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, bytes_per_sect)    != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, num_of_free_clust) != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, tot_num_of_clust)  != P_YES) return P_ERROR;

   free_space = num_of_free_clust;
   free_space *= sec_per_clust;
   free_space *= bytes_per_sect;

   return P_YES;
}
