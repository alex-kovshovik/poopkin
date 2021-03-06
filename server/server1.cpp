#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "version.h"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)
////////////////////////////////////////////////////////////////////////////////
// ��������� �������
////////////////////////////////////////////////////////////////////////////////

// �������� ����������� �������
int CPushkinServClient::SendGreeting()
{
   const char *greet_msg = "Fuck off!!!";

   if (send(m_socket, greet_msg, (int)strlen(greet_msg), 0) == SOCKET_ERROR)
      return P_ERROR;

   return P_YES;
}


// ��������� ������� �� �������
int CPushkinServClient::GetCommand(UINT &command)
{
   int len = recv(m_socket, (char*)&command, sizeof(UINT), 0);
   if (len == SOCKET_ERROR)
      return P_ERROR;

   if (len != sizeof(UINT))
      return P_ERROR;

   return P_YES;
}

// ������� ������ ������ �������
int CPushkinServClient::SendVersion()
{
   // �� ������� �����������!!!
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ����� ������
   if (SendUINT(m_socket, SERVER_VERSION) != P_YES) return P_ERROR;

   return P_YES;
}

// ����� ��������� �� ������
int CPushkinServClient::ShowMessage()
{
   // �������� ��������� ��� ��������� ��������� (����� ��������� � ���� ���������)
   String message;
   String caption;
   UINT flags;
   UINT b_async;  // ������� ������������� ���������

   // ��������� �����������
   if (!m_bAuthorized)
      return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);
   else
      SendReply(m_socket, CC_CONFIRM);  // ���� ������������� ������ ������

   if (ReceiveString(m_socket, message) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, caption) != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, flags) != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, b_async) != P_YES) return P_ERROR;

   // ���������� ��������� � �������� �����
   // (���� ����� ���������� ������ ���������� ���������)
   MessageBox(GetDesktopWindow(), message, caption, flags);

   return SendReply(m_socket, CC_CONFIRM); // ������������ ����� ���������
}

#pragma comment(lib, "Winmm.lib")

// �������� ������ CD-ROM'a
int CPushkinServClient::OpenCDDoor()
{
   UINT b_open;
   
   // �������� ������� ��������/�������� CD-ROM'a
   if (ReceiveUINT(m_socket, b_open) != P_YES) return P_ERROR;

   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // ��������� ��� ��������� CD-ROM
   if (b_open)
      mciSendString("Set cdaudio door open wait",0,0,0);
   else
      mciSendString("Set cdaudio door closed wait",0,0,0);

   // �������� �������������
   return SendReply(m_socket, CC_CONFIRM);
}

// ������ ��������� �� ����������
int CPushkinServClient::ServCreateProcess()
{
   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� �������������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ��������� ������� ��������
   String cmd_line;
   String working_dir;

   // �������� ��������� ������ � ������� �������
   if (ReceiveString(m_socket, cmd_line) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, working_dir) != P_YES) return P_ERROR;

   // ��������� �������
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // ��������� ��������� ���������� �������
   if( !CreateProcess( NULL, (LPSTR)cmd_line.c_str(), // Command line.
                        NULL,             // Process handle not inheritable.
                        NULL,             // Thread handle not inheritable.
                        FALSE,            // Set handle inheritance to FALSE.
                        0,                // No creation flags.
                        NULL,             // Use parent's environment block.
                        (LPSTR)working_dir.c_str(),      // ������� �������
                        &si,              // Pointer to STARTUPINFO structure.
                        &pi ) )           // Pointer to PROCESS_INFORMATION structure.
      return SendReply(m_socket, CC_ERROR, ERR_CREATING_PROCESS);

   // �������� ������������� ����, ��� ������� �������
   return SendReply(m_socket, CC_CONFIRM);
}
// ������ ������-���� �����, ��������, �������� ��� MP3
#pragma warning(disable: 4311)
int CPushkinServClient::ServShellExecute()
{
   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� ������������� ���������� ������� ���������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ��������� � �������
   String operation;
   String file;
   String dir;

   if (ReceiveString(m_socket, operation) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, file) != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, dir) != P_YES) return P_ERROR;

   // ��������� ������� ShellExecute
   if ((int)ShellExecute(NULL, operation, file, NULL, dir, SW_RESTORE) <= 32)
      return SendReply(m_socket, CC_ERROR, ERR_SHELL_EXECUTE);

   return SendReply(m_socket, CC_CONFIRM);
}

// ������� ���� ������������ �� Winamp'e
int CPushkinServClient::MaximizeWinampVolume()
{
   HWND hWinamp = FindWindow("Winamp v1.x", NULL);

   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   if (!hWinamp) return SendReply(m_socket, CC_ERROR, ERR_WINAMP_NOT_FOUND);

   for (int i=0; i<100; i++)
      SendMessage(hWinamp, WM_KEYDOWN, 38, 1);

   return SendReply(m_socket, CC_CONFIRM);
}

// ��������� ������ ���������� �����
int CPushkinServClient::SendFreeSpace()
{
   // �������� ��������� �����
   String path;  // ���� � �����, �� ������� ����� ���������� ��������� �����
   DWORD sec_per_clust;
   DWORD bytes_per_sect;
   DWORD num_of_free_clust;
   DWORD tot_num_of_clust;

   // ��������� ���������������� ������������
   if (!m_bAuthorized)
      return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� ������������� ���������� ������� ���������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ���������
   if (ReceiveString(m_socket, path) != P_YES)
      return P_ERROR;

   if (!GetDiskFreeSpace(path, &sec_per_clust, &bytes_per_sect, &num_of_free_clust, &tot_num_of_clust))
      return SendReply(m_socket, CC_ERROR, ERR_GETDISTFREESPACE);

   // �������� ������������� ������� ���������� �����
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ��������� ����� ������� �������� �� 4 �����
   if (SendUINT(m_socket, sec_per_clust)     != P_YES) return P_ERROR;
   if (SendUINT(m_socket, bytes_per_sect)    != P_YES) return P_ERROR;
   if (SendUINT(m_socket, num_of_free_clust) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, tot_num_of_clust)  != P_YES) return P_ERROR;

   return P_YES;
}

// �������� ����� �� �������
int CPushkinServClient::DeleteFile()
{
   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� ������������� ���������� ������� ���������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // ��������� ��������
   String fname;
   if (ReceiveString(m_socket, fname) != P_YES) return P_ERROR;

   // ������� ����
   if (!::DeleteFile(fname)) return SendReply(m_socket, CC_ERROR, ERR_DELETING_FILE);

   return SendReply(m_socket, CC_CONFIRM);
}
