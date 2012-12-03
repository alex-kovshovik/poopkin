// Реализация класса CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// Буфер для отсылки файла
#define SEND_FILE_BUFFER_SIZE 16384

// Обновление сервера до более новой версии
int CPushkinClient::UpdateServer(const char *new_serv_name)
{
   // Сначала спрашиваем у сервера, можем ли мы обновить его до новой версии
   if (SendCommand(SC_UPDATE_SERVER) != P_YES) return P_ERROR;

   // Принимаем подтверждение
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error updating server. Error code=" << err_code << endl;
      return P_ERROR;
   }

   // Когда подтверждение принято, отсылаем новый файл
   if (::SendFile(m_socket, new_serv_name, 0) != P_YES) return P_ERROR;

   // Получаем подтверждение с сервера об успешном приеме файла
   if (IsConfirmed(m_socket, err_code) != P_YES) return P_ERROR;

   // Отключаемся от сервера и ждем пока он перезагрузится
   Disconnect();

   cout << "Server updated. Client disconnected!" << endl;

   // 10 попыток переконнектиться
   bool bReconnected = false;
   cout << "Will attempt to reconnect in 2 seconds..." << endl;
   Sleep(2000);
   for (int i=0; i<10; i++)
   {
      cout << "Reconnection attemt " << i+1 << endl;

      if (Reconnect() == P_YES)
      {
         bReconnected = true;
         break;
      }

      Sleep(1000);  // Вторая попытка через 1 секунду
   }

   if (bReconnected)
   {
      cout << "Reconnected successfully" << endl;

      // Пытаемся сразу залогиниться
      cout << "Trying to relogin..." << endl;
      if (Relogin() == P_YES)  // Пофиг на возвращаемое значение
         cout << "Login successfull" << endl;
      else
         cout << "Failed to relogin!" << endl << m_err_msg << endl;

      // Получаем сразу версию сервера и выводим на экран
      UINT ver;
      if (GetServerVersion(ver) == P_YES)
         cout << "Server version is " << ver << endl;
      else
         cout << "Error getting server version!" << endl;
   }
   else
      cout << "Failed to reconnect!" << endl;

   return P_YES;
}
