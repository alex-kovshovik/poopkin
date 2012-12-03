#include <windows.h>
#include <stdio.h>
#include "shlwapi.h"

#define P_ERROR -1
#define P_YES    0

#pragma comment(lib, "shlwapi.lib")

// Перезапуск сервера
int RestartServer(const char *name)
{
   if (!name) return P_ERROR;

   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // Запускаем сервер
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

// Обновление сервера до новой версии
int UpdateServer(const char *server_path, const char *new_server_path, const char *recv_file_path)
{
   // Определяем, существует ли файл с новой версией
   FILE *pFile = fopen(recv_file_path, "rb");
   if (!pFile) return 0;
   fclose(pFile);

   while (1)
   {
      // Специально для Windows 98 мы запускаем эту процедуру много-много раз подряд чтоб убедиться, то все хорошо
      DeleteFile(server_path);

      if (!PathFileExists(server_path)) break;  // Если файла уже нет, то выходим из цикла

      Sleep(500);  // Будем пытаться это сделать каждые 500 милисекунд
   }

   // Переименовываем новую версию
   rename(recv_file_path, new_server_path);


   // Запускаем новую версию сервера
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // Запускаем программу обновления сервера
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

// Программа, перезагружающая сервер после опеределенного промежутка времени
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
   if (strlen(lpCmdLine)==0) return 0;

   // Преобразовываем командную строку к массиву
   char *argv[4];

   char *str_p = lpCmdLine;
   char *prev_p = lpCmdLine;  // Начало предыдущего параметра
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

   if (cnt == 4) return 0;  // У нас и так уже 4 параметра

   // Запоминаем последний параметр
   argv[cnt++] = prev_p;

   // 0 - команда программе
   // 1 - имя уже запущенного сервера (для update и для restart)
   // 2 - имя новой версии сервера (только для команды update)
   // 3 - имя принятого файла с клиента  (только для команды update)

   if (strcmp(argv[0], "update") == 0)
      return UpdateServer(argv[1], argv[2], argv[3]);
   else
   if (strcmp(argv[0], "restart") == 0)
      return RestartServer(argv[1]);

   return 0;
}
