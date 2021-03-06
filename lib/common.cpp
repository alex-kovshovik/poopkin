#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include "commands.h"
#include "common.h"
#include "strlist.h"

using namespace std;

#pragma warning (disable: 4996)

String g_err_msg;  // ����� ��� ��������� �� ������� ��� ���������� �������
int    g_err_code; // ��� �������� ���� ������ ��� ���������� ���������� �������

// ������� ������ �������
int SendReply(SOCKET s, UINT code, UINT err_code)
{
   // �������� ��� �������������
   if (SendUINT(s, code) != P_YES) return P_ERROR;

   // � ��� ������, ���� ������ ����
   if (code == CC_ERROR)
      if (SendUINT(s, err_code) != P_YES) return P_ERROR;

   return P_YES;
}

// ��������� ������������� � ���� ������, ���� ��� �������������
int IsConfirmed(SOCKET s, UINT &err_code)
{
   err_code = 0;  // �� ��������� ������ ���

   UINT code;
   if (ReceiveUINT(s, code) != P_YES) return P_ERROR;

   if (code == CC_CONFIRM) return P_YES;

   // ���� ������ ������ ��� ������, �� �������� ��� �������� ����
   if (ReceiveUINT(s, code) != P_YES) return P_ERROR;
   err_code = code;
   return P_NO; // ������ �� �����������
}

// ������ ������������ ������ � �������
int ReceiveUINT(SOCKET s, UINT &val)
{
   char buf[4];  // ����� ��� �������� ����� � ��������� �����
   
   UINT len;
   UINT len_in = sizeof(val);
   UINT total = 0;  // ����� �������� ����

   while (len_in>0)
   {
      len = recv(s, buf+total, len_in, 0);
      if (len == SOCKET_ERROR) return P_ERROR;

      total += len;
      len_in -= len;
   }

   // ����� ��� ���������, ���� ��������� ��� � �����
   memcpy(&val, buf, sizeof(val));

   return P_YES;
}

// ������ 64-������� ������
int ReceiveINT64(SOCKET s, __int64 &val)
{
   char buf[8];  // ����� ��� �������� ����� � ��������� �����
   
   UINT len;
   UINT len_in = sizeof(val);
   UINT total = 0;  // ����� �������� ����

   while (len_in>0)
   {
      len = recv(s, buf+total, len_in, 0);
      if (len == SOCKET_ERROR) return P_ERROR;

      total += len;
      len_in -= len;
   }

   // ����� ��� ���������, ���� ��������� ��� � �����
   memcpy(&val, buf, sizeof(val));

   return P_YES;
}

// ������ ������ ������ ������������� �������
int ReceiveSTR(SOCKET s, char *buf, int len_in)
{
   if (!buf) return P_ERROR;

   // ����� �������� ������ ���� �� ������� ��� len_in ���� � ������ :)
   UINT len;
   UINT total = 0;  // ����� �������� ����
   while (len_in>0)
   {
      len = recv(s, buf+total, len_in, 0);
      if (len == SOCKET_ERROR) return P_ERROR;

      total += len;
      len_in -= len;
   }

   return P_YES;
}

// �������� ����� �����
int SendUINT(SOCKET s, UINT val)
{
   int len = send(s, (char*)&val, sizeof(val), 0);
   if (len != sizeof(val)) return P_ERROR;

   return P_YES;
}

// �������� ����� 64-������ �����
int SendINT64(SOCKET s, __int64 val)
{
   int len = send(s, (char*)&val, sizeof(val), 0);
   if (len != sizeof(val)) return P_ERROR;

   return P_YES;
}


// �������� ������ ������
int SendSTR(SOCKET s, const char *buf, int len_in)
{
   if (!buf) return P_ERROR;
   int len = send(s, buf, len_in, 0);
   if (len != len_in) return P_ERROR;

   return P_YES;
}

// �������� 8 ���� - ����� �����
int SendFPOST(SOCKET s, fpos_t pos)
{
   int len = send(s, (char*)&pos, sizeof(pos), 0);
   if (len != sizeof(pos)) return P_ERROR;

   return P_YES;
}

// ������� 8 ���� - ����� �����
int ReceiveFPOST(SOCKET s, fpos_t &pos)
{
   int len = recv(s, (char*)&pos, sizeof(pos), 0);
   if (len != sizeof(pos)) return P_ERROR;

   return P_YES;
}


