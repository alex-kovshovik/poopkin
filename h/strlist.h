// Функции для приема/передачи списков строк
#pragma once

#include <vector>
#include "pstring.hpp"

using namespace std;
using namespace PSQL_API;

// Структура, хранящая в себе информацию об одном файле
struct SFileInfo
{
   DWORD             attrs;
   PSQL_API::String  name;
   DWORD             size_low;    // Размер файла
   DWORD             size_high;

   SFileInfo()
   {
      attrs     = 0;
      size_low  = 0;
      size_high = 0;
   }

   SFileInfo(const SFileInfo &fi_in)
   {
      attrs      = fi_in.attrs;
      name       = fi_in.name;
      size_low   = fi_in.size_low;
      size_high  = fi_in.size_high;
   }

   SFileInfo &operator=(const SFileInfo fi_in)
   {
      attrs      = fi_in.attrs;
      name       = fi_in.name;
      size_low   = fi_in.size_low;
      size_high  = fi_in.size_high;

      return *this;
   }
};

//  Класс, представляющий из себя двоичный буфер
// - для отсылки относительно большого количества данных мелких данных одним пакетом
class CBinaryBuffer
{
   char   *buf;           // Указатель на буфер
   size_t  m_allocated;   // количество выделенной памяти
   size_t  m_size;        // Текущий размер данных

   void Allocate(size_t new_size);

   int SendBlock(SOCKET s, const char *&buf_p, UINT size, UINT &sent_size);
public:
   CBinaryBuffer();
   ~CBinaryBuffer();

   void Reset();

   void AddUINT(UINT val);
   void AddSTR(const char *val, size_t len);
   void AddFPOST(fpos_t pos);

   int GetUINT(const char *&buf_p, UINT &val);
   int GetSTR(const char *&buf_p, char *buf, size_t len);
   int GetFPOST(const char *&buf_p, fpos_t &val);

   const char *GetBuffer() {return buf;}
   size_t      GetSize() {return m_size;}

   // Передача и прием двоичного буфера
   int Send(SOCKET     s);
   int Receive(SOCKET  s);

   // Скидывание содержимого в файл
   void Dump(const char *fname);
};

int SendStrList(SOCKET s, vector<SFileInfo> &list);
int ReceiveStrList(SOCKET s, vector<SFileInfo> &list);
