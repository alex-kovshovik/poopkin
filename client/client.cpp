// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "common.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// ���������� ���������� WSock32.lib
#pragma comment(lib, "WSock32.lib")

//////////////////////////////////////////////////////////////////////////////
// ������������ � ����������
//////////////////////////////////////////////////////////////////////////////
CPushkinClient::CPushkinClient()
{
   InitStructures();
}

// ����������� �����������
CPushkinClient::CPushkinClient(CPushkinClient &cl)
{
   InitStructures();

   strcpy(m_connect_addr, cl.m_connect_addr);
   m_connect_port  = cl.m_connect_port;
   strcpy(m_prev_login, cl.m_prev_login);
   strcpy(m_prev_pwd, cl.m_prev_pwd);
}

CPushkinClient::~CPushkinClient()
{
   // ��������� �����, ���� �� ��� ������ (����������� �� �������)
   if (m_socket) closesocket(m_socket);
}

//////////////////////////////////////////////////////////////////////////////
// ������������� �������� ������
//////////////////////////////////////////////////////////////////////////////
void CPushkinClient::InitStructures()
{
   m_socket = NULL;                             // ���������� �����
   m_err_code = 0;
   m_server_version = 0;

   strcpy(m_connect_addr, "");
   m_connect_port = 0;

   strcpy(m_prev_login, "");
   strcpy(m_prev_pwd, "");

   BlockReadCallback = NULL;
   ReceivingFileCallback = NULL;
   ErrReceivingFileCallback = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// ����������� � ������� � ����������
//////////////////////////////////////////////////////////////////////////////
int CPushkinClient::Connect(const char *IP, int port)
{
   m_server_version = 0;

   // ���������� ���������� ��� ������������ ����������
   strcpy(m_connect_addr, IP);
   m_connect_port = port;
   m_err_code = 0;

   // ��������� �����, ���� �� ��� ���� ���������
   if (m_socket) Disconnect();

   // ��������� �����
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket == INVALID_SOCKET)
   {
      m_err_msg = "������ �������� ������";
      return P_ERROR;
   }

   // ������������ � �������
   memset(&m_server_addr, 0, sizeof(m_server_addr));
   m_server_addr.sin_family            = AF_INET;
   m_server_addr.sin_addr.S_un.S_addr  = inet_addr(IP);
   m_server_addr.sin_port              = htons(port);

   // �����������
   if (connect(m_socket, (LPSOCKADDR)&m_server_addr, sizeof(m_server_addr)) == SOCKET_ERROR)
   {
      m_err_msg = "������ ���������� � ��������. IP:";
      m_err_msg += IP;
      m_err_msg += ", Port:";
      m_err_msg += (Int)port;

      Disconnect();
      return P_ERROR;
   }

   // �������� ����������� �������
   if (ReceiveGreeting() != P_YES)
   {
      Disconnect();
      return P_ERROR;  // ���� ����������� ���������, �� ������� - ������ ������
   }

   // ������������� �������� ������ �������
   UINT version;
   if (GetServerVersion(version) != P_YES)
   {
      Disconnect();    // ����� �� ����������� ����������� �������������
      return P_ERROR;  // ������ ��� ���������
   }
   m_server_version = version;

   return P_YES;
}

int CPushkinClient::Reconnect()
{
   return Connect(m_connect_addr, m_connect_port);
}

// ���������� �� ������� (�������� ������)
void CPushkinClient::Disconnect()
{
   closesocket(m_socket);
   m_socket = NULL;
   m_server_version = 0;
}
