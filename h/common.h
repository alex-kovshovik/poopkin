#pragma once

#ifndef __PSTRING_HPP__
#include "pstring.hpp"
#endif

#ifndef P_YES
   #define P_YES     0
   #define P_NO      1
   #define P_ERROR  -1
#endif

// ����� ������� ��� ��������/������ ������
int SendReply(SOCKET s, UINT code, UINT err_code=0);  // ������� ������ �������
int IsConfirmed(SOCKET s, UINT &err_code);            // ��������� ������������� � ���� ������, ���� ��� �������������

int ReceiveUINT(SOCKET s, UINT &val);                 // ������ ������������ ������ � �������
int ReceiveINT64(SOCKET s, __int64 &val);             // ������ 64-������� ������
int ReceiveSTR(SOCKET s, char *buf, int len);         // ������ ������ ������ ������������� �������
int SendUINT(SOCKET s, UINT val);                     // �������� ����� �����
int SendINT64(SOCKET s, __int64 val);                  // �������� ����� 64-������ �����
int SendSTR(SOCKET s, const char *buf, int len);      // �������� ������ ������

int SendFPOST(SOCKET s, fpos_t pos);                  // �������� 8 ���� - ����� �����
int ReceiveFPOST(SOCKET s, fpos_t &pos);              // ������� 8 ���� - ����� �����

// �������� � ��������� ������
int SendFile(SOCKET s, const char *fpath, __int64 off);   // ��������� ����
int ReceiveFile(SOCKET s, const char *fpath, int b_update, int (*BlockReadCallback)(DWORD)=NULL);         // ������� ����

// �������� � ��������� ����� � ���������� PSQL_API::String
int ReceiveString(SOCKET s, PSQL_API::String &str_out);
int SendString(SOCKET s, PSQL_API::String &str_in);
