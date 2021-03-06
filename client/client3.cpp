// ���������� ������ CPushkinClient
#include <iostream>
#include "client.h"
#include "commands.h"
#include "common.h"

#pragma warning (disable: 4996)

using namespace PUSHKIN_CLIENT;
using namespace std;

extern String g_err_msg;

// ��������� ����� � �������
int CPushkinClient::GetFile(const char *fname_src, const char *fname_dest)
{
   m_err_code = 0;
   if (!fname_src)
   {
      m_err_msg = "������� �������� fname_src � ������� ��������� ����� � �������";
      return P_ERROR;
   }

   // �������� ������� �� ��������� �����
   if (SendCommand(SC_GET_FILE) != P_YES) return P_ERROR;  // ��������� �� ������ ��� ���������

   // � ����� �������� ��������� ��������� �����
   // (����� ����� - ����� ���� � �����)
   if (SendString(m_socket, (String)fname_src) != P_YES)
   {
      m_err_msg = "������ ������� ���� � ����� ��� �������� � �������";
      return P_ERROR;
   }

   int ret = ReceiveFile(m_socket, fname_dest, 0, BlockReadCallback);
   if (ret != P_YES)
   {
      m_err_msg = g_err_msg;
      return ret;
   }
   return P_YES;
}

// ��������� ������ �������
int CPushkinClient::GetFilesList(const char *add_s, vector<SFileInfo> &list_in, const char *dest_path)
{
   // dest_path - ����, ���� ���������� �����
   m_err_code = 0;

   // ���� ������� �������
   if (SendCommand(SC_GET_FILES_LIST) != P_YES) return P_ERROR;  // ��������� �� ������ ��� ���������

   // ��������� ����� 
   UINT err_code;
   int ret = IsConfirmed(m_socket, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      m_err_code = err_code;
      m_err_msg = "������ ��������� ������ ������. ";
      m_err_msg += GetErrorText(err_code);
      return P_ERROR;
   }

   // ���� ��������� ��������� ������
   if (SendString(m_socket, (String)add_s) != P_YES)
   {
      m_err_msg = "������ ������� ���������� �������";
      return P_ERROR;
   }

   if (SendStrList(m_socket, list_in) != P_YES)
   {
      m_err_msg = "������ ������� ���������� �������";
      return P_ERROR;
   }

   String fname;       // ��� �����, �������� � �������
   String fname_res;   // ������ �������������� ��� �����

   bool b_skip_err_files = false; // ������� ����, ��� ����� � �������� ����� ����������

   // �������� ����� �� ������
   while (1)
   {
      // �������� ��� ������������ �����
      if (ReceiveString(m_socket, fname) != P_YES)
      {
         m_err_msg = "������ ��������� ����� ������������ �����";
         return P_ERROR;
      }

      // ���� ��� - ������ ������, �� ��� ����� ������ ����������� ������
      if (fname == "") break;

      fname_res = dest_path;
      fname_res += fname;

      // �������� ����������� ��������� ���������� � ��������� �����
      if (ReceivingFileCallback)
         if (!ReceivingFileCallback(fname_res))
            return P_NO;  // ������� ����������� �������

      // ��������� ��� ����
      ret = ::ReceiveFile(m_socket, fname_res, 0, BlockReadCallback);
      if (ret == P_NO) return P_NO;  // ������� ����������� �������
      if (ret == P_ERROR)
      {
         m_err_msg = g_err_msg;

         // ���� � ������� ������ ��� ���������� ����� � ����� ������ ��� ������
         for (int i=0; i<(int)list_in.size();i++)
         {
            if (list_in[i].name == fname)
            {
               if (BlockReadCallback)
                  if (!BlockReadCallback(list_in[i].size_low)) return P_NO;  // ���������� �������� �����������
               break;
            }
         }

         // ���� �� ����� ���������� ��� ����� � ��������, �� ������ ���������� ��
         if (b_skip_err_files) continue;

         // ������ ����������� ������-�� ����� - �� ���� �� ��������
         if (ErrReceivingFileCallback)
            ret = ErrReceivingFileCallback(m_err_msg, fname_res);
         else
            ret = 2;  // ���� ���������� �� �����, �� ����� ����� ����� ������ ����������

         if (ret == 2) b_skip_err_files = true;

         if (ret == 0) return P_NO;  // ���������� �������� ����������� �������������
      }
   }

   return P_YES;
}