////////////////////////////////////////////////////////////////////////////////////
// �����/�������� ������� �����
////////////////////////////////////////////////////////////////////////////////////
// �������� ������ ����� (������)
int SendStrList(SOCKET s, vector<SFileInfo> &list)
{
   // ����� ������� ���������� ����� ���������� ����� � ������, �����
   // ����� ������ � ������. ����� ������� ������ ������ ����� �������� �������������
   // ��� �����������

   // ����� �������� - ����� ��� �������� ��� �����.
   // ������� ����������� �������� ���� � ������ ��� ���,
   // ����� ��� ��������
   CBinaryBuffer bin;

   // ��������� �������� ����� �������
   bin.AddUINT((UINT)list.size());

   // ���� �� ���� �������
   for (int i=0; i<(int)list.size(); i++)
   {
      UINT len = (UINT)strlen(list[i].name.c_str());
      
      // ���������� �������� ����� � ��� ������
      bin.AddUINT(list[i].attrs);
      bin.AddUINT(list[i].size_high);
      bin.AddUINT(list[i].size_low);
      
      // ���������� ����� ������ � ���� ������
      bin.AddUINT(len);
      bin.AddSTR(list[i].name.c_str(), len);
   }

//   bin.Dump("from_server.bin");

   // �������� �������� �����
   if (bin.Send(s) != P_YES) return P_ERROR;

   return P_YES;
}

// ��������� ������ �����
int ReceiveStrList(SOCKET s, vector<SFileInfo> &list)
{
   // ������� ���������� ������ �����
   list.clear();

   UINT count_in;  // ��� �������� ���������� ����������� �����
   SFileInfo fi;   // ��� �������� ���������� �� ����� �����
   UINT str_len;   // ��� �������� ���� �����
   
   // �������� �����
   UINT attrs;
   UINT size_high;
   UINT size_low;

   CBinaryBuffer bin;

   // ��������� �������� �����
   if (bin.Receive(s) != P_YES) return P_ERROR;

//   bin.Dump("from_client.bin");

   // �����������, ���� �� ��� ��������
   const char *buf_p = bin.GetBuffer();

   // ���������� ������ ������ � ������
   if (bin.GetUINT(buf_p, count_in) != P_YES) return P_ERROR;

   // �������� ������ �� ������
   UINT allocated = 1024;  // ����������� ���������� ������
   char *str_p = (char*)malloc(allocated);
   if (!str_p) return P_ERROR;

   for (int i=0; i<(int)count_in; i++)
   {
      // �������� ���������� � ����� (�������� � ������)
      if (bin.GetUINT(buf_p, attrs) != P_YES) {free(str_p);return P_ERROR;}
      if (bin.GetUINT(buf_p, size_high) != P_YES)  {free(str_p);return P_ERROR;}
      if (bin.GetUINT(buf_p, size_low) != P_YES)  {free(str_p);return P_ERROR;}

      fi.attrs     = attrs;
      fi.size_high = size_high;
      fi.size_low  = size_low;

      // �������� ������ ������
      if (bin.GetUINT(buf_p, str_len) != P_YES) {free(str_p); return P_ERROR;}

      // ���� ������ ������������ ������ ������, ��� ������ ������ - ���������� ��� ������
      if (str_len+1 > allocated)
      {
         str_p = (char*)realloc(str_p, str_len+1);
         allocated = str_len+1;
         if (!str_p) return P_ERROR;
      }
   
      // �������� ���� ������
      if (bin.GetSTR(buf_p, str_p, str_len) != P_YES)  { free(str_p); return P_ERROR; }
      str_p[str_len] = 0;  // �� �������� ������������� ����!
      
      // ���������� ��� � �������
      fi.name = str_p;
      list.push_back(fi);
   }

   free(str_p);

   return P_YES;
}


////////////////////////////////////////////////////////////////////////////////////////////////
// �������� � ��������� ������
////////////////////////////////////////////////////////////////////////////////////////////////

// ������������ ������ ������ ��� �������/������ ������ (�������� ����)
#define SF_MAX_BUF_SIZE 1024*1024

