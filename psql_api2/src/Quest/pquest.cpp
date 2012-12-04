#include <stdio.h>
#include "pquest.hpp"

using namespace PSQL_API;
//============================================================================
//=========       реализация класса PConnection       ========================
//============================================================================

//+++++++++++ Конструктор ++++++++++++++++++++++++++++++++++++++++++++++++++++
PEnv::PEnv()
{
   henv=NULL;
   hdbc=NULL;

   err_code = 0;  // По умолчанию нет кода ошибки
}

//+++++++++++ Отключение от БД +++++++++++++++++++++++++++++++++++++++++++++++
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

//++++++++++ Подключение к БД ++++++++++++++++++++++++++++++++++++++++++++++++
int PEnv::Connect(const char *DSN, const char *user_name, const char *pwd)
{
   // Сначала на всякий случай делаем отключение, если мы подключены
   Disconnect();

   // Загружаем менеджер драйвера
   if (::SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv) == SQL_ERROR)
   {
      err_code = 1000;  // Ошибка загрузки менеджера драйвера
      return 0;
   }

   // Устанавливаем версию ODBC для нашего драйвера
   ::SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, SQL_IS_INTEGER);

   // Создаем дескриптор источника данных
   if (::SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc) == SQL_ERROR)
   {
      err_code = 1001; // Ошибка создания дескриптора источника данных
      return 0;
   }

   // Подключаемся к источнику данных
   if (::SQLConnect(hdbc, (SQLTCHAR*)DSN, SQL_NTS,
            (SQLTCHAR*)user_name, (SQLSMALLINT)strlen(user_name), (SQLTCHAR*)pwd, 
            (SQLSMALLINT)strlen(pwd) ) == SQL_ERROR )
   {
      err_code = 1002; // Ошибка подключения к источнику данных
      return 0;
   }

   return 1;
}

//++++++++ Формирует строку с ошибками +++++++++++++++++++++++++++++++++++++++
void PEnv :: FormatErrorMessage( SQLSMALLINT HandleType,
                                 SQLHANDLE   Handle,
                                 SQLRETURN   retcode)
{
   err_msg = "";  // Очищаем строку от предыдущего содержимого
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

      // Не делаем ограничений на количество ошибок
//      if ( count > MAX_ERRORS )
            //break;
   }
}

//++++++ Создание своего источника данных для доступа к данным Microsoft Access +++++++
int PEnv :: CreateAccessDSN(const char *file_path, const char *DSN_name)
{
   // Тут все переменные будут своими, локальными
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
//=========       реализация класса PGTStream      ===========================
//============================================================================
PGTStream &PGTStream :: operator,(PGenType &gt_in)
{
   if (!m_quest) return *this;

   if (!m_quest->BindField(gt_in))
   {
      errors += m_quest->GetErrorMessage();
      errors += "\n\n";  // Разедлитель ошибок

      err_count++;
   }

   return *this;
}

//============================================================================
//=========       реализация класса Quest          ===========================
//============================================================================

//+++++++++++ Конструктор ++++++++++++++++++++++++++++++++++++++++++++++++++++

#pragma warning(disable:4355)
Quest :: Quest(PEnv &env_in)
       : gt_stream(this)
{
   m_hstmt     = NULL;
   err_code    = 0;
   m_env_p     = &env_in;   // Запоминаем указатель на соединение

   // Сбрасываем состояния
   b_was_sqlexecute = 0;
   n_col     = 1;
}

//+++++++++++ Сброс результатов предыдущего запроса ++++++++++++++++++++++++++
void Quest :: Reset()
{
   if (m_hstmt)
   {
      // Освобождаем курсор
      if (b_was_sqlexecute)
         SQLCloseCursor(m_hstmt);

      b_was_sqlexecute = 0;

      // Освобождаем дескриптор запроса
      SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);

      m_hstmt = NULL;
   }

   sql = "";

   n_col = 1;  // Номер связываемой с запросом колонки
}

//+++++++++++ Установить новый SQL запрос ++++++++++++++++++++++++++++++++++++
int Quest :: Statement(const char *stmt_in)
{
   // Проверяем, не пустую ли строку передали в запросе
   if (!stmt_in)
   {
      err_code = 2000;  // Нулевая строка запроса
      return 0;
   }

   // Сбрасываем предыдущий запрос
   Reset();

   // Создаем новый дескриптор запроса
   SQLRETURN ret = ::SQLAllocHandle(SQL_HANDLE_STMT, m_env_p->GetConnectionHandle(), &m_hstmt);
   if (ret == SQL_ERROR)
   {
      FormatErrorMessage(ret);  // Ошибка - у нас нет дескриптора, по которому можно получить ошибку
      err_code = 2001;  // Ошибка создания дескриптора запроса
      return 0;
   }

   // Подготавливаем запрос к выполнению
   sql = stmt_in;  // Запоминаем запрос
   ret = SQLPrepare(m_hstmt, (SQLCHAR *)stmt_in, SQL_NTS);
   
   if (ret!=SQL_SUCCESS && ret!=SQL_SUCCESS_WITH_INFO)
   {
      FormatErrorMessage(ret);
      err_code = 2011; // Ошибка подготовки запроса к выполнению
      return 0;
   }

   return 1;
}

