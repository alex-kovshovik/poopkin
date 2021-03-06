#ifndef __PINT_HPP__
#define __PINT_HPP__

#include "pgentype.hpp"

namespace PSQL_API
{
   class String;
   class Int : public PGenType
   {
      char buf[11];  // ����� ��� �������������� � ������
   public:
      int val;  // �������� ���������� - �� ����� ������ ���������, �. �. ��� ������ �� ��������

      // ������������ (���������� ��� �� �����)
      Int();
      Int(int i_in);
      Int(Int &i_in);           // ����������� �����������
      Int(const char *str_in);  // �������������� �������������� �� ������

      // ����������� ������� ��� ������ � ODBC
      void* GetBufferPtr() {return (void*)&val;}  // ��������� ��������� �� ����� ����������
      int GetBufferSize() {return sizeof(int);}  // ��������� ������� ������

      // ��������� �������������� ����
      operator int();
      operator const char*();
      const char *c_str();   // ��� ������ ��� ��������

      // ��������� ����������
      Int operator=(Int i_in);
      Int operator=(char *s_in);

      // �������������� ���������
      // 0
      Int operator++();     // ���������� ��������
      Int operator++(int);  // �����������
      Int operator--();
      Int operator--(int);  // �����������
      // 1
      Int operator+(Int i_in);
      Int operator-(Int i_in);
      Int operator*(Int i_in);
      Int operator/(Int i_in);

      Int operator+(int i_in);
      Int operator-(int i_in);
      Int operator*(int i_in);
      Int operator/(int i_in);
   
      // 2
      Int operator+=(Int i_in);
      Int operator-=(Int i_in);
      Int operator*=(Int i_in);
      Int operator/=(Int i_in);

      // ��������� ���������
      int operator >(Int i_in);
      int operator <(Int i_in);
      int operator >=(Int i_in);
      int operator <=(Int i_in);
      int operator !=(Int i_in);
      int operator == (Int i_in);

      int operator >(int i_in);
      int operator <(int i_in);
      int operator >=(int i_in);
      int operator <=(int i_in);
      int operator !=(int i_in);
      int operator == (int i_in);

      // ������ �������� �������
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");

      Int LoadFromHex(const char *hex_str_in);
   };
}
#endif
