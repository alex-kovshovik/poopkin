// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "common.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// Подключаем библиотеку WSock32.lib
#pragma comment(lib, "WSock32.lib")

//////////////////////////////////////////////////////////////////////////////
// Конструкторы и деструктор
//////////////////////////////////////////////////////////////////////////////
CPushkinClient::CPushkinClient()
{
   InitStructures();
}

// Конструктор копирования
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
   // Закрываем сокет, если он был открыт (отключаемся от сервера)
   if (m_socket) closesocket(m_socket);
}

//////////////////////////////////////////////////////////////////////////////
// Инициализация структур данных
//////////////////////////////////////////////////////////////////////////////
void CPushkinClient::InitStructures()
{
   m_socket = NULL;                             // Клиентский сокет
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
// Подключение к серверу и отключение
//////////////////////////////////////////////////////////////////////////////
int CPushkinClient::Connect(const char *IP, int port)
{
   m_server_version = 0;

   // Запоминаем информацию для последующего реконнекта
   strcpy(m_connect_addr, IP);
   m_connect_port = port;
   m_err_code = 0;

   // Разрываем связь, если мы уже были соединены
   if (m_socket) Disconnect();

   // Открываем сокет
   m_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (m_socket == INVALID_SOCKET)
   {
      m_err_msg = "Ошибка создания сокета";
      return P_ERROR;
   }

   // Подключаемся к серверу
   memset(&m_server_addr, 0, sizeof(m_server_addr));
   m_server_addr.sin_family            = AF_INET;
   m_server_addr.sin_addr.S_un.S_addr  = inet_addr(IP);
   m_server_addr.sin_port              = htons(port);

   // Соединяемся
   if (connect(m_socket, (LPSOCKADDR)&m_server_addr, sizeof(m_server_addr)) == SOCKET_ERROR)
   {
      m_err_msg = "Ошибка соединения с сервером. IP:";
      m_err_msg += IP;
      m_err_msg += ", Port:";
      m_err_msg += (Int)port;

      Disconnect();
      return P_ERROR;
   }

   // Получаем приветствие сервера
   if (ReceiveGreeting() != P_YES)
   {
      Disconnect();
      return P_ERROR;  // Если приветствие ошибочное, то хреново - ошибка внутри
   }

   // Автоматически получаем версию сервера
   UINT version;
   if (GetServerVersion(version) != P_YES)
   {
      Disconnect();    // Сразу же отключаемся воизбежание недоразумений
      return P_ERROR;  // Ошибка уже заполнена
   }
   m_server_version = version;

   return P_YES;
}

int CPushkinClient::Reconnect()
{
   return Connect(m_connect_addr, m_connect_port);
}

// Отключение от сервера (закрытие сокета)
void CPushkinClient::Disconnect()
{
   closesocket(m_socket);
   m_socket = NULL;
   m_server_version = 0;
}
