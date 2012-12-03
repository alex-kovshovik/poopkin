// Получение списка файлов по данному пути
#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "tlhelp32.h"

using namespace PUSHKIN_SERVER;
using namespace std;
using namespace PSQL_API;

#pragma warning (disable: 4996)
// Получение списка файлов и каталогов по данному пути
int CPushkinServClient::GetFileList()
{
   // Получаем сначала все необходимые данные запроса (длину пути и сам путь)
   String path;
   if (ReceiveString(m_socket, path) != P_YES) return P_ERROR;

   // Надо обрезать последний слэш, если он там есть!!!

   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Подготавливаем список файлов
   vector<SFileInfo> list;
   SFileInfo fi;

   WIN32_FIND_DATA fd;
   ZeroMemory(&fd, sizeof(fd));

   HANDLE hFile = FindFirstFile(path+"\\*", &fd);

   if (hFile == INVALID_HANDLE_VALUE) return SendReply(m_socket, CC_ERROR, ERR_SEARCHING_FILES);

   do
   {
      fi.attrs     = fd.dwFileAttributes;
      fi.name      = fd.cFileName;
      fi.size_high = fd.nFileSizeHigh;
      fi.size_low  = fd.nFileSizeLow;

      list.push_back(fi);
   } while(FindNextFile(hFile, &fd));

   FindClose(hFile);  // Не забываем закрыть хэндл поиска

   // Когда подготовили список файлов, отсылаем подтверждение возможности его получения
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Отсылаем сам список
   return SendStrList(m_socket, list);
}

// Получение списка логических дисков
int CPushkinServClient::SendDrivesList()
{
   // Проверяем авторизацию и отсылаем
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   UINT drives = GetLogicalDrives();

   return SendUINT(m_socket, drives);
}


int CPushkinServClient::GetProcessList()
{
   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Подготавливаем список процессов
   vector<SFileInfo> list;
   SFileInfo fi;

   HANDLE hSnap;
   hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
   if (hSnap == NULL)
      return SendReply(m_socket, CC_ERROR, ERR_GET_PROCESS_LIST);

   PROCESSENTRY32 proc;
	if (Process32First(hSnap, &proc))
	{
      fi.name = proc.szExeFile;
      list.push_back(fi);
		while (Process32Next(hSnap, &proc)) 
      {
         fi.name = proc.szExeFile;
         list.push_back(fi);
      }
	}

   // Когда подготовили список процессов, отсылаем подтверждение возможности его получения
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Отсылаем сам список
   return SendStrList(m_socket, list);
}

// Рекурсивная функция прохода по списку файлов и каталогов
// (в одном списке будут и файлы, и каталоги, только каталоги - раньше)
int AddFilesTree(String path, String rel_path, vector<SFileInfo> &list, __int64 &total_size)
{
   // rel_path - относительный путь - его мы и будем запоминать, а передавать дальше будем оба
   // total_size - для хранения полного объема всех файлов в дереве

   // Подготавливаем список файлов
   SFileInfo fi;

   // Нужно удалить последний слэш в пути (для этого переделаем PSQL_API::String - допишем там функцию, 
   // удаляющею из строки подстроку).
   const char *path_p = path.c_str();
   if (path.Length() > 0)
      if (path_p[path.Length()-1] == '\\')
         path.Cut(path.Length()-1, 1);

   // Тоже самое делаем и с относительным путем
   path_p = rel_path.c_str();
   if (rel_path.Length() > 0)
      if (path_p[rel_path.Length()-1] == '\\')
         rel_path.Cut(rel_path.Length()-1, 1);

   WIN32_FIND_DATA fd;
   ZeroMemory(&fd, sizeof(fd));

   HANDLE hFile = FindFirstFile(path+"\\*", &fd);

   if (hFile == INVALID_HANDLE_VALUE) return P_YES;  // Не смогли войти в какой-то путь - ну и пох :)
   int ret = P_YES;

   do
   {
      if (strcmp(fd.cFileName,".")==0 || strcmp(fd.cFileName,"..")==0) continue;

      String rel_path_tmp = rel_path;
      rel_path_tmp += "\\";
      rel_path_tmp += fd.cFileName;

      fi.attrs     = fd.dwFileAttributes;
      fi.name      = rel_path_tmp;
      fi.size_high = fd.nFileSizeHigh;
      fi.size_low  = fd.nFileSizeLow;

      __int64 temp64 = fd.nFileSizeHigh;
      temp64 <<= 32;
      temp64 |= fd.nFileSizeLow;
      total_size += temp64;

      // Добавляем в список не зависимо ни от чего
      list.push_back(fi);

      // Затем смотрим. Если это директория, то идем ввнутрь
      if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
         String path_tmp = path;
         path_tmp += "\\";
         path_tmp += fd.cFileName;

         ret = AddFilesTree(path_tmp, rel_path_tmp, list, total_size);
         if (ret != P_YES) break;
      }
   } while(FindNextFile(hFile, &fd));

   FindClose(hFile);  // Не забываем закрыть хэндл поиска

   return ret;
}

// Создание дерева файлов и каталогов
int CPushkinServClient::GetFilesTree()
{
   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение готовности отослать все, что клиенту угодно
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Будем отсылать только относительный путь к файлу на сервере
   // (относительно переданного пути)
   // А полный путь будет генериться на сервере свой, на клиенте свой

   // Принимаем путь, по которому будем искать файлы (и каталоги)
   String path;
   String add_s;  // То, что нужно добавить к началу каждого пути
   if (ReceiveString(m_socket, path) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, add_s) != P_YES) return P_ERROR;

   vector<SFileInfo> list;
   __int64 total_size = 0;

   AddFilesTree(path, add_s, list, total_size);

   // Отсылаем полученный список
   if (SendStrList(m_socket, list) != P_YES) return P_ERROR;

   // За одно отсылаем количество прочитанных байт в виде переменной типа __int64
   if (SendINT64(m_socket, total_size) != P_YES) return P_ERROR;

   return P_YES;
}
