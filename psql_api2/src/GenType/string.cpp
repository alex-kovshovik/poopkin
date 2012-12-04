#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "pstring.hpp"

using namespace PSQL_API;

// Чисто местные определения
#define TRUE 1
#define FALSE 0

char* String::empty_string = "";

////////////////////////////////////////////////////////////////////////////
// Конструкторы и деструктор
////////////////////////////////////////////////////////////////////////////
String :: String()
{
   // Присваиваем тип переменной
   GT_type = PGT_STRING;
   
   // Обнуляем указатель на строку и количество выделенной памяти
   str_p     = NULL;
   alloc_len = 0;

#ifdef FAST_PSTRING
   real_len = 0;
#endif
}

String :: String(const String &str_in)
{
   // Присваиваем тип переменной
   GT_type = PGT_STRING;
   
   // Обнуляем указатель на строку и количество выделенной памяти
   str_p     = NULL;
   alloc_len = 0;
#ifdef FAST_PSTRING
   real_len = 0;
#endif

   // Копируем содержимое входной строки
   operator=(str_in.str_p);
}

String :: String(const char *str_in)
{
   // Присваиваем тип переменной
   GT_type = PGT_STRING;
   
   // Обнуляем указатель на строку и количество выделенной памяти
   str_p     = NULL;
   alloc_len = 0;
#ifdef FAST_PSTRING
   real_len = 0;
#endif

   // Копируем содержимое входной строки
   operator=(str_in);
}

String::~String()
{
   // Освобождаем буфер
   if (str_p) free(str_p);
   str_p = NULL;
}

////////////////////////////////////////////////////////////////////////////
// Выделение памяти под строку
////////////////////////////////////////////////////////////////////////////
int String :: Allocate(int new_len)
{
   if (new_len <= (int)alloc_len) return 1;  // Уже выделено достаточно памяти

   // Выделяем память
   str_p = (char*)realloc(str_p,new_len);

   if (!str_p)
   {
      alloc_len = 0;

#ifdef FAST_PSTRING
      real_len = 0;
#endif
      return 0;
   }

   alloc_len = new_len;
   return 1;
}

////////////////////////////////////////////////////////////////////////////
// Операторы преобразования типа
////////////////////////////////////////////////////////////////////////////
String :: operator const char *()
{
   if (!str_p) return empty_string;  // Чтоб ничего никогда не падало
   return str_p;
}

const char *String :: c_str()
{
   return operator const char *();
}

////////////////////////////////////////////////////////////////////////////
// Операторы присваивания
////////////////////////////////////////////////////////////////////////////
String &String :: operator =(const char *s_in)
{
   // alloc_len - сколько реально выделено памяти
   if(!s_in)
   {
      *this = empty_string;
      return *this;
   }

#ifdef FAST_PSTRING
   real_len = (unsigned int)strlen(s_in);
#else
   unsigned int real_len = (unsigned int)strlen(s_in);
#endif

   if(real_len+1>alloc_len)   // +1 потому, что ноль в конце строки
   {
      // Это, может и не самый быстрый способ выделения памяти, но зато экономный
      alloc_len = (real_len>128)?real_len*2:real_len+100+1;

      str_p = (char*)realloc(str_p,alloc_len);
   }

   strcpy(str_p,s_in);
   return *this;
}
String &String ::operator =(String s_in)
{
    operator=((const char*)s_in);
    return *this;
}

////////////////////////////////////////////////////////////////////////////
// Арифметические операторы
////////////////////////////////////////////////////////////////////////////
String &String :: operator +=(const char *s_in)
{
   if(!s_in) return *this;
   
   unsigned int len_in   = (unsigned int)strlen(s_in);
   
#ifndef FAST_PSTRING   
   unsigned int real_len = 0;

   // Расчитываем текущую длину строки (это теперь так будет - не сильно быстро, но ладно)
   if (str_p) real_len = (unsigned int)strlen(str_p);

   // В быстром варианте строки мы длину не расчитываем каждый раз
#endif

   unsigned int old_len = real_len;  // Запоминаем старую длину строки
   
   // Проверяем, выделено ли необходимео количество памяти, если нет,
   // то выделяем больше памяти (На n Байт больше, чем необходимо)
   if(real_len+len_in+1>alloc_len)
   {
      real_len += len_in;
      alloc_len = (real_len>128)?real_len*2:real_len+100+1;
      str_p = (char*)realloc(str_p,alloc_len);

      // Нас ниибет, если не хватило памяти, ради скорости мы это не проверяем
   }
   else
      real_len += len_in;
   
   // Более наворочаное копирование строки !!! (с учетом уже расчитанной старой длины строки)
   char *str_tmp = str_p + old_len;
   strcpy(str_tmp,s_in);
   return *this;
}

