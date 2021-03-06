// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// ����� ��� ������� �����
#define SEND_FILE_BUFFER_SIZE 16384

// ���������� ������� �� ����� ����� ������
int CPushkinClient::UpdateServer(const char *new_serv_name)
{
   // ������� ���������� � �������, ����� �� �� �������� ��� �� ����� ������
   if (SendCommand(SC_UPDATE_SERVER) != P_YES) return P_ERROR;

   // ��������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error updating server. Error code=" << err_code << endl;
      return P_ERROR;
   }

   // ����� ������������� �������, �������� ����� ����
   if (::SendFile(m_socket, new_serv_name, 0) != P_YES) return P_ERROR;

   // �������� ������������� � ������� �� �������� ������ �����
   if (IsConfirmed(m_socket, err_code) != P_YES) return P_ERROR;

   // ����������� �� ������� � ���� ���� �� ��������������
   Disconnect();

   cout << "Server updated. Client disconnected!" << endl;

   // 10 ������� ����������������
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

      Sleep(1000);  // ������ ������� ����� 1 �������
   }

   if (bReconnected)
   {
      cout << "Reconnected successfully" << endl;

      // �������� ����� ������������
      cout << "Trying to relogin..." << endl;
      if (Relogin() == P_YES)  // ����� �� ������������ ��������
         cout << "Login successfull" << endl;
      else
         cout << "Failed to relogin!" << endl << m_err_msg << endl;

      // �������� ����� ������ ������� � ������� �� �����
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
