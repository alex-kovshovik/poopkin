#ifndef __DATESUP_HPP__

namespace PSQL_API
{
   // �������� ���� � ������� �� ������������
   int GetMonthDays(int Year,int Month);                         // ��������� ���������� ���� � ������ �� ���� � ������ ������
   int CheckDate(TIMESTAMP_STRUCT &val, int *err_code=NULL);     // �������� ���� �� ������������
   int CheckTime(TIMESTAMP_STRUCT &val, int *err_code=NULL);     // �������� ������� �� ������������
   int CheckDateTime(TIMESTAMP_STRUCT &val, int *err_code=NULL); // �������� ���� � ������� �� ������������

   // ������ �������
   int GetIntFromDate(const char *str_in, int num, int &i_out); // ��� �������������� ������ � ����
}

#endif