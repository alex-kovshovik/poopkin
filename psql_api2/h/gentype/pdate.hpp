#ifndef __PDATE_HPP__
#define __PDATE_HPP__

#include "pgentype.hpp"

// ��� ���� ��� ��������� TIMESTAMP_STRUCT
#ifdef WIN32
#  include <windows.h>
#  include <odbcinst.h>
#  include <sqlext.h>
#  include <sql.h>
#endif

namespace PSQL_API
{
   class DateTime;
   class Date : public PGenType
   {
      char buf[11];          // ����� ��� �������������� � ������ (������ ��� �������� ������� - XXXX.XX.XX - 10 �������� + ���� � �����)
      int err_code;          // ��� ��������� ������ (� ��������� � ������)
   public:
      TIMESTAMP_STRUCT val;  // �������� ���� � ������� (�������� ��� ������������ - �� �������)

      // ������������ � ����������
      Date();
      Date(int Year, int Month, int Day);
      Date(Date &d_in);      // ����������� �����������
      Date(DateTime &d_in);  // �������������� �� ���� ���� ��������
      Date(const char *str_in);

      // ����������� ������� ��� ������ � ODBC
      void* GetBufferPtr() {return (void*)&val;}               // ��������� ��������� �� ����� ����������
      int GetBufferSize() {return sizeof(TIMESTAMP_STRUCT);}   // ��������� ������� ������

      // ��������� �������������� ����
      operator const char*();
      const char *c_str();   // ��� ������ ��� ��������

      // ��������� ����������
      Date operator=(Date d_in);        // ����� ������ �������� - ������������ ��������� � ������������� �����������
      Date operator=(DateTime d_in);
      Date operator=(const char *s_in);

      // �������������� ��������� ��� ������
      double operator-(Date d_in);  // ���������� ���������� ���� ������� ����� ������
      Date operator+=(int days_in);
      Date operator-=(int days_in);
      Date operator++(int);
      Date operator--(int);

      // ��������� ��������� ���
      int operator > (Date d_in);
      int operator < (Date d_in);
      int operator >=(Date d_in);
      int operator <=(Date d_in);
      int operator !=(Date d_in);
      int operator ==(Date d_in);

      // ������� ��������� ����
      void Today(); // ������������� ������� ����
      int Set(int Year,int Month,int Day); // ������������� ����/�����

      // ������� ��������� ���� � �������
      int Year() {return val.year;}
      int Month() {return val.month;}
      int Day() {return val.day;}

      // ������ �������� �������
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}
#endif
