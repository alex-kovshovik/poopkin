#ifndef __PDATE_HPP__
#define __PDATE_HPP__

#include "pgentype.hpp"

// Это надо для структуры TIMESTAMP_STRUCT
#ifdef WIN32
#  include <windows.h>
#  include <odbcinst.h>
#  include <sqlext.h>
#  include <sql.h>
#endif

namespace PSQL_API
{
   class DateTime;
   class Date : public PGenType
   {
      char buf[11];          // Буфер для преобразования к строке (только для базового формата - XXXX.XX.XX - 10 символов + ноль в конце)
      int err_code;          // Код последней ошибки (в операциях с датами)
   public:
      TIMESTAMP_STRUCT val;  // Значение даты и времени (доступно для пользователя - не страшно)

      // Конструкторы и деструктор
      Date();
      Date(int Year, int Month, int Day);
      Date(Date &d_in);      // Конструктор копирования
      Date(DateTime &d_in);  // Преобразование из даты тоже доступно
      Date(const char *str_in);

      // Виртуальные функции для работы с ODBC
      void* GetBufferPtr() {return (void*)&val;}               // Получение указателя на буфер переменной
      int GetBufferSize() {return sizeof(TIMESTAMP_STRUCT);}   // Получение размера буфера

      // Операторы преобразования типа
      operator const char*();
      const char *c_str();   // Это только для удобства

      // Операторы присвоения
      Date operator=(Date d_in);        // Очень важный оператор - используется совместно с конструктором копирования
      Date operator=(DateTime d_in);
      Date operator=(const char *s_in);

      // Арифметические операторы над датами
      double operator-(Date d_in);  // Возвращает количество дней разницы между датами
      Date operator+=(int days_in);
      Date operator-=(int days_in);
      Date operator++(int);
      Date operator--(int);

      // Операторы сравнения дат
      int operator > (Date d_in);
      int operator < (Date d_in);
      int operator >=(Date d_in);
      int operator <=(Date d_in);
      int operator !=(Date d_in);
      int operator ==(Date d_in);

      // Функции установки даты
      void Today(); // Устанавливает текущую дату
      int Set(int Year,int Month,int Day); // Устанавливает дату/время

      // Функции получения даты и времени
      int Year() {return val.year;}
      int Month() {return val.month;}
      int Day() {return val.day;}

      // Прочие полезные функции
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}
#endif
