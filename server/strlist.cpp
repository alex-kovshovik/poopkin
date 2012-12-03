#include <winsock2.h>
#include <stdio.h>
#include "commands.h"
#include "common.h"
#include "strlist.h"

// Блок данных для массива строк - пол мегабайта, но можно менять! :)
#define RECV_BLOCK_SIZE 1024*1024   // Один мегабайт - само то :)

///////////////////////////////////////////////////////////////////////////////////////////////////
// Реализация класса CBinaryBuffer
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

   // Память выделена - копируем данные
   memcpy(buf+m_size, &val, sizeof(val));
   m_size += sizeof(val);
}
void CBinaryBuffer::AddSTR(const char *val, size_t len)
{
   if (!val) return;
   if (len == 0) return;

   if (m_allocated - m_size < len) Allocate(m_allocated<len?m_allocated+len:m_allocated*2);

   // Память выделена - копируем данные
   memcpy(buf+m_size, val, len);
   m_size += len;
}
void CBinaryBuffer::AddFPOST(fpos_t val)
{
   if (m_allocated - m_size < sizeof(val)) Allocate(m_allocated<RECV_BLOCK_SIZE?m_allocated+RECV_BLOCK_SIZE:m_allocated*2);

   // Память выделена - копируем данные
   memcpy(buf+m_size, &val, sizeof(val));
   m_size += sizeof(val);
}

int CBinaryBuffer::GetUINT(const char *&buf_p, UINT &val)
{
   // Проверяем, достаточно ли данных у нас в буфере
   if (buf+m_size-buf_p < sizeof(val)) return P_NO;

   UINT *val_p = (UINT*)buf_p;
   val = *val_p;
   buf_p += sizeof(val);

   return P_YES;
}

int CBinaryBuffer::GetSTR(const char *&buf_p, char *buf, size_t len)
{
   // Проверяем, достаточно ли данных у нас в буфере
   if (size_t(buf+m_size-buf_p) < len) return P_NO;

   memcpy(buf, buf_p, len);
   buf_p += len;

   return P_YES;
}

int CBinaryBuffer::GetFPOST(const char *&buf_p, fpos_t &val)
{
   // Проверяем, достаточно ли данных у нас в буфере
   if (buf+m_size-buf_p < sizeof(val)) return P_NO;

   fpos_t *val_p = (fpos_t*)buf_p;
   val = *val_p;
   buf_p += sizeof(val);

   return P_YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Передача и прием двоичного буфера
///////////////////////////////////////////////////////////////////////////////////////////////////
int CBinaryBuffer::Send(SOCKET s)
{
   // "Спрашиваем" у клиента размер отсылаемого куска
   UINT block_size;  // Размер отсылаемого куска
   UINT sent_size;   // Размер реально отосланного блока
   const char *buf_p = buf;  // Двигающийся указатель на буфер
   
   // Цикл отсылки данных - шлем запрос на получение блока и шлем блок
   while (1)
   {
      // Запрос на получение блока определенного размера
      if (ReceiveUINT(s, block_size) != P_YES) return P_ERROR;

      if (block_size == 0) break;  // Клиент прервал отсылку данных

      if (SendBlock(s, buf_p, block_size, sent_size) != P_YES) return P_ERROR;

      if (sent_size == 0) break;   // Уже отосланы все данные
   }

   return P_YES;
}

// Отсылка блока данных
int CBinaryBuffer::SendBlock(SOCKET s, const char *&buf_p, UINT size, UINT &sent_size)
{
   // buf_p - указатель на начало буфера
   // size - размер его
   // sent_size - размер реально отосланных данных

   sent_size = 0;
   if (buf_p >= buf+m_size) return SendUINT(s, 0);

   // Если требуемый размер блока превышает количество оставшихся данных, то обрезаем блок
   if ( size > UINT((buf+m_size)-buf_p) )
      sent_size = (UINT)((buf+m_size)-buf_p);
   else
      sent_size = size;

   // Отсылаем размер отсылаемых данных и сами данные
   if (SendUINT(s, sent_size) != P_YES) return P_ERROR;
   if (SendSTR(s, buf_p, sent_size) != P_YES) return P_ERROR;

   buf_p += sent_size;

   return P_YES;
}

// Прием данных в двоичный буфер
int CBinaryBuffer::Receive(SOCKET  s)
{
   char *buf = (char*)malloc(RECV_BLOCK_SIZE);
   if (!buf) return P_ERROR;
   UINT block_size;

   // Сбрасываем предыдущее содержимое буфера
   Reset();

   // Цикл приема данных
   while (1)
   {
      // Шлем запрос на получение блока данных
      if (SendUINT(s, RECV_BLOCK_SIZE) != P_YES) {free(buf); return P_ERROR;}

      // Получаем размер принимаемого блока
      if (ReceiveUINT(s, block_size) != P_YES) {free(buf); return P_ERROR;}

      if (block_size == 0) break;  // Больше нет данных от сервера

      // Получаем сам блок данных
      if (ReceiveSTR(s, buf, block_size) != P_YES) {free(buf); return P_ERROR;}

      // Запоминаем блок данных в буфере
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
