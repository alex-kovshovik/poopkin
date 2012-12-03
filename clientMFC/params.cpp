#include <string.h>
#include <stdlib.h>
#include "inifile.hpp"
#include "params.hpp"

int LoadParams(SParams &par)
{
   CIniFile ini;
   if (ini.Init("client.ini") != YES) return 0;

   char buf[512];

   ini.GetString("connect.ip", par.ip);
   ini.GetString("connect.port", buf);
   par.port = atoi(buf);

   ini.GetString("login.user", par.login);
   ini.GetString("login.password", par.pwd);

   if (ini.GetString("connect.autoconnect", buf) == YES)
      if (stricmp(buf, "YES") == 0) par.autoconnect = 1;

   if (ini.GetString("login.autologin", buf) == YES)
      if (stricmp(buf, "YES") == 0) par.autologin = 1;

   if (ini.GetString("copying.fast_copy", buf) == YES)
      if (stricmp(buf, "YES") == 0) par.b_fast_copy = true;

   return 1;
}
