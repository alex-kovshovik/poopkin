// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "pint.hpp"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

// ��������� ������ ������ � �������
int CPushkinClient::GetFileList(const char *path, vector<SFileInfo> &list)
{
   m_err_msg = "������ ��������� ������ ������";

   // �������� ������ �������
   if (SendCommand(SC_GET_FILE_LIST) != P_YES) return P_ERROR;

   // �������� ����� ������ � �����
   UINT path_len = (UINT)strlen(path);
   if (SendUINT(m_socket, path_len) != P_YES) return P_ERROR;

   // �������� ���� � ������ � ������
   if (SendSTR(m_socket, path, (int)strlen(path)) != P_YES) return P_ERROR;

   // �������� �������������
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      cout << "Error getting file list. Error code=" << err_code << endl;
      return P_ERROR;
   }

   // �������� ��� ������ ������
   if (ReceiveStrList(m_socket, list) != P_YES)
   {
      m_err_msg = "������ ��������� ������� ����� � �������";
      return P_ERROR;
   }

   return P_YES;
}

// ��������� ������ ���������� ������
int CPushkinClient::GetDrivesList(UINT &drives)
{
   // �������� ������ �� ��������� ������ ���������� ������
   if (SendCommand(SC_GET_DRIVES_LIST) != P_YES) return P_ERROR;

   // ��������� �������������
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_msg = "������ ��������� ������ ���������� ������. ��� ";
      m_err_msg += (Int)err_code;
      return P_ERROR;
   }

   // �������� ��� ������
   return ReceiveUINT(m_socket, drives);
}

// ��������� ������ ���������
int CPushkinClient::GetProcessList(std::vector<SFileInfo> &list)
{
   // �������� ������ �� ��������� ������ ���������
   if (SendCommand(SC_GET_PROCESS_LIST) != P_YES) return P_ERROR;

   // ��������� �������������1
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_msg = "������ ��������� ������ ���������. ��� ";
      m_err_msg += (Int)err_code;
      return P_ERROR;
   }

   // �������� ��� ������
   if (ReceiveStrList(m_socket, list) != P_YES)
   {
      m_err_msg = "������ ��������� ������� ����� � �������";
      return P_ERROR;
   }

   return P_YES;
}

// ��������� ������ ������ � ���������
int CPushkinClient::GetFilesTree(const char *path, const char *add_s, std::vector<SFileInfo> &list, __int64 &total_size)
{
   // add_s - ��, ��� ���� �������� � ������ ������ ������������ ������
   m_err_code = 0;

   // �������� ������ �� ��������� ������ ���������
   if (SendCommand(SC_GET_FILES_TREE) != P_YES) return P_ERROR;

   // ��������� �������������
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg = "������ ��������� ������ ������ � ���������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // ���� ��������� �������
   if (SendString(m_socket, (String)path) != P_YES)
   {
      m_err_msg = "������ ��������� ������ ������ � ���������. ������ ��� ������� ���������� ����";
      return P_ERROR;
   }

   // ���� ��, ��� ����� �������� � ������ ������ ������������ ������
   if (SendString(m_socket, (String)add_s) != P_YES)
   {
      m_err_msg = "������ ��������� ������ ������ � ���������. ������ ��� ������� ������� � ����";
      return P_ERROR;
   }

   vector<SFileInfo> temp_list;

   // �������� ��� ������
   if (ReceiveStrList(m_socket, temp_list) != P_YES)
   {
      m_err_msg = "������ ��������� ������� ����� � �������";
      return P_ERROR;
   }

   // ��������� ��� ���������� �� �������� ������
   for (int i=0; i<(int)temp_list.size(); i++)
      list.push_back(temp_list[i]);

   // �������� ������ ����� ���� ������
   if (ReceiveINT64(m_socket, total_size) != P_YES)
   {
      m_err_msg = "������ ��������� ������� ������ ���� ������";
      return P_ERROR;
   }

   return P_YES;
}

// ������� �����
int CPushkinClient::GetFileEx(const char *src, const char *dest)
{
   m_err_code = 0;
   if (!src)
   {
      m_err_msg = "������� �������� src � ������� GetFileEx";
      return P_ERROR;
   }

   // ����������� ������ �����
   HANDLE hFile = CreateFile(dest, FILE_READ_DATA|FILE_READ_ATTRIBUTES,FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
      return P_ERROR;

   DWORD fSizeHigh = 0;
   DWORD fSizeLow = GetFileSize(hFile, &fSizeHigh);
   __int64 fSize = fSizeHigh;
   fSize <<=32;
   fSize |= fSizeLow;

   // �������� ������� �� ��������� �����
   if (SendCommand(SC_GET_FILE_EX) != P_YES) return P_ERROR;  // ��������� �� ������ ��� ���������

   // ��������� �������������
   int ret;
   UINT err_code;
   ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg = "������ ��������� ����� GetFileEx. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // � ����� �������� ��������� ��������� �����
   if (SendString(m_socket, (String)src) != P_YES)
   {
      m_err_msg = "������ ������� ���� � ����� ��� �������� � �������";
      return P_ERROR;
   }

   // �������� �� ������
   if (SendINT64(m_socket, fSize+1) != P_YES) return P_ERROR;

   ret = ReceiveFile(m_socket, dest, 1, BlockReadCallback);
   if (ret != P_YES)
   {
//      m_err_msg = g_err_msg;
      return ret;
   }
   return P_YES;
}
