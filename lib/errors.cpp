#include <stdio.h>
#include "commands.h"

const SErrorDescriptor g_errors[] = 
   { ERR_WRONG_LOGIN_OF_PWD,  "��������� ����� ��� ������",
     ERR_SERVER_IS_DOWN,      "������ �����",
     ERR_WRONG_LOGIN_DATA,    "��������� ������ ��� �����������",
     ERR_NOT_AUTHORIZED,      "������� �������� ������ ����� �����������",
     ERR_FILE_NOT_FOUND,      "���� �� ������",
     ERR_WRITING_FILE,        "������ ������ � ����",
     ERR_BLOCK_SIZE_TOOBIG,   "������ ����� ������� �������",
     ERR_BLOCK_SIZE,          "��������� ������ ���������� ������",
     ERR_RECEIVED_FILE_SIZE,  "������������ ������ ����������� �����",
     ERR_UNKNOWN_COMMAND,     "����������� �������",
     ERR_ZERO_PATH_LEN,       "������� ����� ����",
     ERR_SEARCHING_FILES,     "������ ��� ������ ������",
     ERR_OUT_OF_MEMORY,       "�� ���������� ������ ��� ���������� ��������",
     ERR_STRING_IS_TOO_LONG,  "������� ������� ������ ���������",
     ERR_GET_PROCESS_LIST,    "������ ��������� ������ ���������",
     ERR_MAKING_SCREENSHOT,   "������ ��������� ����������� �� ������",
     ERR_STARTING_PROCESS,    "������ �������� ��������",
     ERR_WINAMP_NOT_FOUND,    "�� ������� ���� Winamp'a",
     ERR_WRONG_SCR_FORMAT,    "�������� ��������� ������ �����������",
     ERR_WRONG_JPEG_QUALITY,  "��������� ��������� �������� ������ JPEG (������ ���� �� 0 �� 100)",
     ERR_CONVERTING_TO_JPEG,  "������ ��������������� ����������� � JPEG",
     ERR_OPENING_FILE,        "������ �������� �����",
     ERR_CREATING_PROCESS,    "������ �������� ��������",
     ERR_FPATH_PARAM_NULL,    "������� �������� fpath � ������� ������� ����� �� �������",
     ERR_GETDISTFREESPACE,    "������ ���������� ������� GetDiskFreeSpace",
     ERR_DELETING_FILE,       "������ �������� �����",
     0, 0
   };

const char *err_getting_data = "������ ��������� ������";

// ��������� ������ ������ �� �� ����
const char *GetErrorText(int code)
{
   for (int i=0; g_errors[i].code != 0; i++)
      if (g_errors[i].code == code) return g_errors[i].text;

   static char buf[128];
   sprintf(buf, "����������� ������ ����� %i", code);

   return buf;
}
