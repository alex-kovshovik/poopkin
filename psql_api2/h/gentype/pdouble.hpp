#ifndef __PDOUBLE_HPP__
#define __PDOUBLE_HPP__

#include "pgentype.hpp"

namespace PSQL_API
{
   class String;
   class Double : public PGenType
   {
      char   buf[30];  // Буфер для преобразования в строку
   public:
      double val;      // Значение переменной - ее можно делать доступной, т. к. это ничего не испортит

      // Конструкторы (деструктор тут не нужен)
      Double();
      Double(double d_in);
      Double(Double &d_in);        // Конструктор копирования
      Double(const char *str_in);  // Автоматическое преобразование из строки

      // Виртуальные функции для работы с ODBC
      void* GetBufferPtr() {return (void*)&val;}    // Получение указателя на буфер переменной
      int GetBufferSize() {return sizeof(double);}  // Получение размера буфера

      // Операторы преобразования типа
      operator double();
      operator const char*();
      const char *c_str();   // Это только для удобства

      // Операторы присвоения
      Double operator=(Double d_in);
      Double operator=(char *s_in);

      // Арифметические операторы
      // 0
      Double operator++();     // префиксный оператор
      Double operator++(int);  // постфиксный
      Double operator--();
      Double operator--(int);  // постфиксный
      // 1
      Double operator+(Double d_in);
      Double operator-(Double d_in);
      Double operator*(Double d_in);
      Double operator/(Double d_in);

      Double operator+(double d_in);
      Double operator-(double d_in);
      Double operator*(double d_in);
      Double operator/(double d_in);
   
      // 2
      Double operator+=(Double d_in);
      Double operator-=(Double d_in);
      Double operator*=(Double d_in);
      Double operator/=(Double d_in);

      // Операторы сравнения
      int operator >(Double d_in);
      int operator <(Double d_in);
      int operator >=(Double d_in);
      int operator <=(Double d_in);
      int operator !=(Double d_in);
      int operator == (Double d_in);

      int operator >(double d_in);
      int operator <(double d_in);
      int operator >=(double d_in);
      int operator <=(double d_in);
      int operator !=(double d_in);
      int operator == (double d_in);

      // Прочие полезные функции
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}
#endif
