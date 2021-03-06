#ifndef __PDOUBLE_HPP__
#define __PDOUBLE_HPP__

#include "pgentype.hpp"

namespace PSQL_API
{
   class String;
   class Double : public PGenType
   {
      char   buf[30];  // ����� ��� �������������� � ������
   public:
      double val;      // �������� ���������� - �� ����� ������ ���������, �. �. ��� ������ �� ��������

      // ������������ (���������� ��� �� �����)
      Double();
      Double(double d_in);
      Double(Double &d_in);        // ����������� �����������
      Double(const char *str_in);  // �������������� �������������� �� ������

      // ����������� ������� ��� ������ � ODBC
      void* GetBufferPtr() {return (void*)&val;}    // ��������� ��������� �� ����� ����������
      int GetBufferSize() {return sizeof(double);}  // ��������� ������� ������

      // ��������� �������������� ����
      operator double();
      operator const char*();
      const char *c_str();   // ��� ������ ��� ��������

      // ��������� ����������
      Double operator=(Double d_in);
      Double operator=(char *s_in);

      // �������������� ���������
      // 0
      Double operator++();     // ���������� ��������
      Double operator++(int);  // �����������
      Double operator--();
      Double operator--(int);  // �����������
      // 1
      Double operator+(Double d_in);
      Double operator-(Double d_in);
      Double operator*(Double d_in);
      Double operator/(Double d_in);

      Double operator+(double d_in);
      Double operator-(double d_in);
      Double operator*(double d_in);
      Double operator/(double d_in);
   
      // 2
      Double operator+=(Double d_in);
      Double operator-=(Double d_in);
      Double operator*=(Double d_in);
      Double operator/=(Double d_in);

      // ��������� ���������
      int operator >(Double d_in);
      int operator <(Double d_in);
      int operator >=(Double d_in);
      int operator <=(Double d_in);
      int operator !=(Double d_in);
      int operator == (Double d_in);

      int operator >(double d_in);
      int operator <(double d_in);
      int operator >=(double d_in);
      int operator <=(double d_in);
      int operator !=(double d_in);
      int operator == (double d_in);

      // ������ �������� �������
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}
#endif
