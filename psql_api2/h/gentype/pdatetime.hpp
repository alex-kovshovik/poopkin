#ifndef __PDATETIME_HPP__
#define __PDATETIME_HPP__

#ifdef WIN32
#  include <windows.h>
#  include <odbcinst.h>
#  include <sqlext.h>
#  include <sql.h>
#endif

#include "pgentype.hpp"

namespace PSQL_API
{
   class Date;
   class DateTime : public PGenType
   {
      char buf[20];            // ����� ��� �������������� � ������ (������ ��� �������� ������� - XX.XX.XX XXXX.XX.XX - 19 �������� + ���� � �����)
      int err_code;            // ��� ��������� ������ (� ��������� � ������)
   public:
      TIMESTAMP_STRUCT val;  // �������� ���� � ������� (�������� ��� ������������ - �� �������)

      // ������������ � ����������
      DateTime();
      DateTime(int Year, int Month, int Day, int Hour=0, int Minute=0, int Second=0);
      DateTime(DateTime &d_in);  // ����������� �����������
      DateTime(Date &d_in);      // �������������� �� ���� ���� ��������
      DateTime(const char *str_in);

      // ����������� ������� ��� ������ � ODBC
      void* GetBufferPtr() {return (void*)&val;} // ��������� ��������� �� ����� ����������
      int GetBufferSize() {return sizeof(TIMESTAMP_STRUCT);}          // ��������� ������� ������

      // ��������� �������������� ����
      operator const char*();
      const char *c_str();   // ��� ������ ��� ��������

      // ��������� ����������
      DateTime operator=(DateTime d_in);
      DateTime operator=(Date d_in);
      DateTime operator=(const char *s_in);

      // �������������� ��������� ��� ������
      // !!! �������� !!!
//        double operator-(DateTime d_in); // ���������� ���������� ����� ������� ����� ������
      // ��������� ��������� ���
      int operator >(DateTime d_in);
      int operator <(DateTime d_in);
      int operator >=(DateTime d_in);
      int operator <=(DateTime d_in);
      int operator !=(DateTime d_in);
      int operator == (DateTime d_in);

      // ������� ��������� ����
      void Now();   // ������������� ������� �����/����
      void Today(); // ������������� ������� ����, ����� ��������
      int  SetDate(int Year,int Month,int Day); // ������ ������������� ����
      int  SetTime(int Hour,int Minute,int Second); // ������������� �����
      int  Set(int Year,int Month,int Day,int Hour,int Minute,int Second); // ������������� ����/�����

      // ������� ��������� ���� � �������
      int Year() {return val.year;}
      int Month() {return val.month;}
      int Day() {return val.day;}
      int Hour() {return val.hour;}
      int Minute() {return val.minute;}
      int Second() {return val.second;}

      // ������ �������� �������
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}
#endif
