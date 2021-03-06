// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "pint.hpp"
#include "shlwapi.h"

#pragma comment(lib, "shlwapi.lib")
#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// �������� ���� �� ������
int CPushkinClient::SendFile(const char *path, const char *dest_path)
{
   if (!path || !dest_path) return P_ERROR;

   // ��������� ���� �� �������������
   if (!PathFileExists(path)) return P_ERROR;  // ���� �� ����������

   // �������� ������ �� �������� ����� �� ������
   if (SendCommand(SC_UPLOAD_FILE) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error sending file. Error code=" << err_code << endl;
      return P_ERROR;
   }

   // �������� ����� ���� � ����� � ��� ���� ����� �� �������
   UINT path_len = (UINT)strlen(dest_path);
   if (SendUINT(m_socket, path_len) != P_YES) return P_ERROR;
   if (SendSTR(m_socket, dest_path, path_len) != P_YES) return P_ERROR;

   // �������� ��� ����
   if (::SendFile(m_socket, path, 0) != P_YES) return P_ERROR;

   // �������� ������������� ������ �����
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error sending file2. Error code=" << err_code << endl;
      return P_ERROR;
   }

   return P_YES;  // ���� �������� �� ������
}

// ������ ����������� ������ - ����� �������
int CPushkinClient::SendFileDirect(const char *src, const char *dest)
{
   m_err_code = 0;
   m_err_msg = "";

   if (!src || !dest) return P_ERROR;

   // �������� ������ �� �������� ����� �� ������
   if (SendCommand(SC_DIRECT_UPLOAD) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "������ ����������� �����. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // �������� ���� �� ������
   if (SendString(m_socket, (String)src)  != P_YES) return P_ERROR;
   if (SendString(m_socket, (String)dest) != P_YES) return P_ERROR;

   // �������� ������������� ����������� �����
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg  = "������ ����������� �����. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   return P_YES;
}
