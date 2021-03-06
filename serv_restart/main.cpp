#include <windows.h>
#include <stdio.h>
#include "shlwapi.h"

#define P_ERROR -1
#define P_YES    0

#pragma comment(lib, "shlwapi.lib")

// ���������� �������
int RestartServer(const char *name)
{
   if (!name) return P_ERROR;

   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // ��������� ������
   if( !CreateProcess( NULL,
                       (LPSTR)name,      // Command line.
                       NULL,             // Process handle not inheritable. 
                       NULL,             // Thread handle not inheritable. 
                       FALSE,            // Set handle inheritance to FALSE. 
                       0,                // No creation flags. 
                       NULL,             // Use parent's environment block. 
                       NULL,             // Use parent's starting directory. 
                       &si,              // Pointer to STARTUPINFO structure.
                       &pi ) )           // Pointer to PROCESS_INFORMATION structure.
      return P_ERROR;

   return P_YES;
}

// ���������� ������� �� ����� ������
int UpdateServer(const char *server_path, const char *new_server_path, const char *recv_file_path)
{
   // ����������, ���������� �� ���� � ����� �������
   FILE *pFile = fopen(recv_file_path, "rb");
   if (!pFile) return 0;
   fclose(pFile);

   while (1)
   {
      // ���������� ��� Windows 98 �� ��������� ��� ��������� �����-����� ��� ������ ���� ���������, �� ��� ������
      DeleteFile(server_path);

      if (!PathFileExists(server_path)) break;  // ���� ����� ��� ���, �� ������� �� �����

      Sleep(500);  // ����� �������� ��� ������� ������ 500 ����������
   }

   // ��������������� ����� ������
   rename(recv_file_path, new_server_path);


   // ��������� ����� ������ �������
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // ��������� ��������� ���������� �������
   if( !CreateProcess( NULL,
                        (LPSTR)new_server_path, // Command line. 
                        NULL,             // Process handle not inheritable. 
                        NULL,             // Thread handle not inheritable. 
                        FALSE,            // Set handle inheritance to FALSE. 
                        0,                // No creation flags. 
                        NULL,             // Use parent's environment block. 
                        NULL,             // Use parent's starting directory. 
                        &si,              // Pointer to STARTUPINFO structure.
                        &pi ) )            // Pointer to PROCESS_INFORMATION structure.
      return P_ERROR;

   return P_YES;
}

// ���������, ��������������� ������ ����� �������������� ���������� �������
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
   if (strlen(lpCmdLine)==0) return 0;

   // ��������������� ��������� ������ � �������
   char *argv[4];

   char *str_p = lpCmdLine;
   char *prev_p = lpCmdLine;  // ������ ����������� ���������
   int cnt = 0;
   int b_space = 0;

   while (*str_p)
   {
      if (!b_space && *str_p == ' ')
      {
         *str_p = 0;
         argv[cnt] = prev_p;
         b_space = 1;
         cnt++;

         if (cnt==4) return 0;
      }
      else
      if (b_space && *str_p != ' ')
      {
         prev_p = str_p;
         b_space = 0;
      }

      str_p++;
   }

   if (cnt == 4) return 0;  // � ��� � ��� ��� 4 ���������

   // ���������� ��������� ��������
   argv[cnt++] = prev_p;

   // 0 - ������� ���������
   // 1 - ��� ��� ����������� ������� (��� update � ��� restart)
   // 2 - ��� ����� ������ ������� (������ ��� ������� update)
   // 3 - ��� ��������� ����� � �������  (������ ��� ������� update)

   if (strcmp(argv[0], "update") == 0)
      return UpdateServer(argv[1], argv[2], argv[3]);
   else
   if (strcmp(argv[0], "restart") == 0)
      return RestartServer(argv[1]);

   return 0;
}