String String :: operator +(const char *s_in)
{
   String str_tmp(*this);
   str_tmp += s_in;
   return str_tmp;
}

String String :: operator +(String s_in)
{
   String str_tmp(*this);
   str_tmp += (const char*)s_in;
   return str_tmp;
}

// Добавление к строке одного только символа
String &String :: operator+=(char c)
{
#ifndef FAST_PSTRING
   // Если мы работаем с медленными строками, то не расчитываем
   // текущую длину строки, иначе не расчитываем
   unsigned int real_len = 0;

   if (str_p) real_len = (unsigned int)strlen(str_p);
#endif

   if (alloc_len == 0) // Если не выделено вообще памяти
      Allocate(101);
   else
   {
      // Если выделено недостаточно памяти, то выделяем еще
      if (alloc_len <= real_len + 1)
      {
         alloc_len = (real_len>128)?real_len*2:real_len+100+1;
         str_p = (char*)realloc(str_p,alloc_len);
      }
   }
   
   // Несколько более быстрое добавление одного символа (в FastString еще быстрее, на за счет надежности)
   *(str_p+real_len)   = c;
   *(str_p+ ++real_len) = 0; // :-))))  Можно и так написать: str_p+++real_len, но тогда будет глюк
   return *this;
}

////////////////////////////////////////////////////////////////////////////
// Операторы сравнения
////////////////////////////////////////////////////////////////////////////
int String :: operator >  (String s_in)       {return strcmp(*this,s_in)>0 ? TRUE : FALSE;}
int String :: operator >= (String s_in)       {return strcmp(*this,s_in)>=0 ? TRUE : FALSE;}
int String :: operator <  (String s_in)       {return strcmp(*this,s_in)<0 ? TRUE : FALSE;}
int String :: operator <= (String s_in)       {return strcmp(*this,s_in)<=0 ? TRUE : FALSE;}
int String :: operator == (String s_in)       {return strcmp(*this,s_in)==0 ? TRUE : FALSE;}
int String :: operator != (String s_in)       {return strcmp(*this,s_in)!=0 ? TRUE : FALSE;}

int String :: operator >  (const char *s_in)  {return strcmp(*this,s_in)>0 ? TRUE : FALSE;}
int String :: operator >= (const char *s_in)  {return strcmp(*this,s_in)>=0 ? TRUE : FALSE;}
int String :: operator <  (const char *s_in)  {return strcmp(*this,s_in)<0 ? TRUE : FALSE;}
int String :: operator <= (const char *s_in)  {return strcmp(*this,s_in)<=0 ? TRUE : FALSE;}
int String :: operator == (const char *s_in)  {return strcmp(*this,s_in)==0 ? TRUE : FALSE;}
int String :: operator != (const char *s_in)  {return strcmp(*this,s_in)!=0 ? TRUE : FALSE;}

////////////////////////////////////////////////////////////////////////////
// Операции со строкой
////////////////////////////////////////////////////////////////////////////
// Удаление определенного количества символов из строки
// (память перевыделять тут не будем)
int String::Cut(int index, int count)
{
   int len = Length();

   if (len == 0 || !str_p) return 0;

   // Всякие проверки сначала
   if (count<=0) return 0;
   if (index<0) return 0;
   if (index >= len) return 0;
   if (index+count > len) return 0;

   // Теперь удаляем
   // Рассмотрим случаи:
   // 1. Фрагмент находится в самом конце строки
   if (index+count == len)
   {
      str_p[index] = 0;
      #ifdef FAST_PSTRING
      real_len = index+1;
      #endif

      return 1;
   }

   // 2. Фрагмент находится в середине строки
   // тогда нам нужно скопировать конец строки ближе к началу
   strcpy(str_p+index, str_p+index+count);
   return 1;
}

// Получение длины строки (имеет смысл пользоваться, когда выставлена директива FAST_PSTRING)
int String::Length()
{
#ifdef FAST_PSTRING
   return real_len;
#else
   if (str_p) return strlen(str_p);
   else return 0;
#endif
}

////////////////////////////////////////////////////////////////////////////
// Прочие полезные функции
////////////////////////////////////////////////////////////////////////////
void String :: CopyTo(String &str_in, const char *format)
{
   str_in.operator =(str_p);
}

void String :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcpy(str_in,(const char*)*this);  // Явное преобразование чтоб не вернулся NULL
}

void String :: AddTo(String &str_in, const char *format)
{
   str_in.operator +=(str_p);
}

void String :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcat(str_in,(const char*)*this);  // Явное преобразование чтоб не вернулся NULL
}

void String :: Reset()
{
   if (str_p) free(str_p);
   str_p = NULL;

   alloc_len = 0;
   real_len = 0;
}