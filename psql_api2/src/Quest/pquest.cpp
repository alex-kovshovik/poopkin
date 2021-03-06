#include <stdio.h>
#include "pquest.hpp"

using namespace PSQL_API;
//============================================================================
//=========       ���������� ������ PConnection       ========================
//============================================================================

//+++++++++++ ����������� ++++++++++++++++++++++++++++++++++++++++++++++++++++
PEnv::PEnv()
{
   henv=NULL;
   hdbc=NULL;

   err_code = 0;  // �� ��������� ��� ���� ������
}

//+++++++++++ ���������� �� �� +++++++++++++++++++++++++++++++++++++++++++++++
void PEnv::Disconnect()
{
   if (hdbc != NULL)
   {
      SQLDisconnect(hdbc);
      SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
      hdbc = NULL;
   }

   if (henv != NULL)
   {
      SQLFreeHandle(SQL_HANDLE_ENV, henv);
      henv = NULL;
   }
}

//++++++++++ ����������� � �� ++++++++++++++++++++++++++++++++++++++++++++++++
int PEnv::Connect(const char *DSN, const char *user_name, const char *pwd)
{
   // ������� �� ������ ������ ������ ����������, ���� �� ����������
   Disconnect();

   // ��������� �������� ��������
   if (::SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv) == SQL_ERROR)
   {
      err_code = 1000;  // ������ �������� ��������� ��������
      return 0;
   }

   // ������������� ������ ODBC ��� ������ ��������
   ::SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, SQL_IS_INTEGER);

   // ������� ���������� ��������� ������
   if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) == SQL_ERROR)
   {
      err_code = 1001; // ������ �������� ����������� ��������� ������
      return 0;
   }

   // ������������ � ��������� ������
   if (::SQLConnect(hdbc, (SQLTCHAR*)DSN, SQL_NTS,
            (SQLTCHAR*)user_name, (SQLSMALLINT)strlen(user_name), (SQLTCHAR*)pwd, 
            (SQLSMALLINT)strlen(pwd) ) == SQL_ERROR )
   {
      err_code = 1002; // ������ ����������� � ��������� ������
      return 0;
   }

   return 1;
}

//++++++++ ��������� ������ � �������� +++++++++++++++++++++++++++++++++++++++
void PEnv :: FormatErrorMessage( SQLSMALLINT HandleType,
                                 SQLHANDLE   Handle,
                                 SQLRETURN   retcode)
{
   err_msg = "";  // ������� ������ �� ����������� �����������
   SQLCHAR     szErrorMsg[SQL_MAX_MESSAGE_LENGTH + 1];
   SQLCHAR     szSqlState[6];
   SQLINTEGER  iNativeError;
   SQLSMALLINT bErrorMsg;

   int        count = 1;

   while
   (
      SQLGetDiagRec ( HandleType,
                      Handle,
                      count,
                      szSqlState,
                      &iNativeError,
                      szErrorMsg,
                      sizeof(szErrorMsg),
                      &bErrorMsg) != SQL_NO_DATA
   )
   {
      err_msg += "State : ";
      err_msg += (char*)szSqlState;
      err_msg += "\n";
      err_msg += "Mess: ";
      err_msg += (char*)szErrorMsg;
      count++;

      // �� ������ ����������� �� ���������� ������
//      if ( count > MAX_ERRORS )
            //break;
   }
}

//++++++ �������� ������ ��������� ������ ��� ������� � ������ Microsoft Access +++++++
int PEnv :: CreateAccessDSN(const char *file_path, const char *DSN_name)
{
   // ��� ��� ���������� ����� ������, ����������
   SQLHENV  hEnv=NULL;
   int retCode;

   SQLAllocEnv( &hEnv );

   WORD fRequest = ODBC_ADD_SYS_DSN;
   UCHAR lpszDriver[] = "Microsoft Access Driver (*.mdb)";
   UCHAR lpszAttributes[255] = "DSN=";
   
   strcat((char*)lpszAttributes, DSN_name);
   strcat((char*)lpszAttributes, ";DBQ=");
   strcat((char*)lpszAttributes, file_path);

   retCode = SQLConfigDataSource(NULL, fRequest, (const char*)lpszDriver, (const char*)lpszAttributes);
   
   SQLFreeEnv( hEnv );
   
   if (retCode)
      return 1;
   else
      return 0;
}


