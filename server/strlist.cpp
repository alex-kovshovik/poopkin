#include <winsock2.h>
#include <stdio.h>
#include "commands.h"
#include "common.h"
#include "strlist.h"

// ���� ������ ��� ������� ����� - ��� ���������, �� ����� ������! :)
#define RECV_BLOCK_SIZE 1024*1024   // ���� �������� - ���� �� :)

///////////////////////////////////////////////////////////////////////////////////////////////////
// ���������� ������ CBinaryBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
CBinaryBuffer::CBinaryBuffer()
{
   buf = NULL;
   m_allocated = 0;
   m_size = 0;
}

CBinaryBuffer::~CBinaryBuffer()
{
   Reset();
}

void CBinaryBuffer::Allocate(size_t new_size)
{
   if (new_size <= m_allocated) return;

   buf = (char*)realloc(buf, new_size);

   m_allocated = new_size;
}

void CBinaryBuffer::Reset()
{
   if (buf) {free(buf); buf = NULL;}
   m_allocated = 0;
   m_size = 0;
}

void CBinaryBuffer::AddUINT(UINT val)
{
   if (m_allocated - m_size < sizeof(val)) Allocate(m_allocated<RECV_BLOCK_SIZE?m_allocated+RECV_BLOCK_SIZE:m_allocated*2);

   // ������ �������� - �������� ������
   memcpy(buf+m_size, &val, sizeof(val));
   m_size += sizeof(val);
}
void CBinaryBuffer::AddSTR(const char *val, size_t len)
{
   if (!val) return;
   if (len == 0) return;

   if (m_allocated - m_size < len) Allocate(m_allocated<len?m_allocated+len:m_allocated*2);

   // ������ �������� - �������� ������
   memcpy(buf+m_size, val, len);
   m_size += len;
}
void CBinaryBuffer::AddFPOST(fpos_t val)
{
   if (m_allocated - m_size < sizeof(val)) Allocate(m_allocated<RECV_BLOCK_SIZE?m_allocated+RECV_BLOCK_SIZE:m_allocated*2);

   // ������ �������� - �������� ������
   memcpy(buf+m_size, &val, sizeof(val));
   m_size += sizeof(val);
}

int CBinaryBuffer::GetUINT(const char *&buf_p, UINT &val)
{
   // ���������, ���������� �� ������ � ��� � ������
   if (buf+m_size-buf_p < sizeof(val)) return P_NO;

   UINT *val_p = (UINT*)buf_p;
   val = *val_p;
   buf_p += sizeof(val);

   return P_YES;
}

int CBinaryBuffer::GetSTR(const char *&buf_p, char *buf, size_t len)
{
   // ���������, ���������� �� ������ � ��� � ������
   if (size_t(buf+m_size-buf_p) < len) return P_NO;

   memcpy(buf, buf_p, len);
   buf_p += len;

   return P_YES;
}

int CBinaryBuffer::GetFPOST(const char *&buf_p, fpos_t &val)
{
   // ���������, ���������� �� ������ � ��� � ������
   if (buf+m_size-buf_p < sizeof(val)) return P_NO;

   fpos_t *val_p = (fpos_t*)buf_p;
   val = *val_p;
   buf_p += sizeof(val);

   return P_YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// �������� � ����� ��������� ������
///////////////////////////////////////////////////////////////////////////////////////////////////
int CBinaryBuffer::Send(SOCKET s)
{
   // "����������" � ������� ������ ����������� �����
   UINT block_size;  // ������ ����������� �����
   UINT sent_size;   // ������ ������� ����������� �����
   const char *buf_p = buf;  // ����������� ��������� �� �����
   
   // ���� ������� ������ - ���� ������ �� ��������� ����� � ���� ����
   while (1)
   {
      // ������ �� ��������� ����� ������������� �������
      if (ReceiveUINT(s, block_size) != P_YES) return P_ERROR;

      if (block_size == 0) break;  // ������ ������� ������� ������

      if (SendBlock(s, buf_p, block_size, sent_size) != P_YES) return P_ERROR;

      if (sent_size == 0) break;   // ��� �������� ��� ������
   }

   return P_YES;
}

// ������� ����� ������
int CBinaryBuffer::SendBlock(SOCKET s, const char *&buf_p, UINT size, UINT &sent_size)
{
   // buf_p - ��������� �� ������ ������
   // size - ������ ���
   // sent_size - ������ ������� ���������� ������

   sent_size = 0;
   if (buf_p >= buf+m_size) return SendUINT(s, 0);

   // ���� ��������� ������ ����� ��������� ���������� ���������� ������, �� �������� ����
   if ( size > UINT((buf+m_size)-buf_p) )
      sent_size = (UINT)((buf+m_size)-buf_p);
   else
      sent_size = size;

   // �������� ������ ���������� ������ � ���� ������
   if (SendUINT(s, sent_size) != P_YES) return P_ERROR;
   if (SendSTR(s, buf_p, sent_size) != P_YES) return P_ERROR;

   buf_p += sent_size;

   return P_YES;
}

// ����� ������ � �������� �����
int CBinaryBuffer::Receive(SOCKET  s)
{
   char *buf = (char*)malloc(RECV_BLOCK_SIZE);
   if (!buf) return P_ERROR;
   UINT block_size;

   // ���������� ���������� ���������� ������
   Reset();

   // ���� ������ ������
   while (1)
   {
      // ���� ������ �� ��������� ����� ������
      if (SendUINT(s, RECV_BLOCK_SIZE) != P_YES) {free(buf); return P_ERROR;}

      // �������� ������ ������������ �����
      if (ReceiveUINT(s, block_size) != P_YES) {free(buf); return P_ERROR;}

      if (block_size == 0) break;  // ������ ��� ������ �� �������

      // �������� ��� ���� ������
      if (ReceiveSTR(s, buf, block_size) != P_YES) {free(buf); return P_ERROR;}

      // ���������� ���� ������ � ������
      AddSTR(buf, block_size);
   }

   free(buf);

   return P_YES;
}

void CBinaryBuffer::Dump(const char *fname)
{
   FILE *pFile = fopen(fname, "wb");

   if (!pFile) return;

   fwrite(buf, 1, m_size, pFile);

   fclose(pFile);
}
