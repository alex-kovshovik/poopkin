#ifndef __DATESUP_HPP__

namespace PSQL_API
{
   // Проверка даты и времени на корректность
   int GetMonthDays(int Year,int Month);                         // Получение количества дней в месяце по году и номеру месяца
   int CheckDate(TIMESTAMP_STRUCT &val, int *err_code=NULL);     // Проверка даты на корректность
   int CheckTime(TIMESTAMP_STRUCT &val, int *err_code=NULL);     // Проверка времени на корректность
   int CheckDateTime(TIMESTAMP_STRUCT &val, int *err_code=NULL); // Проверка даты и времени на корректность

   // Прочие функции
   int GetIntFromDate(const char *str_in, int num, int &i_out); // Для преобразования строки в дату
}

#endif