//+++++++++++ Выполнить запрос +++++++++++++++++++++++++++++++++++++++++++++++
int Quest :: ExecSQL()
{
   if (b_was_sqlexecute)
   {
      err_code = 2002;  // Запрос уже пошел (нельзя пока вызывать ExecSQL дважды для одного Statement'a)
      return 0;
   }

   if (!m_hstmt)
   {
      err_code = 2003;  // Нулевой дескриптор запроса
      return 0;
   }

   int ret = SQLExecute(m_hstmt);
   if (ret == SQL_ERROR)
   {
      FormatErrorMessage(ret);
      err_code = 2004;  // Ошибка выполнения запроса
      return 0;
   }

   b_was_sqlexecute = 1;  // Устанавливаем признак того, что запуск запроса произошел и у нас есть курсор
   
   return 1;
}

//++++++++++ Связать переменную с результатом запроса +++++++++++++++++++++++++
int Quest :: BindField(PGenType &gt)
{
   if (!m_hstmt)
   {
      err_code = 2005;  // Попытка связать переменную с несуществующим запросом
      return 0;
   }

   // Это чтоб номер связываемой колонки приращался даже если связывание не прошло успешно -
   // чтоб можно было связать остальные колонки
   int n_col_local = n_col++;

   // Получаем информацию о связываемой колонке
   SQLRETURN ret;
//   SQLCHAR col_name[65];
//   SQLSMALLINT col_name_length;  // Требуемая длина имени колонки
   SQLSMALLINT data_type;        // Тип колонки
   SQLUINTEGER data_size;        // Размер буфера, необходимы для хранения колонки
   SQLSMALLINT decimal_digits;   // Точность колонки (только для SQL_DECIMAL, SQL_NUMERIC, SQL_TIMESTAMP)
//   SQLSMALLINT nullable_status;  // Можно ли в данной колонке оставлять NULL

   // Не получаем информацию о ненужных полях
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
      err_code = 2009; // Ошибка получения информации о колонке
      return 0;
   }

   // DEBUG!!!!!!!!!!!!!!!!!!!!!!
//   printf("BindField: n_col=%i, data_type=%i, data_size=%i, decimal_digits=%i<br>\n",(int)n_col, (int)data_type, (int)data_size, (int)decimal_digits);
   // !!!!!!!!!!!!!!!!!!!!!!!!!!!

   // Если в таблице ODBC это строковая колонка, то выделяем буфер под нее
   if (data_type==SQL_VARCHAR || data_type==SQL_CHAR)
      gt.Allocate(data_size+1);  // +1 потому что еще терминирующий ноль
   else
   if  (data_type==SQL_NUMERIC)
      gt.Allocate(data_size+decimal_digits+2);  // +2 потому что еще терминирующий нуль и десятичная точка
   else
   if (data_type==SQL_DOUBLE || data_type==SQL_REAL || data_type==SQL_FLOAT)
      gt.Allocate(data_size+1);  // +1 ибо терминирующий ноль
   else
   if (data_type==SQL_INTEGER || data_type==SQL_SMALLINT)
      gt.Allocate(data_size+1);  // для автопреобразования чисел в строки

   // Связываем колонку с буфером
   SQLLEN len_or_ind;   // Переменная для передачи последним параметром
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
      err_code = 2006;  // Ошибка при связывании переменной с результатом запроса    
      return 0;
   }

   return 1;
}

//++++++++ Связать с запросом сразу несколько GenType'ов ++++++++++++++++++++++
PGTStream &Quest :: BindFields(void)
{
   gt_stream.Reset();  // Сбрасываем поток с полями (пока касается только ошибок)
   return gt_stream;
}

//++++++++ Вытянуть одну строку запроса +++++++++++++++++++++++++++++++++++++++
int Quest :: Fetch()
{
   if (!m_hstmt)
   {
      err_code = 2007;  // Попытка получить результат несуществующего запроса
      return 0;
   }

   SQLRETURN ret;
   // Pushkin : тут очень скользкое место. Или это компилятор глючит, или еще какая проблема, но
   // если не написать try/catch, то тут все нахрен валится при использовании поточных
   // операторов связки полей запроса
   try
   {
      if ((ret = SQLFetch(m_hstmt)) == SQL_ERROR)
      {
         FormatErrorMessage(ret);
         err_code = 2008;  // Ошибка получния данных
         return 0;
      }
   }
   catch(...)
   {
      err_code = 2010; // Исключение при получении данных запроса
      return 0;
   }

   if (ret == SQL_NO_DATA)
   {
      err_code = 0;  // Нет ошибки - просто нет больше данных
      return 0;
   }
   
   return 1;  // Иначе данные есть
}

//+++++++++++++ Формирование строки с ошибками +++++++++++++++++++++++++++
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
