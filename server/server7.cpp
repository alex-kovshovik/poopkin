#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "version.h"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)

// Загрузка файла на сервер
int CPushkinServClient::UploadFile()
{
   // Проверка авторизации
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Принимаем путь с именем файла, куда его записать
   UINT path_len;  // Длина пути с именем файла
   if (ReceiveUINT(m_socket, path_len) != P_YES) return P_ERROR;

   // Выделяем память для хранения пути
   char *path_p = (char*)malloc(path_len+1);
   if (!path_p) return P_ERROR;

   // Получаем путь с именем файла
   if (ReceiveSTR(m_socket, path_p, path_len) != P_YES)
   {
      free(path_p);
      return P_ERROR;
   }

   path_p[path_len] = 0;  // Обязательный терминирующий нуль

   // Получаем сам файл
   if (::ReceiveFile(m_socket, path_p, 0) != P_YES)
   {
      free(path_p);
      return P_ERROR;
   }

   free(path_p);

   // Отсылаем подтверждение приема файла на клиента
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   return P_YES;  // Выходим - файл загружен
}

// Прямая загрузка файлов с открытой шары или с другого места на сервере - средствами сервера
int CPushkinServClient::DirectFileUpload()
{
   // Проверка авторизации
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // Отсылаем подтверждение
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // Путь, откуда брать файл, и путь, куда его положить
   String src;
   String dest;

   // Принимаем пути
   if (ReceiveString(m_socket, src)  != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, dest) != P_YES) return P_ERROR;

   // Копируем файл из одного места в другое
   if (!CopyFile(src, dest, FALSE))
      return SendReply(m_socket, CC_ERROR, ERR_COPYING_FILE);

   // Отсылаем подтверждение выполнения операции
   return SendReply(m_socket, CC_CONFIRM);
}
