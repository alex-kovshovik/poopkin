#include "pgentype.hpp"

#ifdef WIN32
#  include <windows.h>
#  include <sql.h>
#  include <sqlext.h>
#  include <odbcinst.h>
#endif

using namespace PSQL_API;

// ѕолучение типа данных SQL дл€ типа данных GenType
int PGenType :: GetSQLType()
{
   // ѕока будем работать только с трем€ базовыми типами данных
   switch(GT_type)
   {
   case PGT_INT:
      return SQL_C_LONG;

   case PGT_STRING:
      return SQL_C_CHAR;

   case PGT_DOUBLE:
      return SQL_C_DOUBLE;

   case PGT_DATE:
   case PGT_DATETIME:
      return SQL_C_TYPE_TIMESTAMP;

   default:
      return 0;
   }

   return 0;
}
