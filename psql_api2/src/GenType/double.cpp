#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pdouble.hpp"
#include "pstring.hpp"

using namespace PSQL_API;

////////////////////////////////////////////////////////////////////////////////
// ������������ � �����������
////////////////////////////////////////////////////////////////////////////////
Double::Double()
{
   val = 0;
   GT_type = PGT_DOUBLE;
}

Double::Double(double d_in)
{
   val = d_in;
   GT_type = PGT_DOUBLE;
}

Double::Double(Double &d_in)
{
   val = d_in.val;
   GT_type = PGT_DOUBLE;
}

Double::Double(const char *str_in)
{
   if (!str_in) val = 0;
   else val = atof(str_in);

   GT_type = PGT_DOUBLE;
}
///////////////////////////////////////////////////////////////////////////////////
// ��������� �������������� ����
///////////////////////////////////////////////////////////////////////////////////
Double :: operator double()   {return val;}
Double :: operator const char*() {sprintf(buf,"%f",val);return buf;}
const char *Double :: c_str() {return operator const char*();}

///////////////////////////////////////////////////////////////////////////////////
// ��������� ������������
///////////////////////////////////////////////////////////////////////////////////
Double Double :: operator =(Double d_in) {val = d_in.val;return *this;}

Double Double :: operator =(char *s_in)
{
   if(!s_in) {val = 0;return *this;}

   val = atof(s_in);
   return *this;
}

///////////////////////////////////////////////////////////////////////////////////
// �������������� ���������
///////////////////////////////////////////////////////////////////////////////////
Double Double :: operator++()    {val++;return *this;}
Double Double :: operator++(int) {Double d_tmp=val; val++;return d_tmp;}
Double Double :: operator--()    {val--;return *this;}
Double Double :: operator--(int) {Double d_tmp=val; val--;return d_tmp;}

Double Double :: operator +(Double d_in) {Double d_tmp = val + (double)d_in;return d_tmp;}
Double Double :: operator -(Double d_in) {Double d_tmp = val - (double)d_in;return d_tmp;}
Double Double :: operator *(Double d_in) {Double d_tmp = val * (double)d_in;return d_tmp;}
Double Double :: operator /(Double d_in) {Double d_tmp = val / (double)d_in;return d_tmp;}

Double Double :: operator +(double d_in) {Double d_tmp = val + d_in;return d_tmp;}
Double Double :: operator -(double d_in) {Double d_tmp = val - d_in;return d_tmp;}
Double Double :: operator *(double d_in) {Double d_tmp = val * d_in;return d_tmp;}
Double Double :: operator /(double d_in) {Double d_tmp = val / d_in;return d_tmp;}

Double Double :: operator +=(Double d_in) {val += (double)d_in;return *this;}
Double Double :: operator -=(Double d_in) {val -= (double)d_in;return *this;}
Double Double :: operator *=(Double d_in) {val *= (double)d_in;return *this;}
Double Double :: operator /=(Double d_in) {val /= (double)d_in;return *this;}

///////////////////////////////////////////////////////////////////////////////////
// ��������� ���������
///////////////////////////////////////////////////////////////////////////////////
int Double :: operator >(Double d_in) {return val>d_in.val;}
int Double :: operator >=(Double d_in) {return val>=d_in.val;}
int Double :: operator <(Double d_in) {return val<d_in.val;}
int Double :: operator <=(Double d_in) {return val<=d_in.val;}
int Double :: operator !=(Double d_in) {return val!=d_in.val;}
int Double :: operator ==(Double d_in) {return val==d_in.val;}

int Double :: operator >(double d_in) {return val>(double)d_in;}
int Double :: operator >=(double d_in) {return val>=(double)d_in;}
int Double :: operator <(double d_in) {return val<(double)d_in;}
int Double :: operator <=(double d_in) {return val<=(double)d_in;}
int Double :: operator !=(double d_in) {return val!=(double)d_in;}
int Double :: operator ==(double d_in) {return val==(double)d_in;}

///////////////////////////////////////////////////////////////////////////////////
// ������ �������� �������
///////////////////////////////////////////////////////////////////////////////////
void Double :: CopyTo(String &str_in, const char *format)  // ���������� �������� ���������� ���� Double � ������ String
{
   str_in = operator const char*();
}

void Double :: CopyTo(char *str_in, const char *format)
{
   if (!str_in) return; // ���� ������ ������� - ����������� �� ������

   strcpy(str_in, (const char*)*this);  // �� ������ ������ ����� ����
}

void Double :: AddTo(String &str_in, const char *format)
{
   str_in += operator const char*();
}

void Double :: AddTo(char *str_in, const char *format)
{
   if (!str_in) return;

   strcat(str_in, (const char*)*this);    // �� ������ ������ ����� ����� ��������������
}
