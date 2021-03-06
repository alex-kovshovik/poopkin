#pragma once

// ����������� ����� ������ �������
#define  SC_AUTHORIZE        1000  // �����������
#define  SC_GET_FILE         1001  // ��������� �����
#define  SC_SHUTDOWN_SERVER  1002  // ������� �������
#define  SC_CONTINUE         1003  // ������� ������� ���������� �������� (���� ������ ��� ��������� ������)
#define  SC_GETVERSION       1004  // ��������� ������ ������� (����� - ��������� + 4 ����� - ����� ������)
#define  SC_UPDATE_SERVER    1005  // ���������� ������� �� ����� ������
#define  SC_GET_FILE_LIST    1006  // ��������� ������ ������ � ������ ��������
#define  SC_GET_DRIVES_LIST  1007  // ��������� ������ ���������� ������
#define  SC_SHOW_MESSAGE     1008  // ���������� ��������� �� ������
#define  SC_GET_PROCESS_LIST 1009  // ��������� ������ ���������� ���������
#define  SC_OPEN_CD_DOOR     1010  // ������� ������ CD-ROM'a
#define  SC_GET_SCREENSHOT   1011  // �������� ����������� � ������ ������
#define  SC_UPLOAD_FILE      1012  // �������� ����� �� ������
#define  SC_CREATE_PROCESS   1013  // ������ ��������� �� ����������
#define  SC_SHELL_EXECUTE    1014  // ������ ������-���� �����, ��������, �������� ��� MP3
#define  SC_MAX_WINAMP_VOL   1015  // ������� ���� � Winamp'e ������������
#define  SC_GET_FILES_TREE   1016  // ��������� ������ ��������� � ������ (� ������������� ��� �� �������)
#define  SC_GET_FILES_LIST   1017  // �������� ������ �������
#define  SC_GET_FREE_SPACE   1018  // ��������� ���������� ���������� �����
#define  SC_DIRECT_UPLOAD    1019  // ������ �������� ������ � �������� ���� ��� � ������� ����� �� ������� - ���������� �������
#define  SC_DELETE_FILE      1020  // �������� ����� � �������
#define  SC_GET_FILE_EX      1021  // ��������� ����� � ��������

// ����� �������:
// - �������� ������ (���������� �������� �������)
// - ��������� ������ ���������
// - ���������� ��������
// - ������������ ����������
// - ��������� ����������� �� ������
// - 

// ����������� ����� ������� �������
#define  CC_ERROR             2000  // ������ ���������� ������� - �� ��� ����� ���� ��� ������, � �����, ��������, � ���� ������
#define  CC_CONFIRM           2001  // ������������� ��������� ���������� �������

/////////////////////////////////////////////////////////////////////////////////////
// ���� ������ ���������� ������
/////////////////////////////////////////////////////////////////////////////////////
#define  ERR_WRONG_LOGIN_OF_PWD 3000  // ��������� ����� ��� ������
#define  ERR_SERVER_IS_DOWN     3001  // ������ ����
#define  ERR_WRONG_LOGIN_DATA   3002  // ��������� ������ ��� ������
#define  ERR_NOT_AUTHORIZED     3003  // ����������������� ������������ �������� ����������
#define  ERR_FILE_NOT_FOUND     3004  // ���� �� ������
#define  ERR_WRITING_FILE       3005  // ������ ������ �����
#define  ERR_BLOCK_SIZE_TOOBIG  3006  // ������ ����� ������ ������� �������
#define  ERR_BLOCK_SIZE         3007  // ��������� ������ ���������� ������
#define  ERR_RECEIVED_FILE_SIZE 3008  // ������������ ������ ��������� �����
#define  ERR_UNKNOWN_COMMAND    3009  // ����������� ������� �������
#define  ERR_ZERO_PATH_LEN      3010  // ������� ����� ����
#define  ERR_SEARCHING_FILES    3011  // ������ ��� ������ ������
#define  ERR_OUT_OF_MEMORY      3012  // �� ������� ������
#define  ERR_STRING_IS_TOO_LONG 3013  // ������� ������� ������ ���������
#define  ERR_GET_PROCESS_LIST   3014  // ������ ��������� ������ ���������
#define  ERR_MAKING_SCREENSHOT  3015  // ������ ��������� ����������� ������ ������
#define  ERR_STARTING_PROCESS   3016  // ������ �������� ��������
#define  ERR_WINAMP_NOT_FOUND   3017  // �� ������� ���� Winamp'a
#define  ERR_WRONG_SCR_FORMAT   3018  // �������� ��������� ������ ��������
#define  ERR_WRONG_JPEG_QUALITY 3019  // ��������� ��������� �������� ������ (�. �. �� 0 �� 100)
#define  ERR_CONVERTING_TO_JPEG 3020  // ������ ��������������� ����������� � JPEG
#define  ERR_OPENING_FILE       3021  // ������ �������� �����
#define  ERR_CREATING_PROCESS   3022  // ������ �������� ��������
#define  ERR_FPATH_PARAM_NULL   3023  // ������� �������� fpath � ������� ������� �����
#define  ERR_SHELL_EXECUTE      3024  // ������ ���������� ������� ShellExecute
#define  ERR_GETDISTFREESPACE   3025  // ������ ���������� ������� GetDiskFreeSpace
#define  ERR_COPYING_FILE       3026  // ������ ����������� �����
#define  ERR_DELETING_FILE      3027  // ������ �������� �����

// ���������-��������� ������
struct SErrorDescriptor
{
   int code;   // ��� ������
   char *text; // ����� ������
};

extern const SErrorDescriptor g_errors[];
extern const char *err_getting_data;
const char *GetErrorText(int code);
