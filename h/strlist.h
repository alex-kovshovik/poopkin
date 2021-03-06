// ������� ��� ������/�������� ������� �����
#pragma once

#include <vector>
#include "pstring.hpp"

using namespace std;
using namespace PSQL_API;

// ���������, �������� � ���� ���������� �� ����� �����
struct SFileInfo
{
   DWORD             attrs;
   PSQL_API::String  name;
   DWORD             size_low;    // ������ �����
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

//  �����, �������������� �� ���� �������� �����
// - ��� ������� ������������ �������� ���������� ������ ������ ������ ����� �������
class CBinaryBuffer
{
   char   *buf;           // ��������� �� �����
   size_t  m_allocated;   // ���������� ���������� ������
   size_t  m_size;        // ������� ������ ������

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

   // �������� � ����� ��������� ������
   int Send(SOCKET     s);
   int Receive(SOCKET  s);

   // ���������� ����������� � ����
   void Dump(const char *fname);
};

int SendStrList(SOCKET s, vector<SFileInfo> &list);
int ReceiveStrList(SOCKET s, vector<SFileInfo> &list);
