#ifdef WIN32
#  include <windows.h>
#  include <odbcinst.h>
#  include <sqlext.h>
#  include <sql.h>
#endif

#include "datesup.hpp"

using namespace PSQL_API;

////////////////////////////////////////////////////////////////////////////////////
// Глобальные функции для работы с датами и временем
////////////////////////////////////////////////////////////////////////////////////

// Проверка правильности даты
int PSQL_API::CheckDate(TIMESTAMP_STRUCT &val, int *err_code)
{
   int err_code_loc;
   if (!err_code)  err_code = &err_code_loc;  // Чтоб не было падение по памяти

   *err_code = 0;
   
   // Базовые проверки на корректность значений
   if (val.year<0) {*err_code = 3000; return 0;}
   if (val.month<1 || val.month>12) {*err_code = 3001; return 0;}
   if (val.day<1 || val.day>31) {*err_code = 3002; return 0;}
   
   // Крутая проверка количества дней в месяце
   int n_days = GetMonthDays(val.year,val.month);

   if (val.day>n_days) {*err_code = 3003; return 0;}

   return 1;
}

// Проверка времени на корректность
int PSQL_API::CheckTime(TIMESTAMP_STRUCT &val, int *err_code)
{
   int err_code_loc;
   if (!err_code)  err_code = &err_code_loc;  // Чтоб не было падение по памяти

   *err_code = 0;

   if ( val.hour<0   || val.hour>23   ) {*err_code = 3004; return 0;}
   if ( val.minute<0 || val.minute>59 ) {*err_code = 3005; return 0;}
   if ( val.second<0 || val.second>59 ) {*err_code = 3006; return 0;}

   return 1;
}

// Проверка даты и времени на корректность
int PSQL_API::CheckDateTime(TIMESTAMP_STRUCT &val, int *err_code)
{
   if (!CheckTime(val,err_code)) return 0;
   if (!CheckDate(val,err_code)) return 0;

   return 1;
}

// Получение количества дней в месяце по году и номеру месяца
inline int PSQL_API::GetMonthDays(int Year,int Month)
{
   int bLeapYear = Year % 4 == 0 ? TRUE : FALSE;    // Призак високосного года
   // Иначе ищем количество дней в месяце
   switch(Month)
   {
   case 1:
   case 3:
   case 5:
   case 7:
   case 8:
   case 10:
   case 12:
        return 31;
   case 2: // Февраль
        if(bLeapYear) return 29; else return 28;
   case 4:
   case 6:
   case 9:
   case 11:
        return 30;
   }
   return 0;  // Ошибочная ситуация
}

int PSQL_API::GetIntFromDate(const char *str_in, int num, int &i_out)
// num - номер числа в строке
// i_out - результирующая цифра
{
   char str[20];   // Максимум 19 символов для даты

   char *str_tmp, *str_p;

   i_out = 0;

   // Смотрим, не на конце ли мы строки
   if(*str_in == 0) return 0;
   strncpy(str,str_in,19); // Запоминаем строку целиком

   // Все операции делаем с временной строкой
   str_p = str;
   for(int i=0;i<=num;i++)
   {
      // Пропускаем все разделители
      while(!(*str_p >= '0' && *str_p <='9')) *str_p++;
      // Проверяем, не наткнулись ли мы на конец строки
      if(*str_p == 0) return 0;
      // Читаем само число - начало числа
      str_tmp = str_p;
      while(*str_p >= '0' && *str_p <='9')
         *str_p++;

      // Обнуляем разделитель, если это разделитель
      if(*str_p != 0) *str_p++ = 0;
   }
   // Получаем число
   i_out = atoi(str_tmp);
   
   if (i_out != 0) return 1;
   return 0;
}
