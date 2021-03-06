// ������ ��� ������ � ������ ���������� �� .ini �����
#ifndef __INIFILE_HPP__
#define __INIFILE_HPP__

#include <stdio.h>
#include "pstring.hpp"
#define YES 1
#define NO 0

class CIniFile
{
   char file_name[128]; // ��� ����� ����������
   int GetString(char *&str_in,PSQL_API::String &str_out); // ��������� ���� ������ �� ����� � ����������� ���������

public:
   char LastError[500]; // ������ � ��������� � ��������� ������
   // ������������� � �������� �����
   int Init(const char *file_name_in); // ��������� ����

   // ������ ������ �� �����
   int GetString(char *path_in,char *str_out); // �������� �������� ���������, ���������� ��� � ������
   int IsSet(char *path_in); // ���������, ���� �� �������� �� ������� ����
   
   // ������ ������ � ����
   int WriteString(char *path_in,char *str_in); // ���������� ������-�������� � ����
};

#endif
