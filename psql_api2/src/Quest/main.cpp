#include <stdio.h>
#include "pquest.hpp"   // Интерфейс для получения доступа к данным через ODBC

using namespace PSQL_API;

#pragma comment(lib,"GenType.lib")

void main()
{
   PEnv env;

   if (!env.Connect("NISSAN_S_DISC_DSN"))
   {
      printf("Error %i\n",env.GetErrorCode());
      return;
   }

   Quest q(env);
   
   Int      num1;
   String   str1;
   String   num2;
   String   num3;
   DateTime date1;

   if (!q.Statement("SELECT num1, str1, num2, num3, date1 FROM test ORDER BY id"))
   {
      printf("Error setting statement %i\n",q.GetErrorCode());
      return;
   }

   q.BindField(num1);
   q.BindField(str1);
   q.BindField(num2);
   q.BindField(num3);
   q.BindField(date1);

   if (!q.ExecSQL())
   {
      printf("Error executing statement %i\n",q.GetErrorCode());
      return;
   }

   while (q.Fetch())
   {
//      printf("num1=%i, str1=%s, num2=%s, num3=%s, date1=%i\n",num1.val, str1.str_p,num2.str_p,num3.str_p,(int)date1.val.year);
   }
}
