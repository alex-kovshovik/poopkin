// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;
using namespace PSQL_API;

// ����������� �� �������
int CPushkinClient::Login(const char *login, const char *pwd)
{
   const char *err_msg = "������ ����������� �� �������";

   char buf[41];  // ����� ���������� ������ - 40 ���� (20 �� �����, 20 �� ������)
   sprintf(buf, "%-20s%-20s", login, pwd);

   // �������� ������ �����������
   if (SendCommand(SC_AUTHORIZE) != P_YES)
   {
      m_err_msg = err_msg;
      return P_ERROR;
   }

   // �������� ������ ��� �����������
   if (send(m_socket, buf, 40, 0) == SOCKET_ERROR)
   {
      m_err_msg = err_msg;
      return P_ERROR;
   }

   // ���� ����� ������� - ��� �� �� ��� ������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      // ����������� ��� ������
      m_err_msg =  "������ ����������� �� �������. ��� ";
      m_err_msg += (Int)err_code;
      return P_ERROR;
   }

   // ���������� ���������� � ������� ������
   strcpy(m_prev_login, login);
   strcpy(m_prev_pwd, pwd);

   return P_YES;  // ����������� ������������
}

// ��������������� :)))
int CPushkinClient :: Relogin()
{
   return Login(m_prev_login, m_prev_pwd);
}
