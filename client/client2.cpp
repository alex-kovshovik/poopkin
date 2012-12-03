// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;
using namespace PSQL_API;

// Авторизация на сервере
int CPushkinClient::Login(const char *login, const char *pwd)
{
   const char *err_msg = "Ошибка авторизации на сервере";

   char buf[41];  // Длина посылаемых данных - 40 байт (20 на логин, 20 на пароль)
   sprintf(buf, "%-20s%-20s", login, pwd);

   // Отсылаем запрос авторизации
   if (SendCommand(SC_AUTHORIZE) != P_YES)
   {
      m_err_msg = err_msg;
      return P_ERROR;
   }

   // Отсылаем данные для авторизации
   if (send(m_socket, buf, 40, 0) == SOCKET_ERROR)
   {
      m_err_msg = err_msg;
      return P_ERROR;
   }

   // Ждем ответ сервера - что он на это скажет
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      // Анализируем код ошибки
      m_err_msg =  "Ошибка авторизации на сервере. Код ";
      m_err_msg += (Int)err_code;
      return P_ERROR;
   }

   // Запоминаем информацию о текущем логине
   strcpy(m_prev_login, login);
   strcpy(m_prev_pwd, pwd);

   return P_YES;  // Авторизация подтверждена
}

// Перелогинивание :)))
int CPushkinClient :: Relogin()
{
   return Login(m_prev_login, m_prev_pwd);
}
