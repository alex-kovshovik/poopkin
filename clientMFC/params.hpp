#pragma once

struct SParams
{
   char ip[30];
   int  port;

   char login[128];
   char pwd[128];

   int autoconnect;
   int autologin;

   bool b_fast_copy;    // Признак быстрого копирования файлов

   void Reset()
   {
      strcpy(ip, "127.0.0.1");
      port = 777;

      strcpy(login, "");
      strcpy(pwd, "");

      autoconnect = 0;
      autologin = 0;

      b_fast_copy = false;
   }
};

int LoadParams(SParams &par);
