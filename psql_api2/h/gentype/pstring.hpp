#ifndef __PSTRING_HPP__
#define __PSTRING_HPP__

#include "pgentype.hpp"

#define FAST_PSTRING // ������� ������!, �� ������ ��� ������ �������������

namespace PSQL_API
{
   // ��� �������: ������� ������� ����� ������ ��� ��������� ��������� ���
   // �� ������� � ������������ �� ������ ���, �� ��� ��������, ���� ��������
   // ���� ��� �������� �������� ������ ������ ������� ��������� ������:
   // ������� ���������, �������� ���������, � ������

   // ������, ����� String ����� ������������ ����� ������, �������
   // � ������ ����� FastString ��� ��������� ������ String, ��� �����
   // ������ ����� ������������� ����������� ���� - ��� ������� ������ �� ��������
   class String : public PGenType
   {
      static char *empty_string; // ����������� ������������ ��������� �� ������ ������
      unsigned int alloc_len;    // ���������� ���������� ������ ��� ������
#ifdef FAST_PSTRING
      unsigned int real_len;     // �������� ����� ������
#endif
      char *str_p;               // ��������� �� ������ (�������, ����������)
   public:
      // ������������ � ����������
      String();
      String(const String &s_in);    // ����������� �����������
      String(const char *s_in);
      ~String();  // ����������

      // ����������� ������� ��� ������ � ODBC
      void* GetBufferPtr() {return (void*)str_p;} // ��������� ��������� �� ����� ����������
      int GetBufferSize() {return alloc_len;}     // ��������� ������� ������

      // ������� ��������� ������ ��� ������
      int Allocate(int new_len);                  // �������� ����� ����� ������ ��� ������
      void Reset();                               // �������� ������ � ���������� ������

      // ��������� �������������� ����
      operator const char*();  // ������ ����� ���������� ������ ����������� ���������
      const char *c_str();   // ��� ������ ��� ��������

      // ��������� ����������
      String &operator=(String s_in);
      String &operator=(const char *s_in);

      // �������������� ��������� !!!
      String operator+(String s_in);
      String operator+(const char *s_in);
      String &operator+=(const char *s_in);
      String &operator+=(char c);

      // ��������� ���������
      int operator >(String s_in);
      int operator <(String s_in);
      int operator >=(String s_in);
      int operator <=(String s_in);
      int operator !=(String s_in);
      int operator == (String s_in);

      int operator >(const char *s_in);
      int operator <(const char *s_in);
      int operator >=(const char *s_in);
      int operator <=(const char *s_in);
      int operator !=(const char *s_in);
      int operator == (const char *s_in);

      // �������� �� ������� (�������� ���������)
      int Cut(int index, int count);   // �������� ������������� ���������� �������� �� ������
      int Length();                    // ��������� ����� ������

      // ������ �������� �������
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}

#endif
