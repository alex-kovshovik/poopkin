#include <stdlib.h>
#include <string.h>
#include "pint.hpp"
#include "pstring.hpp"

using namespace PSQL_API;

////////////////////////////////////////////////////////////////////////////////
// ������������ � �����������
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
// ��������� �������������� ����
///////////////////////////////////////////////////////////////////////////////////
Int :: operator int()   {return val;}
Int :: operator const char*() {return itoa(val,buf,10);}
const char *Int :: c_str() {return operator const char*();}

///////////////////////////////////////////////////////////////////////////////////
// ��������� ������������
///////////////////////////////////////////////////////////////////////////////////
Int Int :: operator =(Int i_in) {val = i_in.val;return *this;}

Int Int :: operator =(char *s_in)
{
   if(!s_in) {val = 0;return *this;}

   val = atoi(s_in);
   return *this;
}

///////////////////////////////////////////////////////////////////////////////////
// �������������� ���������
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
// ��������� ���������
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
// ������ �������� �������
///////////////////////////////////////////////////////////////////////////////////
void Int :: CopyTo(String &str_in, const char *format)  // ���������� �������� ���������� ���� Int � ������ String
{
   str_in = operator const char*();
}

void Int :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return; // ���� ������ ������� - ����������� �� ������

   strcpy(str_in, (const char*)*this);  // �� ������ ������ ����� ����
}

void Int :: AddTo(String &str_in, const char *format)
{
   str_in += operator const char*();
}

void Int :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;

   strcat(str_in, (const char*)*this);    // �� ������ ������ ����� ����� ��������������
}

// �������� ����� � �������
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
   if (strlen(hex_str_in) < 3) return *this;  // ����� ����� ����� ������ ���� ������ ���� ��������

   if (hex_str_in[0]=='0' && (hex_str_in[1]=='x' || hex_str_in[1]=='X'))
   {
      // ��� ���������������� �����
      hex_str_in += 2;  // ���������� ����� �����
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
            // ������ - ��� �� ����������������� �����
            val = 0;
            return *this;
         }
      }
      return *this;
   }

   return *this;
}
