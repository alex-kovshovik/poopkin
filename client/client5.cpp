// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// Получение списка файлов с сервера
int CPushkinClient::GetFileList(const char *path, vector<SFileInfo> &list)
{
   m_err_msg = "Ошибка получения списка файлов";

   // Отсылаем запрос серверу
   if (SendCommand(SC_GET_FILE_LIST) != P_YES) return P_ERROR;

   // Отсылаем длину строки с путем
   UINT path_len = (UINT)strlen(path);
   if (SendUINT(m_socket, path_len) != P_YES) return P_ERROR;

   // Отсылаем путь к файлам и папкам
   if (SendSTR(m_socket, path, (int)strlen(path)) != P_YES) return P_ERROR;

   // Получаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error getting file list. Error code=" << err_code << endl;
      return P_ERROR;
   }

   // Получаем сам список файлов
   if (ReceiveStrList(m_socket, list) != P_YES)
   {
      m_err_msg = "Ошибка получения массива строк с сервера";
      return P_ERROR;
   }

   return P_YES;
}

// Получение списка логических дисков
int CPushkinClient::GetDrivesList(UINT &drives)
{
   // Отсылаем запрос на получение списка логических дисков
   if (SendCommand(SC_GET_DRIVES_LIST) != P_YES) return P_ERROR;

   // Принимаем подтверждение
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_msg = "Ошибка получения списка логических дисков. Код ";
      m_err_msg += (Int)err_code;
      return P_ERROR;
   }

   // Получаем сам список
   return ReceiveUINT(m_socket, drives);
}

// Получение списка процессов
int CPushkinClient::GetProcessList(std::vector<SFileInfo> &list)
{
   // Отсылаем запрос на получение списка процессов
   if (SendCommand(SC_GET_PROCESS_LIST) != P_YES) return P_ERROR;

   // Принимаем подтверждение1
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_msg = "Ошибка получения списка процессов. Код ";
      m_err_msg += (Int)err_code;
      return P_ERROR;
   }

   // Получаем сам список
   if (ReceiveStrList(m_socket, list) != P_YES)
   {
      m_err_msg = "Ошибка получения массива строк с сервера";
      return P_ERROR;
   }

   return P_YES;
}

// Получение дерева файлов и каталогов
int CPushkinClient::GetFilesTree(const char *path, const char *add_s, std::vector<SFileInfo> &list, __int64 &total_size)
{
   // add_s - то, что надо добавить в начало каждой возвращенной строки
   m_err_code = 0;

   // Отсылаем запрос на получение списка процессов
   if (SendCommand(SC_GET_FILES_TREE) != P_YES) return P_ERROR;

   // Принимаем подтверждение
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg = "Ошибка получения дерева файлов и каталогов. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // Шлем начальный каталог
   if (SendString(m_socket, (String)path) != P_YES)
   {
      m_err_msg = "Ошибка получения дерева файлов и каталогов. Ошибка при отсылке начального пути";
      return P_ERROR;
   }

   // Шлем то, что нужно добавить в начало каждой возвращенной строки
   if (SendString(m_socket, (String)add_s) != P_YES)
   {
      m_err_msg = "Ошибка получения дерева файлов и каталогов. Ошибка при отсылке добавки к пути";
      return P_ERROR;
   }

   vector<SFileInfo> temp_list;

   // Получаем сам список
   if (ReceiveStrList(m_socket, temp_list) != P_YES)
   {
      m_err_msg = "Ошибка получения массива строк с сервера";
      return P_ERROR;
   }

   // Добавляем все полученное ко входному списку
   for (int i=0; i<(int)temp_list.size(); i++)
      list.push_back(temp_list[i]);

   // Получаем полный объем всех файлов
   if (ReceiveINT64(m_socket, total_size) != P_YES)
   {
      m_err_msg = "Ошибка получения полного объема всех файлов";
      return P_ERROR;
   }

   return P_YES;
}

// Докачка файла
int CPushkinClient::GetFileEx(const char *src, const char *dest)
{
   m_err_code = 0;
   if (!src)
   {
      m_err_msg = "Нулевой параметр src в функции GetFileEx";
      return P_ERROR;
   }

   // Опеределяем размер файла
   HANDLE hFile = CreateFile(dest, FILE_READ_DATA|FILE_READ_ATTRIBUTES,FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
      return P_ERROR;

   DWORD fSizeHigh = 0;
   DWORD fSizeLow = GetFileSize(hFile, &fSizeHigh);
   __int64 fSize = fSizeHigh;
   fSize <<=32;
   fSize |= fSizeLow;

   // Отсылаем команду на получение файла
   if (SendCommand(SC_GET_FILE_EX) != P_YES) return P_ERROR;  // Сообщение об ошибке уже заполнено

   // Принимаем подтверждение
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg = "Ошибка получения файла GetFileEx. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // И сразу отсылаем параметры получения файла
   if (SendString(m_socket, (String)src) != P_YES)
   {
      m_err_msg = "Ошибка отсылки пути к файлу для загрузки с сервера";
      return P_ERROR;
   }

   // Смещение от начала
   if (SendINT64(m_socket, fSize+1) != P_YES) return P_ERROR;

   ret = ReceiveFile(m_socket, dest, 1, BlockReadCallback);
   if (ret != P_YES)
   {
//      m_err_msg = g_err_msg;
      return ret;
   }
   return P_YES;
}
