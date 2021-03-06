#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "pstring.hpp"

using namespace PSQL_API;

// ����� ������� �����������
#define TRUE 1
#define FALSE 0

char* String::empty_string = "";

////////////////////////////////////////////////////////////////////////////
// ������������ � ����������
////////////////////////////////////////////////////////////////////////////
String :: String()
{
   // ����������� ��� ����������
   GT_type = PGT_STRING;
   
   // �������� ��������� �� ������ � ���������� ���������� ������
   str_p     = NULL;
   alloc_len = 0;

#ifdef FAST_PSTRING
   real_len = 0;
#endif
}

String :: String(const String &str_in)
{
   // ����������� ��� ����������
   GT_type = PGT_STRING;
   
   // �������� ��������� �� ������ � ���������� ���������� ������
   str_p     = NULL;
   alloc_len = 0;
#ifdef FAST_PSTRING
   real_len = 0;
#endif

   // �������� ���������� ������� ������
   operator=(str_in.str_p);
}

String :: String(const char *str_in)
{
   // ����������� ��� ����������
   GT_type = PGT_STRING;
   
   // �������� ��������� �� ������ � ���������� ���������� ������
   str_p     = NULL;
   alloc_len = 0;
#ifdef FAST_PSTRING
   real_len = 0;
#endif

   // �������� ���������� ������� ������
   operator=(str_in);
}

String::~String()
{
   // ����������� �����
   if (str_p) free(str_p);
   str_p = NULL;
}

////////////////////////////////////////////////////////////////////////////
// ��������� ������ ��� ������
////////////////////////////////////////////////////////////////////////////
int String :: Allocate(int new_len)
{
   if (new_len <= (int)alloc_len) return 1;  // ��� �������� ���������� ������

   // �������� ������
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
// ��������� �������������� ����
////////////////////////////////////////////////////////////////////////////
String :: operator const char *()
{
   if (!str_p) return empty_string;  // ���� ������ ������� �� ������
   return str_p;
}

const char *String :: c_str()
{
   return operator const char *();
}

////////////////////////////////////////////////////////////////////////////
// ��������� ������������
////////////////////////////////////////////////////////////////////////////
String &String :: operator =(const char *s_in)
{
   // alloc_len - ������� ������� �������� ������
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

   if(real_len+1>alloc_len)   // +1 ������, ��� ���� � ����� ������
   {
      // ���, ����� � �� ����� ������� ������ ��������� ������, �� ���� ���������
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
// �������������� ���������
////////////////////////////////////////////////////////////////////////////
String &String :: operator +=(const char *s_in)
{
   if(!s_in) return *this;
   
   unsigned int len_in   = (unsigned int)strlen(s_in);
   
#ifndef FAST_PSTRING   
   unsigned int real_len = 0;

   // ����������� ������� ����� ������ (��� ������ ��� ����� - �� ������ ������, �� �����)
   if (str_p) real_len = (unsigned int)strlen(str_p);

   // � ������� �������� ������ �� ����� �� ����������� ������ ���
#endif

   unsigned int old_len = real_len;  // ���������� ������ ����� ������
   
   // ���������, �������� �� ����������� ���������� ������, ���� ���,
   // �� �������� ������ ������ (�� n ���� ������, ��� ����������)
   if(real_len+len_in+1>alloc_len)
   {
      real_len += len_in;
      alloc_len = (real_len>128)?real_len*2:real_len+100+1;
      str_p = (char*)realloc(str_p,alloc_len);

      // ��� ������, ���� �� ������� ������, ���� �������� �� ��� �� ���������
   }
   else
      real_len += len_in;
   
   // ����� ����������� ����������� ������ !!! (� ������ ��� ����������� ������ ����� ������)
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

// ���������� � ������ ������ ������ �������
String &String :: operator+=(char c)
{
#ifndef FAST_PSTRING
   // ���� �� �������� � ���������� ��������, �� �� �����������
   // ������� ����� ������, ����� �� �����������
   unsigned int real_len = 0;

   if (str_p) real_len = (unsigned int)strlen(str_p);
#endif

   if (alloc_len == 0) // ���� �� �������� ������ ������
      Allocate(101);
   else
   {
      // ���� �������� ������������ ������, �� �������� ���
      if (alloc_len <= real_len + 1)
      {
         alloc_len = (real_len>128)?real_len*2:real_len+100+1;
         str_p = (char*)realloc(str_p,alloc_len);
      }
   }
   
   // ��������� ����� ������� ���������� ������ ������� (� FastString ��� �������, �� �� ���� ����������)
   *(str_p+real_len)   = c;
   *(str_p+ ++real_len) = 0; // :-))))  ����� � ��� ��������: str_p+++real_len, �� ����� ����� ����
   return *this;
}

////////////////////////////////////////////////////////////////////////////
// ��������� ���������
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
// �������� �� �������
////////////////////////////////////////////////////////////////////////////
// �������� ������������� ���������� �������� �� ������
// (������ ������������ ��� �� �����)
int String::Cut(int index, int count)
{
   int len = Length();

   if (len == 0 || !str_p) return 0;

   // ������ �������� �������
   if (count<=0) return 0;
   if (index<0) return 0;
   if (index >= len) return 0;
   if (index+count > len) return 0;

   // ������ �������
   // ���������� ������:
   // 1. �������� ��������� � ����� ����� ������
   if (index+count == len)
   {
      str_p[index] = 0;
      #ifdef FAST_PSTRING
      real_len = index+1;
      #endif

      return 1;
   }

   // 2. �������� ��������� � �������� ������
   // ����� ��� ����� ����������� ����� ������ ����� � ������
   strcpy(str_p+index, str_p+index+count);
   return 1;
}

// ��������� ����� ������ (����� ����� ������������, ����� ���������� ��������� FAST_PSTRING)
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
// ������ �������� �������
////////////////////////////////////////////////////////////////////////////
void String :: CopyTo(String &str_in, const char *format)
{
   str_in.operator =(str_p);
}

void String :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcpy(str_in,(const char*)*this);  // ����� �������������� ���� �� �������� NULL
}

void String :: AddTo(String &str_in, const char *format)
{
   str_in.operator +=(str_p);
}

void String :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;
   strcat(str_in,(const char*)*this);  // ����� �������������� ���� �� �������� NULL
}

void String :: Reset()
{
   if (str_p) free(str_p);
   str_p = NULL;

   alloc_len = 0;
   real_len = 0;
}