// ��������� ����
int SendFile(SOCKET s, const char *fpath, __int64 off)
{
   // ������� �������� ������������� ������� �����   
   // ����� �������� ������ ����� - ������ 4 �����, ����� ��� ����
   if (!fpath)  // ��� ������ ������������� ������� �������
   {
      SendReply(s, CC_ERROR, ERR_FPATH_PARAM_NULL);
      return P_ERROR;  // ��� ������ - ��������� �������
   }

   // ������ ����� ��������� ���� ��� ������ API �����
   HANDLE hFile = CreateFile(fpath, FILE_READ_DATA|FILE_READ_ATTRIBUTES,FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
      return SendReply(s, CC_ERROR, ERR_OPENING_FILE);  // !!! - ��� ���������� ��������, ������ �� ��� ��������� ������� - !!!

   // ���������� ������ �����
   DWORD fSizeHigh = 0;
   DWORD fSizeLow = GetFileSize(hFile, &fSizeHigh);

   // ���� ����� ������������� �� �����, �� ������ ���
   LARGE_INTEGER li;
   li.QuadPart = (LONGLONG)off;

   if (off != 0)
      if (!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN));  // ���� �� ������ ������ �� ����� ������
   
   // ������ ����� �������� ������ ������� ���������� ������ (���� ��� �� ���)
   __int64 fSize = fSizeHigh;
   fSize <<=32;
   fSize |= fSizeLow;

//   fSize = fSize - off + 1;

   char *buf = (char*)malloc(SF_MAX_BUF_SIZE);
   if (!buf)
   {
      CloseHandle(hFile);
      return SendReply(s, CC_ERROR, ERR_OUT_OF_MEMORY);
   }

   // �������� ������������� ����, ��� �� �� ����� ����� ����
   if (SendReply(s, CC_CONFIRM) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}

   // �������� ������ ����� - ������ __int64
   if (SendINT64(s, fSize) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}

   DWORD bytes_read = 0;
   
   // �������� ���� �������
   while (ReadFile(hFile, buf, SF_MAX_BUF_SIZE, &bytes_read, NULL))
   {
      // �������� ������ �����
      if (SendUINT(s, (UINT)bytes_read) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}

      // ���� �� ��������� ������� ����, �� ��� ����� �����
      if (bytes_read == 0) break;

      // �������� ��� ����
      if (SendSTR(s, buf, (UINT)bytes_read) != P_YES) {free(buf); CloseHandle(hFile); return P_ERROR;}
   }

   free(buf);
   CloseHandle(hFile);

   // ����������� ������� ���� ��� �������

   return P_YES;
}

