#ifndef __PSTRING_HPP__
#define __PSTRING_HPP__

#include "pgentype.hpp"

#define FAST_PSTRING // Быстрые строки!, но опасно для других пользователей

namespace PSQL_API
{
   // Тут дилемма: хранить текущую длину строки для ускорения обработки или
   // не хранить и рассчитывать ее каждый раз, но так медленно, зато надежнее
   // Чтоб все ускорить придется писать самому функции обработки строки:
   // вставка подстроки, удаление подстроки, и прочие

   // Короче, класс String будет рассчитывать длину всегда, поэтому
   // Я создам класс FastString как наследник класса String, где длина
   // строки будет расчитываться значительно реже - для быстрой работы со строками
   class String : public PGenType
   {
      static char *empty_string; // Повсеместно используемый указатель на пустую строку
      unsigned int alloc_len;    // Количество выделенной памяти под строку
#ifdef FAST_PSTRING
      unsigned int real_len;     // Реальная длина строки
#endif
      char *str_p;               // Указатель на строку (ессесна, недоступно)
   public:
      // Конструкторы и деструктор
      String();
      String(const String &s_in);    // Конструктор копирования
      String(const char *s_in);
      ~String();  // Деструктор

      // Виртуальные функции для работы с ODBC
      void* GetBufferPtr() {return (void*)str_p;} // Получение указателя на буфер переменной
      int GetBufferSize() {return alloc_len;}     // Получение размера буфера

      // Функции выделения памяти под строку
      int Allocate(int new_len);                  // Выделить новый объем памяти под строку
      void Reset();                               // Очистить строку и освободить память

      // Операторы преобразования типа
      operator const char*();  // Теперь будем возвращать только константное выражение
      const char *c_str();   // Это только для удобства

      // Операторы присвоения
      String &operator=(String s_in);
      String &operator=(const char *s_in);

      // Арифметические операторы !!!
      String operator+(String s_in);
      String operator+(const char *s_in);
      String &operator+=(const char *s_in);
      String &operator+=(char c);

      // Операторы сравнения
      int operator >(String s_in);
      int operator <(String s_in);
      int operator >=(String s_in);
      int operator <=(String s_in);
      int operator !=(String s_in);
      int operator == (String s_in);

      int operator >(const char *s_in);
      int operator <(const char *s_in);
      int operator >=(const char *s_in);
      int operator <=(const char *s_in);
      int operator !=(const char *s_in);
      int operator == (const char *s_in);

      // Операции со строкой (удаление подстроки)
      int Cut(int index, int count);   // Удаление определенного количества символов из строки
      int Length();                    // Получение длины строки

      // Прочие полезные функции
      void CopyTo(String &str_in, const char *format = "");
      void CopyTo(char *str_in, const char *format = "");
      void AddTo(String &str_in, const char *format = "");
      void AddTo(char *str_in, const char *format = "");
   };
}

#endif
