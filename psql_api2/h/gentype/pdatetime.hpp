#ifndef __PDATETIME_HPP__
#define __PDATETIME_HPP__

#ifdef WIN32
#  include <windows.h>
#  include <odbcinst.h>
#  include <sqlext.h>
#  include <sql.h>
#endif

#include "pgentype.hpp"

namespace PSQL_API
{
   class Date;
   class DateTime : public PGenType
   {
      char buf[20];            // Буфер для преобразования к строке (только для базового формата - XX.XX.XX XXXX.XX.XX - 19 символов + ноль в конце)
      int err_code;            // Код последней ошибки (в операциях с датами)
   public:
      TIMESTAMP_STRUCT val;  // Значение даты и времени (доступно для пользователя - не страшно)

      // Конструкторы и деструктор
      DateTime();
      DateTime(int Year, int Month, int Day, int Hour=0, int Minute=0, int Second=0);
      DateTime(DateTime &d_in);  // Конструктор копирования
      DateTime(Date &d_in);      // Преобразование из даты тоже доступно
      DateTime(const char *str_in);

      // Виртуальные функции для работы с ODBC
      void* GetBufferPtr() {return (void*)&val;} // Получение указателя на буфер переменной
      int GetBufferSize() {return sizeof(TIMESTAMP_STRUCT);}          // Получение размера буфера

      // Операторы преобразования типа
      operator const char*();
      const char *c_str();   // Это только для удобства

      // Операторы присвоения
      DateTime operator=(DateTime d_in);
      DateTime operator=(Date d_in);
      DateTime operator=(const char *s_in);

      // Арифметические операторы над датами
      // !!! Доделать !!!
//        double operator-(DateTime d_in); // Возвращает количество часов разницы между датами
      // Операторы сравнения дат
      int operator >(DateTime d_in);
      int operator <(DateTime d_in);
      int operator >=(DateTime d_in);
      int operator <=(DateTime d_in);
      int operator !=(DateTime d_in);
      int operator == (DateTime d_in);

      // Функции установки даты
      void Now();   // Устанавливает текущее время/дату
      void Today(); // Устанавливает текущую дату, время обнуляет
      int  SetDate(int Year,int Month,int Day); // Просто устанавливает дату
      int  SetTime(int Hour,int Minute,int Second); // Устанавливает время
      int  Set(int Year,int Month,int Day,int Hour,int Minute,int Second); // Устанавливает дату/время

      // Функции получения даты и времени
      int Year() {return val.year;}
      int Month() {return val.month;}
      int Day() {return val.day;}
      int Hour() {return val.hour;}
      int Minute() {return val.minute;}
      int Second() {return val.second;}

      // Прочие полезные функции
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}
#endif
