#include <iostream>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "version.h"

using namespace PUSHKIN_SERVER;
using namespace std;

#pragma warning (disable: 4996)

// �������� ����� �� ������
int CPushkinServClient::UploadFile()
{
   // �������� �����������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� �������������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // ��������� ���� � ������ �����, ���� ��� ��������
   UINT path_len;  // ����� ���� � ������ �����
   if (ReceiveUINT(m_socket, path_len) != P_YES) return P_ERROR;

   // �������� ������ ��� �������� ����
   char *path_p = (char*)malloc(path_len+1);
   if (!path_p) return P_ERROR;

   // �������� ���� � ������ �����
   if (ReceiveSTR(m_socket, path_p, path_len) != P_YES)
   {
      free(path_p);
      return P_ERROR;
   }

   path_p[path_len] = 0;  // ������������ ������������� ����

   // �������� ��� ����
   if (::ReceiveFile(m_socket, path_p, 0) != P_YES)
   {
      free(path_p);
      return P_ERROR;
   }

   free(path_p);

   // �������� ������������� ������ ����� �� �������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   return P_YES;  // ������� - ���� ��������
}

// ������ �������� ������ � �������� ���� ��� � ������� ����� �� ������� - ���������� �������
int CPushkinServClient::DirectFileUpload()
{
   // �������� �����������
   if (!m_bAuthorized) return SendReply(m_socket, CC_ERROR, ERR_NOT_AUTHORIZED);

   // �������� �������������
   if (SendReply(m_socket, CC_CONFIRM) != P_YES) return P_ERROR;

   // ����, ������ ����� ����, � ����, ���� ��� ��������
   String src;
   String dest;

   // ��������� ����
   if (ReceiveString(m_socket, src)  != P_YES) return P_ERROR;
   if (ReceiveString(m_socket, dest) != P_YES) return P_ERROR;

   // �������� ���� �� ������ ����� � ������
   if (!CopyFile(src, dest, FALSE))
      return SendReply(m_socket, CC_ERROR, ERR_COPYING_FILE);

   // �������� ������������� ���������� ��������
   return SendReply(m_socket, CC_CONFIRM);
}
