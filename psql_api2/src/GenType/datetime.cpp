#include <string.h>
#include <stdio.h>
#include <time.h>
#include "pdate.hpp"
#include "pdatetime.hpp"
#include "pstring.hpp"

#include "datesup.hpp"  // Дополнительные функции для работы с датой

// Местные объявления
#define TRUE 1
#define FALSE 0

using namespace PSQL_API;

////////////////////////////////////////////////////////////////////
// Конструктор и деструктор
////////////////////////////////////////////////////////////////////
DateTime :: DateTime()
{
   GT_type = PGT_DATETIME;

   // По умолчанию ставим нулевую дату и время
   memset(&val,0,sizeof(val));
}

DateTime :: DateTime(DateTime &d_in)   // Конструктор копирования
{
   GT_type = PGT_DATETIME;

   val = d_in.val;
}

DateTime :: DateTime(const char *str_in)
{
   GT_type = PGT_DATETIME;

   operator=(str_in);
}

DateTime :: DateTime(Date &d_in)      // Преобразование из даты тоже доступно
{
   GT_type = PGT_DATETIME;

//   val = d_in.val;  // Пока не доступно а то даты пока не работают
}

DateTime :: DateTime(int Year, int Month, int Day, int Hour, int Minute, int Second)
{
   GT_type = PGT_DATETIME;

   val.year   = Year;
   val.month  = Month;
   val.day    = Day;
   val.hour   = Hour;
   val.minute = Minute;
   val.second = Second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Чтение даты из строки - разделители между числами могут быть любыми и их может быть сколько угодно
//////////////////////////////////////////////////////////////////////////////////////////////////////
DateTime DateTime :: operator =(const char *str_in)
{
   // Дата на правильносьт тут не проверяется - оставим это на совести SQL сервера
   if(!str_in) return *this;

   // Возможны следующие форматы даты (с любыми разделителями):
   // 1) XX - просто день месяца, при этом время нулевое, а месяц и год текущие
   // 2) XX/XX - месяц и день
   // 3) XXXX/XX/XX - год, месяц и день
   // 4) Если присутствует время, то оно должно быть только целиком

   // Проверка строки даты на длину
   if(strlen(str_in)<1 || strlen(str_in)>19)
   {
      memset(&val,0,sizeof(val));
      return *this;
   }

   int b_num_present[6];  // Массив признаков присутствия чисел
   int nums[6];           // Массив самих чисел

   // Распарсиваем строку на числа
   for (int i=0;i<6;i++)
      b_num_present[i] = GetIntFromDate(str_in,i,nums[i]);

   // Разворачиваем дату
   if (b_num_present[0] && !b_num_present[1])  // Если присутствует только одно число - это число месяца
   {
      // Это новшество! Тут так быть вообще-то не должно
      Today();  // Устанавливаем текущую дату и обнуляем время
      val.day = nums[0];  // В дате только день - сегодня
      return *this;
   }

   if (b_num_present[0] && b_num_present[1] && !b_num_present[2])  // Если только два значения
   {
      Today();  // Устанавливаем текущую дату и обнуляем время
      val.month  = nums[0];
      val.day    = nums[1];
      return *this;
   }

   if (b_num_present[0] && b_num_present[1] && b_num_present[2] && !b_num_present[3])  // Вся дата есть, но нет времени
   {
      val.year   = nums[0];
      val.month  = nums[1];
      val.day    = nums[2];

      val.hour   = 0;
      val.minute = 0;
      val.second = 0;

      return *this;
   }

   // Если же присутствуют все поля, то дата и время считается полностью правильным
   if (b_num_present[0] &&
       b_num_present[1] &&
       b_num_present[2] &&
       b_num_present[3] &&
       b_num_present[4] &&
       b_num_present[5])
   {
      val.year   = nums[0];
      val.month  = nums[1];
      val.day    = nums[2];

      val.hour   = nums[3];
      val.minute = nums[4];
      val.second = nums[5];

      return *this;
   }

   // Во всех остальных случаях дата на входе ошибочная
   memset(&val,0,sizeof(val));
   return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
// Операторы преобразования типа
///////////////////////////////////////////////////////////////////////////////////////
DateTime :: operator const char*()
{
   sprintf(buf,"%i/%i/%i %i:%i:%i",(int)val.year,(int)val.month,(int)val.day,(int)val.hour,(int)val.minute,(int)val.second);
   return buf;  
}

const char *DateTime :: c_str()
{
   return operator const char*();
}

///////////////////////////////////////////////////////////////////////////////////////
// Операторы присваивания
///////////////////////////////////////////////////////////////////////////////////////
DateTime DateTime :: operator =(DateTime d_in)
{
   val = d_in.val;
   return *this;
}

DateTime DateTime :: operator =(Date d_in)
{
   val = d_in.val;
   return *this;
}

///////////////////////////////////////////////////////////////////////////////////////
// Вычисляем разницу между датами в часах  !!! Доделать !!!
///////////////////////////////////////////////////////////////////////////////////////
/*
double DateTime :: operator -(DateTime d_in)
{
   // Нужна строгая проверка допустимости значений дат !!!!!!
   // Проверка даты *this
   if(Value.wYear < 0) return 0.0;
   if(Value.wMonth>12 || Value.wMonth<1) return 0.0;
   if(Value.wDay>31 || Value.wDay<1) return 0.0;
   // Проверка даты d_in
   if(d_in.Value.wYear < 0) return 0.0;
   if(d_in.Value.wMonth>12 || d_in.Value.wMonth<1) return 0.0;
   if(d_in.Value.wDay>31 || d_in.Value.wDay<1) return 0.0;

   double n_days=0.0;
   DateTime d1,d2;
   if (d_in == *this) return 0.0;
   
   // Записываем даты в порядке возрастания
   if(d_in>*this) {d1=*this;d2=d_in;}
   else {d1=d_in;d2=*this;}

   // Считаем годы между датами, если разница между датами больше года
   if (d2.Value.wYear-d1.Value.wYear > 1)
            for (  int i=d1.Value.wYear+1;   i<=d2.Value.wYear-1;  i++   )
                  n_days += i % 4==0 ? 366 : 365;  // Учет високосных лет

   // Если разница годов > 0, то считаем дни от первой даты до конца того года + дни от начала года второй даты...
   if (d2.Value.wYear-d1.Value.wYear > 0)
   {
            // Первая дата (пока не дошли до 31 декабря)
            while (!(d1.Value.wMonth == 12 && d1.Value.wDay == 31))
            {
                  d1++;
                  n_days += 1.0;
            }

            // Вторая дата (пока не дошли до 1 января)
            while (!(d2.Value.wMonth == 1 && d2.Value.wDay == 1))
            {
                  d2--;
                  n_days += 1.0;
            }
   }
   else // Если года не отличаются, то считаем дни между датами
   {
            while (d1 != d2)
            {
                  d1++;
                  n_days += 1.0;
            }
   }

   return n_days;
}
*/

///////////////////////////////////////////////////////////////////////////////////////
// Операторы сравнения
///////////////////////////////////////////////////////////////////////////////////////
int DateTime :: operator >(DateTime d_in)
{
   double d1,d2;
   if(val.year > d_in.val.year) return TRUE;
   if(val.year == d_in.val.year)
   {
      if(val.month > d_in.val.month) return TRUE;
      if(val.month == d_in.val.month)
      {
         if(val.day > d_in.val.day) return TRUE;
         d1 = val.hour*3600.0+val.minute*60.0+val.second;
         d2 = d_in.Hour()*3600.0+d_in.Minute()*60.0+d_in.Second();
         if (d1>d2) return TRUE;
         else return FALSE;
      }
      else
        return FALSE;
   }
   else
     return FALSE;
}
int DateTime :: operator >=(DateTime d_in)
{
        return (operator>(d_in) || operator==(d_in));
}
int DateTime :: operator <(DateTime d_in)
{
   double d1,d2;
   if(val.year < d_in.val.year) return TRUE;
   if(val.year == d_in.val.year)
   {
      if(val.month < d_in.val.month) return TRUE;
      if(val.month == d_in.val.month)
      {
         if(val.day < d_in.val.day) return TRUE;
         d1 = val.hour*3600.0+val.minute*60.0+val.second;
         d2 = d_in.Hour()*3600.0+d_in.Minute()*60.0+d_in.Second();
         if (d1<d2) return TRUE;
         else return FALSE;
      }
      else
        return FALSE;
   }
   else
     return FALSE;
}
int DateTime :: operator <=(DateTime d_in)
{
   return (operator<(d_in) || operator==(d_in));
}
int DateTime :: operator !=(DateTime d_in)
{
   return !operator==(d_in);
}
int DateTime :: operator ==(DateTime d_in)
{
   if (val.year == d_in.val.year     &&
      val.month == d_in.val.month   &&
      val.day == d_in.val.day       &&
      val.hour == d_in.val.hour     &&
      val.minute == d_in.val.minute &&
      val.second == d_in.val.second) return TRUE;
   else return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции установки даты и времени
/////////////////////////////////////////////////////////////////////////////////////////////////////
void DateTime :: Now()   // "сейчас"
{
   time_t  t;
   struct  tm  *tm;
   time (&t);
   tm = localtime (&t);

   val.year     = tm->tm_year + 1900;
   val.month    = tm->tm_mon+1;
   val.day      = tm->tm_mday;
   val.hour     = tm->tm_hour;
   val.minute   = tm->tm_min;
   val.second   = tm->tm_sec;
}

void DateTime :: Today() // "сегодня"
{
   time_t  t;
   struct  tm  *tm;
   time (&t);
   tm = localtime (&t);

   val.year     = tm->tm_year + 1900;
   val.month    = tm->tm_mon+1;
   val.day      = tm->tm_mday;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Функции установки даты и времени (с контролями)
/////////////////////////////////////////////////////////////////////////////////////////////////////
int DateTime :: Set(int year_in, int month_in, int day_in, int hour_in, int minute_in, int second_in)
{
   val.year = year_in;
   val.month = month_in;
   val.day = day_in;
   val.hour = hour_in;
   val.minute = minute_in;
   // Секунда
   if (second_in < 0) second_in = 0;
   if (second_in >59) second_in = 59;
   val.second = second_in;

   return CheckDateTime(val,&err_code);
}

int DateTime :: SetDate(int year_in, int month_in, int day_in)
{
   val.year  = year_in;
   val.month = month_in;
   val.day   = day_in;

   return CheckDate(val,&err_code);
}

int DateTime :: SetTime(int hour_in, int minute_in, int second_in)
{
   val.hour   = hour_in;
   val.minute = minute_in;
   val.second = second_in;

   return CheckTime(val,&err_code);
}
//////////////////////////////////////////////////////////////////////////////
// Прочие полезные функции
//////////////////////////////////////////////////////////////////////////////
void DateTime :: CopyTo(String &str_in, const char *format)
{
   // Пока совершенно плюем на формат
   str_in = (const char*)*this;
}
void DateTime :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcpy(str_in, (const char*)*this);
}
void DateTime :: AddTo(String &str_in, const char *format)
{
   str_in += (const char*)*this;
}
void DateTime :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;

   strcat(str_in, (const char*)*this);
}
