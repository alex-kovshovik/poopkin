// ������� ������
#include <iostream>
#include <vector>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "pstring.hpp"

using namespace PUSHKIN_SERVER;
using namespace std;
using namespace PSQL_API;

#pragma warning (disable: 4996)

// ������� �����
int CPushkinServClient::SendFile()
{
   // �������� ������� ��� ������ ��� �������� ����� � �������, ����� ��������� �����������
   String fpath;
   if (ReceiveString(m_socket, fpath) != P_YES) return P_ERROR;  // ��� �������� ������, ��������� �������

   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   return ::SendFile(m_socket, fpath, 0);
}

// ��������� ������ �� ����������� ������
int CPushkinServClient::SendFilesList()
{
   // ��������� �����������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // ���� ������������� ���������� �������� ������ ������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ��������� ������� (������� � ������ ���� ������� ����� � ��� ������ ����� � ������)
   String add_s;
   vector<SFileInfo> list;

   if (ReceiveString(m_socket, add_s) != P_YES) return P_ERROR;
   if (ReceiveStrList(m_socket, list) != P_YES) return P_ERROR;

   // ���� �� ����������� ������ � �������� ����� �� ������
   for (int i=0; i<(int)list.size(); i++)
   {
      // ���������� ���������� - �� �� �������� �� �����
      if (list[i].attrs & FILE_ATTRIBUTE_DIRECTORY) continue;

      // �������� ��� ������������� ����� �������
      if (SendString(m_socket, list[i].name) != P_YES) return P_ERROR;

      // ����� ���� ��� ����
      String path = add_s + list[i].name;
      if (::SendFile(m_socket, path, 0) != P_YES) return P_ERROR;
   }

   // � ����� ������������� �������, ��� ������ ������ �� ����� - ���� ������ ������
   if (SendString(m_socket, (String)"") != P_YES) return P_ERROR;

   return P_YES;
}

// ��������� ����� � ��������
int CPushkinServClient::GetFileEx()
{
   // ��������� ���������������� ������������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� ������������� ���������� ������� ���������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // �������� ��� ������ ��� �������� ����� � �������
   String fpath;
   if (ReceiveString(m_socket, fpath) != P_YES) return P_ERROR;

   // �������� ����� �����, � �������� ����� ���������� ����
   __int64 off;  // �������� ������������ ������ �����
   if (ReceiveINT64(m_socket, off) != P_YES) return P_ERROR;

   return ::SendFile(m_socket, fpath, off);
}
