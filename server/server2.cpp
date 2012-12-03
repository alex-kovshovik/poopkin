// Авторизация
#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)
// Обрезает пробелы справа от выражения
static void CutSpacesRight(char *m_socket)
{
   while (*m_socket && *m_socket!=' ') m_socket++;
   if (*m_socket==' ') *m_socket=0;
}

// Авторизация клиента
int CPushkinServClient::Authorize()
{
   // 1. Получаем логин и пароль. Длина данных авторизации = 40 (20 логин + 20 пароль)
   char buf[41];
   int len = recv(m_socket, buf, 40, 0);
   if (len == SOCKET_ERROR) return P_ERROR;
   if (len != 40) return SendReply(m_socket, CC_ERROR, ERR_WRONG_LOGIN_DATA);  // Не достаточно данных

   // Выделяем логин и пароль из строки
   char login[21];
   char pwd[21];
   strncpy(login, buf, 20);
   login[20] = 0;
   strncpy(pwd, buf+20, 20);
   pwd[20] = 0;

   CutSpacesRight(login);
   CutSpacesRight(pwd);

   if (strcmp(login, "pushkin")==0 && strcmp(pwd,"whore")==0)
   {
      m_bAuthorized  = true;
      return SendReply(m_socket, CC_CONFIRM);
   }

   return SendReply(m_socket, CC_ERROR, ERR_WRONG_LOGIN_OF_PWD);
}
