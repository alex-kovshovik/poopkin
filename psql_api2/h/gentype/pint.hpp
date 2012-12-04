#ifndef __PINT_HPP__
#define __PINT_HPP__

#include "pgentype.hpp"

namespace PSQL_API
{
   class String;
   class Int : public PGenType
   {
      char buf[11];  // Буфер для преобразования в строку
   public:
      int val;  // Значение переменной - ее можно делать доступной, т. к. это ничего не испортит

      // Конструкторы (деструктор тут не нужен)
      Int();
      Int(int i_in);
      Int(Int &i_in);           // Конструктор копирования
      Int(const char *str_in);  // Автоматическое преобразование из строки

      // Виртуальные функции для работы с ODBC
      void* GetBufferPtr() {return (void*)&val;}  // Получение указателя на буфер переменной
      int GetBufferSize() {return sizeof(int);}  // Получение размера буфера

      // Операторы преобразования типа
      operator int();
      operator const char*();
      const char *c_str();   // Это только для удобства

      // Операторы присвоения
      Int operator=(Int i_in);
      Int operator=(char *s_in);

      // Арифметические операторы
      // 0
      Int operator++();     // префиксный оператор
      Int operator++(int);  // постфиксный
      Int operator--();
      Int operator--(int);  // постфиксный
      // 1
      Int operator+(Int i_in);
      Int operator-(Int i_in);
      Int operator*(Int i_in);
      Int operator/(Int i_in);

      Int operator+(int i_in);
      Int operator-(int i_in);
      Int operator*(int i_in);
      Int operator/(int i_in);
   
      // 2
      Int operator+=(Int i_in);
      Int operator-=(Int i_in);
      Int operator*=(Int i_in);
      Int operator/=(Int i_in);

      // Операторы сравнения
      int operator >(Int i_in);
      int operator <(Int i_in);
      int operator >=(Int i_in);
      int operator <=(Int i_in);
      int operator !=(Int i_in);
      int operator == (Int i_in);

      int operator >(int i_in);
      int operator <(int i_in);
      int operator >=(int i_in);
      int operator <=(int i_in);
      int operator !=(int i_in);
      int operator == (int i_in);

      // Прочие полезные функции
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");

      Int LoadFromHex(const char *hex_str_in);
   };
}
#endif