//============================================================================
//=========       ���������� ������ PGTStream      ===========================
//============================================================================
PGTStream &PGTStream :: operator,(PGenType &gt_in)
{
   if (!m_quest) return *this;

   if (!m_quest->BindField(gt_in))
   {
      errors += m_quest->GetErrorMessage();
      errors += "\n\n";  // ����������� ������

      err_count++;
   }

   return *this;
}

//============================================================================
//=========       ���������� ������ Quest          ===========================
//============================================================================

//+++++++++++ ����������� ++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma warning(disable:4355)
Quest :: Quest(PEnv &env_in)
       : gt_stream(this)
{
   m_hstmt     = NULL;
   err_code    = 0;
   m_env_p     = &env_in;   // ���������� ��������� �� ����������

   // ���������� ���������
   b_was_sqlexecute = 0;
   n_col     = 1;
}

//+++++++++++ ����� ����������� ����������� ������� ++++++++++++++++++++++++++
void Quest :: Reset()
{
   if (m_hstmt)
   {
      // ����������� ������
      if (b_was_sqlexecute)
         SQLCloseCursor(m_hstmt);

      b_was_sqlexecute = 0;

      // ����������� ���������� �������
      SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);

      m_hstmt = NULL;
   }

   sql = "";

   n_col = 1;  // ����� ����������� � �������� �������
}

//+++++++++++ ���������� ����� SQL ������ ++++++++++++++++++++++++++++++++++++
int Quest :: Statement(const char *stmt_in)
{
   // ���������, �� ������ �� ������ �������� � �������
   if (!stmt_in)
   {
      err_code = 2000;  // ������� ������ �������
      return 0;
   }

   // ���������� ���������� ������
   Reset();

   // ������� ����� ���������� �������
   SQLRETURN ret = ::SQLAllocHandle(SQL_HANDLE_STMT, m_env_p->GetConnectionHandle(), &m_hstmt);
   if (ret == SQL_ERROR)
   {
      FormatErrorMessage(ret);  // ������ - � ��� ��� �����������, �� �������� ����� �������� ������
      err_code = 2001;  // ������ �������� ����������� �������
      return 0;
   }

   // �������������� ������ � ����������
   sql = stmt_in;  // ���������� ������
   ret = SQLPrepare(m_hstmt, (SQLCHAR *)stmt_in, SQL_NTS);
   
   if (ret!=SQL_SUCCESS && ret!=SQL_SUCCESS_WITH_INFO)
   {
      FormatErrorMessage(ret);
      err_code = 2011; // ������ ���������� ������� � ����������
      return 0;
   }

   return 1;
}

//+++++++++++ ��������� ������ +++++++++++++++++++++++++++++++++++++++++++++++
int Quest :: ExecSQL()
{
   if (b_was_sqlexecute)
   {
      err_code = 2002;  // ������ ��� ����� (������ ���� �������� ExecSQL ������ ��� ������ Statement'a)
      return 0;
   }

   if (!m_hstmt)
   {
      err_code = 2003;  // ������� ���������� �������
      return 0;
   }

   int ret = SQLExecute(m_hstmt);
   if (ret == SQL_ERROR)
   {
      FormatErrorMessage(ret);
      err_code = 2004;  // ������ ���������� �������
      return 0;
   }

   b_was_sqlexecute = 1;  // ������������� ������� ����, ��� ������ ������� ��������� � � ��� ���� ������
   
   return 1;
}

