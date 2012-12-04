#include <string.h>
#include <stdio.h>
#include <time.h>
#include "pdate.hpp"
#include "pdatetime.hpp"
#include "pstring.hpp"

#include "datesup.hpp"

// Местные объявления
#define TRUE 1
#define FALSE 0

using namespace PSQL_API;

/////////////////////////////////////////////////////////////////////////////////////
// Конструкторы
/////////////////////////////////////////////////////////////////////////////////////
Date :: Date()
{
   GT_type=PGT_DATE;

   memset(&val,0,sizeof(val));
}
Date :: Date(Date &d_in)
{
   GT_type=PGT_DATE;

   val = d_in.val;
}
Date :: Date(const char *str_in)
{
   GT_type=PGT_DATE;
   operator =(str_in);
}
Date :: Date(int Year, int Month, int Day)
{
   GT_type=PGT_DATE;
   Set(Year,Month,Day);
}
//////////////////////////////////////////////////////////////////////////////
// Операторы преобразования типа
//////////////////////////////////////////////////////////////////////////////
Date :: operator const char*()
{
   // Не проверяем ничего на допустимость значения
   sprintf(buf,"%i/%i/%i",(int)val.year,(int)val.month,(int)val.day);
   return buf;     
}

const char *Date :: c_str()
{
   return operator const char*();
}

//////////////////////////////////////////////////////////////////////////////
// Операторы присваивания
//////////////////////////////////////////////////////////////////////////////
Date Date :: operator =(Date d_in)
{
   val = d_in.val;
   return *this;
}

//////////////////////////////////////////////////////////////////////////////
// Чтение даты из строки - разделители между числами могут быть любыми и их может быть сколько угодно
//////////////////////////////////////////////////////////////////////////////
Date Date :: operator =(const char *str_in)
{
   // Дата на правильносьт тут не проверяется - оставим это на совести SQL сервера
   if(!str_in) return *this;

   // Возможны следующие форматы даты (с любыми разделителями):
   // 1) XX - просто день месяца, при этом время нулевое, а месяц и год текущие
   // 2) XX/XX - месяц и день
   // 3) XXXX/XX/XX - год, месяц и день

   // Проверка строки даты на длину
   if(strlen(str_in)<1 || strlen(str_in)>10)
   {
      memset(&val,0,sizeof(val));
      return *this;
   }

   int b_num_present[3];  // Массив признаков присутствия чисел
   int nums[3];           // Массив самих чисел

   // Распарсиваем строку на числа
   for (int i=0;i<3;i++)
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

   if (b_num_present[0] && b_num_present[1] && b_num_present[2])  // Вся дата есть, но нет времени
   {
      val.year   = nums[0];
      val.month  = nums[1];
      val.day    = nums[2];
      return *this;
   }

   // Во всех остальных случаях дата на входе ошибочная
   memset(&val,0,sizeof(val));
   return *this;
}

//////////////////////////////////////////////////////////////////////////////
// Арифметические операторы !!!
//////////////////////////////////////////////////////////////////////////////
// Инкремент даты на один день
Date Date :: operator++(int)
{
   Date d_tmp = *this;
   int n_days;
   
   // Если это 31 декабря, то делаем и выходим
   if (val.month==12 && val.day == 31)
   {
      // Устанавливаем в 1 января следующего года
      val.year++;
      val.month=1;
      val.day=1;

      return d_tmp;
   }
   
   n_days = GetMonthDays(val.year,val.month);

   // Если мы на последнем дне месяца...
   if (n_days == val.day)
   {
      val.day = 1;
      val.month += 1;
      return d_tmp;
   }

   // Иначе...
   val.day += 1;
   return d_tmp;
}

// Декремент даты на один день
Date Date :: operator--(int)
{
   Date d_tmp = *this;
   int n_days;

   // Если это 1 января, то делаем и выходим
   if (val.month==1 && val.day == 1)
   {
      // Устанавливаем в 31 декабря предыдущего года
      val.year--;
      val.month=12;
      val.day=31;

      return d_tmp;
   }

   // Если мы на начале месяца...
   if (val.day == 1)
   {
      val.month -= 1;
      
      n_days = GetMonthDays(val.year,val.month);
      val.day = n_days;
      return d_tmp;
   }

   // Иначе...
   val.day -= 1;
   return d_tmp;
}