// ������� ����
int ReceiveFile(SOCKET s, const char *fpath_dest, int b_update, int (*BlockReadCallback)(DWORD))
{
   // b_update - ������� ����, ��� ���� ����� ��������
   g_err_msg = "";
   g_err_code = 0;

   bool b_fopen_error = false;

   if (!fpath_dest)  // ��� ���� ����� ������������
   {
      g_err_msg = "������� �������� fpath_dest � ������� ������ �����";
      return P_ERROR;
   }

   __int64 file_size; // ������ ����� (��������� ��������)
   UINT block_size;   // ������ ����� ������
   UINT total_size=0; // ������ ������� ���������� ������

   // �������� ������������� ������ �����
   UINT err_code;
   int ret = IsConfirmed(s, err_code);
   if (ret == P_ERROR) return P_ERROR;
   if (ret == P_NO)
   {
      g_err_code = err_code;
      g_err_msg = GetErrorText(err_code);
      return P_ERROR;
   }

   // �������� ������ �����
   if (ReceiveINT64(s, file_size) != P_YES)
   {
      g_err_msg = "������ ��������� ������� �����";
      return P_ERROR;
   }

   // ���� ����� ����� ��������� ������
   const char *mode;
   if (b_update) mode = "r+b";
   else mode = "wb";

   HANDLE hFile = CreateFile(fpath_dest, FILE_READ_DATA|FILE_READ_ATTRIBUTES|FILE_APPEND_DATA,FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
   {
      g_err_msg = "������ �������� ����� ��� ������";
      b_fopen_error = true;

      LPVOID lpMsgBuf;
      if (!FormatMessage( 
         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
         FORMAT_MESSAGE_FROM_SYSTEM | 
         FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         GetLastError(),
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
         (LPTSTR) &lpMsgBuf,
         0,
         NULL ))
      {
         // Handle the error.
         return 0;
      }

      // Display the string.
      MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );

      // Free the buffer.
      LocalFree( lpMsgBuf );
   }
   /*
   FILE *pFile = fopen(fpath_dest, "wb");
   if (!pFile)  // � ��� ��� ��������� ����� � ����� ������, �������� ���� ���������� ����� ������
   {
      // �. �., �������� �� ����� �� �����, �� � ���� ������ ��������� ����
      g_err_msg = "������ �������� ����� ��� ������";
      b_fopen_error = true;
   }
*/
   // ���� ���� ����� ��������, �� ������������ � ����� �����
   if (b_update)
   {
//      fseek(pFile, 0, SEEK_END);
      SetFilePointer(hFile, 0, NULL, FILE_END);
   }

   // ����� � ������� �������� ���� ������!!!
   if (file_size == 0)
   {
      // ��������� ����      
      if (hFile) {CloseHandle(hFile);hFile=NULL;}
      
      // �������� ��������� ������� ���� ������
      if (ReceiveUINT(s, block_size) != P_YES)
      {
         g_err_msg = "������ ��������� ������������ ����� ��� ����� � ������� ��������";
         return P_ERROR;
      }

      // ��������� ������������ �������� �� ������������
      if (block_size != 0)
      {
         g_err_msg = "��������� ����������� ���� ������� �����";
         return P_ERROR;
      }

      // �������, ���� �������. � ���� �� ���� �� ������, �� ����� ������ ����� �������������
      if (b_fopen_error) return P_ERROR;  // ������ �������� ����� � ������� �������
      else return P_YES;  // ��� ��������� - ���� ����� ������� �����
   }

   // ��� ��, �������, ����� ������ �������� ����� ������������ �����, �. �. ���� ����� ������ �����
   char *buf = (char*)malloc(SF_MAX_BUF_SIZE);
   if (!buf) {if (hFile) CloseHandle(hFile); return P_ERROR;}  // �� ���������� ������

   while (1)
   {
      // �������� ������ �����
      if (ReceiveUINT(s, block_size) != P_YES)
      {
         free(buf);
         if (hFile) {CloseHandle(hFile); hFile=NULL;}
         g_err_msg = "������ ��������� ������� ����� ������";
         return P_ERROR;
      }

      if (block_size == 0) break;  // ��� ����������� ������ ����

      // �������� ��� ����
      if (ReceiveSTR(s, buf, block_size) != P_YES)
      {
         free(buf);
         if (hFile) {CloseHandle(hFile); hFile=NULL;}
         g_err_msg = "������ ��������� ����� ������";
         return P_ERROR;
      }

      // ����� ���� � ����
      DWORD bytes_written = 0;
      if (hFile) WriteFile(hFile, buf, block_size, &bytes_written, NULL);

      // �������� ������� ��� ������������ �������� �����������
      if (BlockReadCallback) 
         if (!BlockReadCallback(block_size))
         {
            // �. �. ��� ����� ������ � ����������� ������, �� ����� ������ ������ ����� �� ����������,
            // ������������� ������ ��� ���������� �������� ������ � ������� ����������
            free(buf);
            if (hFile) {CloseHandle(hFile); hFile=NULL;}
            return P_NO;
         }

      total_size += block_size;
   }

   free(buf);
   if (hFile) {CloseHandle(hFile); hFile=NULL;}

   if (total_size != file_size)
   {
      g_err_msg = "���� ������� �� ���������";
      return P_ERROR;
   }

   if (b_fopen_error) return P_ERROR;  // �� ������ �� �������� � ����, �. �. ��������� ������

   // �� ���� ��������� ������� - ��� � �������
   return P_YES;
}

// ��������� � ������� ����� ����� PSQL_API::String
int ReceiveString(SOCKET s, String &str_out)
{
   // ������� �������� ����� ������
   UINT len;
   if (ReceiveUINT(s, len) != P_YES) return P_ERROR;

   // ���� ������ ������, �� ������ ������ �� ���������, ������ �������� ���������� ������
   if (len == 0)
   {
      str_out = "";
      return P_YES;
   }

   // �������� ������ ��� �������� ���� ������
   char *buf = (char*)malloc(len+1);
   if (!buf) return P_ERROR;  // �� ���������� ������

   // �������� ���� ������
   if (ReceiveSTR(s, buf, len) != P_YES) return P_ERROR;

   // ������ ���� ������������� ����
   buf[len] = 0;

   // ���������� ������ � ���������� ����������
   str_out = buf;

   // ����������� ������ ��� ������
   free(buf);
   return P_YES;
}

int SendString(SOCKET s, String &str_in)
{
   // ������� �������� ����� ������
   UINT len = (UINT)strlen(str_in);

   // �������� ����� ������������ ������
   if (SendUINT(s, len) != P_YES) return P_ERROR;

   // ���� ����� �������, �� �� ���� ���� ������
   if (len == 0) return P_YES;
   
   // �������� ���� ������
   if (SendSTR(s, str_in.c_str(), len) != P_YES) return P_ERROR;

   return P_YES;
}