//++++++++++ ������� ���������� � ����������� ������� +++++++++++++++++++++++++
int Quest :: BindField(PGenType &gt)
{
   if (!m_hstmt)
   {
      err_code = 2005;  // ������� ������� ���������� � �������������� ��������
      return 0;
   }

   // ��� ���� ����� ����������� ������� ���������� ���� ���� ���������� �� ������ ������� -
   // ���� ����� ���� ������� ��������� �������
   int n_col_local = n_col++;

   // �������� ���������� � ����������� �������
   SQLRETURN ret;
//   SQLCHAR col_name[65];
//   SQLSMALLINT col_name_length;  // ��������� ����� ����� �������
   SQLSMALLINT data_type;        // ��� �������
   SQLUINTEGER data_size;        // ������ ������, ���������� ��� �������� �������
   SQLSMALLINT decimal_digits;   // �������� ������� (������ ��� SQL_DECIMAL, SQL_NUMERIC, SQL_TIMESTAMP)
//   SQLSMALLINT nullable_status;  // ����� �� � ������ ������� ��������� NULL

   // �� �������� ���������� � �������� �����
   ret = SQLDescribeCol(m_hstmt,
                        n_col_local,
                        NULL
                        /*col_name*/,
                        /*64*/0,
                        /*&col_name_length*/NULL,
                        &data_type,
                        &data_size,
                        &decimal_digits,
                        /*&nullable_status*/NULL);

   if (ret!=SQL_SUCCESS && ret!=SQL_SUCCESS_WITH_INFO)
   {
      FormatErrorMessage(ret);
      err_code = 2009; // ������ ��������� ���������� � �������
      return 0;
   }

   // DEBUG!!!!!!!!!!!!!!!!!!!!!!
//   printf("BindField: n_col=%i, data_type=%i, data_size=%i, decimal_digits=%i<br>\n",(int)n_col, (int)data_type, (int)data_size, (int)decimal_digits);
   // !!!!!!!!!!!!!!!!!!!!!!!!!!!

   // ���� � ������� ODBC ��� ��������� �������, �� �������� ����� ��� ���
   if (data_type==SQL_VARCHAR || data_type==SQL_CHAR)
      gt.Allocate(data_size+1);  // +1 ������ ��� ��� ������������� ����
   else
   if  (data_type==SQL_NUMERIC)
      gt.Allocate(data_size+decimal_digits+2);  // +2 ������ ��� ��� ������������� ���� � ���������� �����
   else
   if (data_type==SQL_DOUBLE || data_type==SQL_REAL || data_type==SQL_FLOAT)
      gt.Allocate(data_size+1);  // +1 ��� ������������� ����
   else
   if (data_type==SQL_INTEGER || data_type==SQL_SMALLINT)
      gt.Allocate(data_size+1);  // ��� ������������������ ����� � ������

   // ��������� ������� � �������
   SQLLEN len_or_ind;   // ���������� ��� �������� ��������� ����������
   ret = SQLBindCol(m_hstmt,
                    n_col_local,
                    (SQLSMALLINT)gt.GetSQLType(),
                    gt.GetBufferPtr(),
                    gt.GetBufferSize(),
                    &len_or_ind
                   );

   if (ret != SQL_SUCCESS)
   {
      FormatErrorMessage(ret);
      err_code = 2006;  // ������ ��� ���������� ���������� � ����������� �������    
      return 0;
   }

   return 1;
}

//++++++++ ������� � �������� ����� ��������� GenType'�� ++++++++++++++++++++++
PGTStream &Quest :: BindFields(void)
{
   gt_stream.Reset();  // ���������� ����� � ������ (���� �������� ������ ������)
   return gt_stream;
}

//++++++++ �������� ���� ������ ������� +++++++++++++++++++++++++++++++++++++++
int Quest :: Fetch()
{
   if (!m_hstmt)
   {
      err_code = 2007;  // ������� �������� ��������� ��������������� �������
      return 0;
   }

   SQLRETURN ret;
   // Pushkin : ��� ����� ��������� �����. ��� ��� ���������� ������, ��� ��� ����� ��������, ��
   // ���� �� �������� try/catch, �� ��� ��� ������ ������� ��� ������������� ��������
   // ���������� ������ ����� �������
   try
   {
      if ((ret = SQLFetch(m_hstmt)) == SQL_ERROR)
      {
         FormatErrorMessage(ret);
         err_code = 2008;  // ������ �������� ������
         return 0;
      }
   }
   catch(...)
   {
      err_code = 2010; // ���������� ��� ��������� ������ �������
      return 0;
   }

   if (ret == SQL_NO_DATA)
   {
      err_code = 0;  // ��� ������ - ������ ��� ������ ������
      return 0;
   }
   
   return 1;  // ����� ������ ����
}

//+++++++++++++ ������������ ������ � �������� +++++++++++++++++++++++++++
void Quest :: FormatErrorMessage(SQLRETURN retcode)
{
   m_env_p->FormatErrorMessage(SQL_HANDLE_STMT, m_hstmt, retcode);

   err_msg = m_env_p->GetErrorMessage();

   if (sql != "")
   {
      err_msg += "\nSQL call=";
      err_msg += sql;
   }
}