Date Date :: operator +=(int days_in)
{
   int sign = days_in>=0 ? 1 : -1;
   days_in = (int)abs(days_in);
   
   for(int i=0;i<days_in;i++)
      sign == 1 ? operator++(1) : operator--(1);

   return *this;
}
Date Date :: operator -=(int days_in)
{
   int sign = days_in>=0 ? 1 : -1;
   days_in = (int)abs(days_in);
   
   for(int i=0;i<days_in;i++)
      sign == 1 ? operator--(1) : operator++(1);
   return *this;
}

// Вычисляем разницу между датами в днях
double Date :: operator -(Date d_in)
{
   // Нужна строгая проверка допустимости значений дат !!!!!!
   // Проверка даты *this
   if (!CheckDate(val,&err_code)) return 0.0;

   // Проверка даты d_in
   if (!CheckDate(d_in.val, &err_code)) return 0.0;

   double n_days=0.0;
   Date d1,d2;
   if (d_in == *this) return 0.0;
   
   // Записываем даты в порядке возрастания
   if(d_in>*this) {d1=*this;d2=d_in;}
   else {d1=d_in;d2=*this;}

   // Считаем годы между датами, если разница между датами больше года
   if (d2.val.year-d1.val.year > 1)
      for (  int i=d1.val.year+1;   i<=d2.val.year-1;  i++   )
         n_days += i % 4==0 ? 366 : 365;  // Учет високосных лет

   // Если разница годов > 0, то считаем дни от первой даты до конца того года + дни от начала года второй даты...
   if (d2.val.year-d1.val.year > 0)
   {
      // Первая дата (пока не дошли до 31 декабря)
      while (!(d1.val.month == 12 && d1.val.day == 31))
      {
         d1++;
         n_days += 1.0;
      }

      // Вторая дата (пока не дошли до 1 января)
      while (!(d2.val.month == 1 && d2.val.day == 1))
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

/////////////////////////////////////////////////////////////////////////////
// Операторы сравнения
/////////////////////////////////////////////////////////////////////////////
int Date :: operator >(Date d_in)
{
   if(val.year > d_in.val.year) return TRUE;
   if(val.year == d_in.val.year)
   {
      if(val.month > d_in.val.month) return TRUE;
      if(val.month == d_in.val.month)
      {
         if(val.day > d_in.val.day) return TRUE;
         else return FALSE;
      }
      else
        return FALSE;
   }
   else
     return FALSE;
}
int Date :: operator >=(Date d_in)
{
   return (operator>(d_in) || operator==(d_in));
}
int Date :: operator <(Date d_in)
{
   if(val.year < d_in.val.year) return TRUE;
   if(val.year == d_in.val.year)
   {
      if(val.month < d_in.val.month) return TRUE;
      if(val.month == d_in.val.month)
      {
         if(val.day < d_in.val.day) return TRUE;
         else return FALSE;
      }
      else
        return FALSE;
   }
   else
     return FALSE;
}
int Date :: operator <=(Date d_in)
{
   return (operator<(d_in) || operator==(d_in));
}
int Date :: operator !=(Date d_in)
{
   return !operator==(d_in);
}
int Date :: operator ==(Date d_in)
{
   if (val.year == d_in.val.year &&
       val.month == d_in.val.month &&
       val.day == d_in.val.day) return TRUE;
   else return FALSE;
}

// Функции установки даты
void Date ::Today()   // "сегодня"
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
   return;
}

// Установка даты - если дата неправильная, то функция возвращает 0
int Date :: Set(int year_in, int month_in, int day_in)
{
   val.year = year_in;
   val.month = month_in;
   val.day = day_in;

   return  CheckDate(val,&err_code);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Прочие полезные функции
/////////////////////////////////////////////////////////////////////////////////////////
void Date :: CopyTo(String &str_in, const char *format)
{
   str_in = (const char*)*this;
}
void Date :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcpy(str_in,(const char*)*this);
}
void Date :: AddTo(String &str_in, const char *format)
{
   str_in += (const char*)*this;
}
void Date :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcat(str_in, (const char*)*this);
}
