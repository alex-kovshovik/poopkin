#include <stdlib.h>
#include <string.h>
#include "pint.hpp"
#include "pstring.hpp"

using namespace PSQL_API;

////////////////////////////////////////////////////////////////////////////////
// Конструкторы и деструкторы
////////////////////////////////////////////////////////////////////////////////
Int::Int()
{
   val = 0;
   GT_type = PGT_INT;
}

Int::Int(int i_in)
{
   val = i_in;
   GT_type = PGT_INT;
}

Int::Int(Int &i_in)
{
   val = i_in.val;
   GT_type = PGT_INT;
}

Int::Int(const char *str_in)
{
   if (!str_in) val = 0;
   else val = atoi(str_in);

   GT_type = PGT_INT;
}
///////////////////////////////////////////////////////////////////////////////////
// Операторы преобразования типа
///////////////////////////////////////////////////////////////////////////////////
Int :: operator int()   {return val;}
Int :: operator const char*() {return itoa(val,buf,10);}
const char *Int :: c_str() {return operator const char*();}

///////////////////////////////////////////////////////////////////////////////////
// Операторы присваивания
///////////////////////////////////////////////////////////////////////////////////
Int Int :: operator =(Int i_in) {val = i_in.val;return *this;}

Int Int :: operator =(char *s_in)
{
   if(!s_in) {val = 0;return *this;}

   val = atoi(s_in);
   return *this;
}

///////////////////////////////////////////////////////////////////////////////////
// Арифметические операторы
///////////////////////////////////////////////////////////////////////////////////
Int Int :: operator++()    {val++;return *this;}
Int Int :: operator++(int) {Int i_tmp=val; val++;return i_tmp;}
Int Int :: operator--()    {val--;return *this;}
Int Int :: operator--(int) {Int i_tmp=val; val--;return i_tmp;}

Int Int :: operator +(Int i_in) {Int i_tmp = val + (int)i_in;return i_tmp;}
Int Int :: operator -(Int i_in) {Int i_tmp = val - (int)i_in;return i_tmp;}
Int Int :: operator *(Int i_in) {Int i_tmp = val * (int)i_in;return i_tmp;}
Int Int :: operator /(Int i_in) {Int i_tmp = val / (int)i_in;return i_tmp;}

Int Int :: operator +(int i_in) {Int i_tmp = val + i_in;return i_tmp;}
Int Int :: operator -(int i_in) {Int i_tmp = val - i_in;return i_tmp;}
Int Int :: operator *(int i_in) {Int i_tmp = val * i_in;return i_tmp;}
Int Int :: operator /(int i_in) {Int i_tmp = val / i_in;return i_tmp;}

Int Int :: operator +=(Int i_in) {val += (int)i_in;return *this;}
Int Int :: operator -=(Int i_in) {val -= (int)i_in;return *this;}
Int Int :: operator *=(Int i_in) {val *= (int)i_in;return *this;}
Int Int :: operator /=(Int i_in) {val /= (int)i_in;return *this;}

///////////////////////////////////////////////////////////////////////////////////
// Операторы сравнения
///////////////////////////////////////////////////////////////////////////////////
int Int :: operator >(Int i_in) {return val>i_in.val;}
int Int :: operator >=(Int i_in) {return val>=i_in.val;}
int Int :: operator <(Int i_in) {return val<i_in.val;}
int Int :: operator <=(Int i_in) {return val<=i_in.val;}
int Int :: operator !=(Int i_in) {return val!=i_in.val;}
int Int :: operator ==(Int i_in) {return val==i_in.val;}

int Int :: operator >(int i_in) {return val>(int)i_in;}
int Int :: operator >=(int i_in) {return val>=(int)i_in;}
int Int :: operator <(int i_in) {return val<(int)i_in;}
int Int :: operator <=(int i_in) {return val<=(int)i_in;}
int Int :: operator !=(int i_in) {return val!=(int)i_in;}
int Int :: operator ==(int i_in) {return val==(int)i_in;}

///////////////////////////////////////////////////////////////////////////////////
// Прочие полезные функции
///////////////////////////////////////////////////////////////////////////////////
void Int :: CopyTo(String &str_in, const char *format)  // Копировать значение переменной типа Int в строку String
{
   str_in = operator const char*();
}

void Int :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return; // Если строка нулевая - избавляемся от глюков

   strcpy(str_in, (const char*)*this);  // На всякий случай пишем явно
}

void Int :: AddTo(String &str_in, const char *format)
{
   str_in += operator const char*();
}

void Int :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;

   strcat(str_in, (const char*)*this);    // На всякий случай пишем явное преобразование
}

// Возводит число в степень
static int exp_tmp(int num,int exp)
{
   int res = 1;
   for (int i=0;i<exp;i++)
      res *=num;

   return res;
}

Int Int :: LoadFromHex(const char *hex_str_in)
{
   val = 0;
   if (!hex_str_in) return *this;
   if (strlen(hex_str_in) < 3) return *this;  // Длина этого числа должна быть больше двух символов

   if (hex_str_in[0]=='0' && (hex_str_in[1]=='x' || hex_str_in[1]=='X'))
   {
      // Это шестадцатиричное число
      hex_str_in += 2;  // Сдвигаемся вдоль числа
      int len = (int)strlen(hex_str_in);

      for(int i=len-1;i>=0;i--)
      {
         if (hex_str_in[i]>='0' && hex_str_in[i]<='9')
            val += exp_tmp(16,len-1-i)*(hex_str_in[i]-'0');
         else if (hex_str_in[i]>='a' && hex_str_in[i]<='f')
            val += exp_tmp(16,len-1-i)*(hex_str_in[i]-'a'+10);
         else if (hex_str_in[i]>='A' && hex_str_in[i]<='F')
            val += exp_tmp(16,len-1-i)*(hex_str_in[i]-'A'+10);
         else
         {
            // Ошибка - это не шестнадцатиричное число
            val = 0;
            return *this;
         }
      }
      return *this;
   }

   return *this;
}
