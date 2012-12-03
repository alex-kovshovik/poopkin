// Посылка файлов
#include <iostream>
#include <vector>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "pstring.hpp"

using namespace PUSHKIN_SERVER;
using namespace std;
using namespace PSQL_API;

#pragma warning (disable: 4996)

// Отсылка файла
int CPushkinServClient::SendFile()
{
   // Получаем сначала все данные для загрузки файла с клиента, затем проверяем авторизацию
   String fpath;
   if (ReceiveString(m_socket, fpath) != P_YES) return P_ERROR;  // Это охуенная ошибка, отключаем клиента

   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   return ::SendFile(m_socket, fpath, 0);
}

// Получение файлов из переданного списка
int CPushkinServClient::SendFilesList()
{
   // Проверяем авторизацию
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Шлем подтверждение готовности отослать список файлов
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Получаем параметры команды (добавка к началу пути каждого файла и сам список путей к файлам)
   String add_s;
   vector<SFileInfo> list;

   if (ReceiveString(m_socket, add_s) != P_YES) return P_ERROR;
   if (ReceiveStrList(m_socket, list) != P_YES) return P_ERROR;

   // Идем по переданному списку и передаем файлы по одному
   for (int i=0; i<(int)list.size(); i++)
   {
      // Пропускаем директории - их мы посылать не будем
      if (list[i].attrs & FILE_ATTRIBUTE_DIRECTORY) continue;

      // Отсылаем имя передаваемого файла сначала
      if (SendString(m_socket, list[i].name) != P_YES) return P_ERROR;

      // Затем шлем сам файл
      String path = add_s + list[i].name;
      if (::SendFile(m_socket, path, 0) != P_YES) return P_ERROR;
   }

   // В конце сигнализируем клиенту, что файлов больше не будет - шлем пустую строку
   if (SendString(m_socket, (String)"") != P_YES) return P_ERROR;

   return P_YES;
}

// Получение файла с докачкой
int CPushkinServClient::GetFileEx()
{
   // Проверяем авторизованность пользователя
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение готовности принять параметры
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Получаем все данные для загрузки файла с клиента
   String fpath;
   if (ReceiveString(m_socket, fpath) != P_YES) return P_ERROR;

   // Получаем номер байта, с которого нужно докачивать файл
   __int64 off;  // Смещение относительно начала файла
   if (ReceiveINT64(m_socket, off) != P_YES) return P_ERROR;

   return ::SendFile(m_socket, fpath, off);
}
