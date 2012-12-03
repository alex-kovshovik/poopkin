// Реализация класса CPushkinServer
#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "version.h"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)

////////////////////////////////////////////////////////////////////////////////////////
// Конструкторы и деструктор
////////////////////////////////////////////////////////////////////////////////////////
CPushkinServClient::CPushkinServClient()
{
   server_p = NULL;                                   // Указатель на родителя
   m_socket = NULL;                                   // Сокет клиента
   memset(&m_client_addr, 0, sizeof(m_client_addr));  // Информация о клиенте
   m_bAuthorized = false;                             // Признак того, что клиент авторизован
}
CPushkinServClient::CPushkinServClient(const CPushkinServClient &cl_in)
{
   server_p      = cl_in.server_p;
   m_socket      = cl_in.m_socket;
   m_client_addr = cl_in.m_client_addr;
   m_bAuthorized = cl_in.m_bAuthorized;
}

CPushkinServClient &CPushkinServClient::operator=(const CPushkinServClient cl_in)
{
   server_p      = cl_in.server_p;
   m_socket      = cl_in.m_socket;
   m_client_addr = cl_in.m_client_addr;
   m_bAuthorized = cl_in.m_bAuthorized;

   return *this;
}
CPushkinServClient::~CPushkinServClient()
{
}
